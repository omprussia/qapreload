// Copyright (c) 2020 Open Mobile Platform LLС.
#include "QAKeyEngine.hpp"
#include "QAPendingEvent.hpp"

#include <QKeyEvent>

QAKeyEngine::QAKeyEngine(QObject *parent)
    : QObject(parent)
{
}

QAPendingEvent *QAKeyEngine::pressEnter(int count)
{
    QAPendingEvent *pending = new QAPendingEvent(this);

    for (int i = 0; i < count; i++) {
        sendPress('\n', Qt::Key_Enter);
        sendRelease('\n', Qt::Key_Enter);
    }

    QMetaObject::invokeMethod(pending, "setCompleted", Qt::QueuedConnection);
    return pending;
}

QAPendingEvent *QAKeyEngine::pressBackspace(int count)
{
    QAPendingEvent *pending = new QAPendingEvent(this);

    for (int i = 0; i < count; i++) {
        sendPress('\b', Qt::Key_Backspace);
        sendRelease('\b', Qt::Key_Backspace);
    }

    QMetaObject::invokeMethod(pending, "setCompleted", Qt::QueuedConnection);
    return pending;
}

QAPendingEvent *QAKeyEngine::pressKeys(const QString &keys)
{
    QAPendingEvent *pending = new QAPendingEvent(this);

    for (const QChar &key : keys) {
        sendPress(key);
        sendRelease(key);
    }

    QMetaObject::invokeMethod(pending, "setCompleted", Qt::QueuedConnection);
    return pending;
}

void QAKeyEngine::sendPress(const QChar &text, int key)
{
    QKeyEvent event(QKeyEvent::KeyPress,
                    key,
                    Qt::NoModifier,
                    QString(text));

    emit triggered(&event);
}

void QAKeyEngine::sendRelease(const QChar &text, int key)
{
    QKeyEvent event(QKeyEvent::KeyRelease,
                    key,
                    Qt::NoModifier,
                    QString(text));

    emit triggered(&event);
}
