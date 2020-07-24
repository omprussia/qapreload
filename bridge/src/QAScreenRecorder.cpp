// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "QAScreenRecorder.hpp"

#include <QDBusConnection>
#include <QDBusReply>
#include <QEventLoop>

#include <QDebug>

namespace {

const QString s_screenRecorderService = QStringLiteral("org.coderus.screenrecorder");
const QString s_screenRecorderObject = QStringLiteral("/org/coderus/screenrecorder");
const QString s_screenRecorderInterface = QStringLiteral("org.coderus.screenrecorder");

}

QAScreenRecorder::QAScreenRecorder(QObject *parent)
    : QObject(parent)
{
    QDBusConnection systemBus = QDBusConnection::systemBus();

    systemBus.connect(QString(),
        s_screenRecorderObject,
        s_screenRecorderInterface,
        QStringLiteral("RecordingFinished"),
        this, SLOT(onRecordingFinished(QString)));

    systemBus.connect(QString(),
        s_screenRecorderObject,
        s_screenRecorderInterface,
        QStringLiteral("StateChanged"),
        this, SLOT(onStateChanged(int)));

    QDBusMessage stateMessage = QDBusMessage::createMethodCall(
        s_screenRecorderService,
        s_screenRecorderObject,
        s_screenRecorderInterface,
        QStringLiteral("GetState"));
    QDBusReply<int> stateReply = systemBus.call(stateMessage);
    if (!stateReply.isValid()) {
        qWarning() << Q_FUNC_INFO << systemBus.lastError().message();
        return;
    }
    onStateChanged(stateReply.value());
}

QString QAScreenRecorder::lastFilename() const
{
    return m_lastFilename;
}

bool QAScreenRecorder::setScale(double scale)
{
    QDBusConnection systemBus = QDBusConnection::systemBus();

    QDBusMessage scaleMessage = QDBusMessage::createMethodCall(
        s_screenRecorderService,
        s_screenRecorderObject,
        s_screenRecorderInterface,
        QStringLiteral("SetScale"));
    scaleMessage.setArguments({scale});
    QDBusReply<void> scaleReply = systemBus.call(scaleMessage);
    if (!scaleReply.isValid()) {
        qWarning() << Q_FUNC_INFO << systemBus.lastError().message();
        return false;
    }

    return true;
}

bool QAScreenRecorder::start()
{
    QDBusConnection systemBus = QDBusConnection::systemBus();

    if (m_state != 1) {
        qWarning() << Q_FUNC_INFO << "Recorder is busy:" << m_state;
        return false;
    }

    QDBusMessage startMessage = QDBusMessage::createMethodCall(
        s_screenRecorderService,
        s_screenRecorderObject,
        s_screenRecorderInterface,
        QStringLiteral("Start"));
    QDBusReply<void> startReply = systemBus.call(startMessage);
    if (!startReply.isValid()) {
        qWarning() << Q_FUNC_INFO << systemBus.lastError().message();
        return false;
    }

    return true;
}

bool QAScreenRecorder::stop()
{
    QDBusConnection systemBus = QDBusConnection::systemBus();

    if (m_state != 2) {
        qWarning() << Q_FUNC_INFO << "Wrong state:" << m_state;
        return false;
    }

    QEventLoop loop;
    connect(this, &QAScreenRecorder::stateChanged, [&loop](int state) {
        if (state == 1) {
            loop.quit();
        }
    });

    QDBusMessage stopMessage = QDBusMessage::createMethodCall(
        s_screenRecorderService,
        s_screenRecorderObject,
        s_screenRecorderInterface,
        QStringLiteral("Stop"));
    QDBusReply<void> stopReply = systemBus.call(stopMessage);
    if (!stopReply.isValid()) {
        qWarning() << Q_FUNC_INFO << systemBus.lastError().message();
        return false;
    }

    loop.exec();

    return true;
}

void QAScreenRecorder::onStateChanged(int state)
{
    qDebug() << Q_FUNC_INFO << state;
    m_state = state;
    emit stateChanged(state);
}

void QAScreenRecorder::onRecordingFinished(const QString &filename)
{
    qDebug() << Q_FUNC_INFO << filename;
    m_lastFilename = filename;
    emit recordingFinished(filename);
}
