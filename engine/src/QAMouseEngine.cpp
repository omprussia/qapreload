#include "QAMouseEngine.hpp"
#include "QAPendingEvent.hpp"

#include <QMouseEvent>
#include <QTimer>
#include <private/qvariantanimation_p.h>
#include "qpa/qwindowsysteminterface_p.h"
#include <QtMath>

#include <QElapsedTimer>

#include <QDebug>

QAMouseEngine::QAMouseEngine(QObject *parent)
    : QObject(parent)
    , m_eta(new QElapsedTimer())
    , m_timer(new QTimer(this))
    , m_touchDevice(new QTouchDevice())
{
    m_touchDevice->setCapabilities(QTouchDevice::Position | QTouchDevice::Area);
    m_touchDevice->setMaximumTouchPoints(1);
    m_touchDevice->setType(QTouchDevice::TouchScreen);
    m_touchDevice->setName(QStringLiteral("qainput"));

    QWindowSystemInterface::registerTouchDevice(m_touchDevice);

    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, &QAMouseEngine::onMoveTimer);
    m_eta->start();
}

bool QAMouseEngine::isRunning() const
{
    return m_timer->isActive();
}

QAPendingEvent *QAMouseEngine::press(const QPointF &point)
{
    QAPendingEvent *pending = new QAPendingEvent(this);

    sendPress(point);

    QMetaObject::invokeMethod(pending, "setCompleted", Qt::QueuedConnection);
    return pending;
}

QAPendingEvent *QAMouseEngine::release(const QPointF &point)
{
    QAPendingEvent *pending = new QAPendingEvent(this);

    sendRelease(point);

    QMetaObject::invokeMethod(pending, "setCompleted", Qt::QueuedConnection);
    return pending;
}

QAPendingEvent *QAMouseEngine::click(const QPointF &point)
{
    QAPendingEvent *pending = new QAPendingEvent(this);

    sendPress(point);
    sendRelease(point);

    QMetaObject::invokeMethod(pending, "setCompleted", Qt::QueuedConnection);
    return pending;
}

QAPendingEvent *QAMouseEngine::pressAndHold(const QPointF &point, int delay)
{
    QAPendingEvent *pending = new QAPendingEvent(this);

    sendPress(point);
    QTimer::singleShot(delay, this, [this, point, pending]() {
        sendRelease(point);
        QMetaObject::invokeMethod(pending, "setCompleted", Qt::QueuedConnection);
    });

    return pending;
}

QAPendingEvent *QAMouseEngine::move(const QPointF &pointA, const QPointF &pointB, int duration, int moveSteps, int releaseDelay)
{
    if (m_pendingMove) {
        return m_pendingMove;
    }
    m_pendingMove = new QAPendingEvent(this);

    if (duration < 100 || moveSteps < 1) {
        qWarning() << Q_FUNC_INFO << "QA ENGINEER IDIOT";
    }

    sendPress(pointA);

    m_pointA = pointA;
    m_pointB = pointB;
    m_releaseAfterMoveDelay = releaseDelay;

    const float stepX = qAbs(pointB.x() - pointA.x()) / moveSteps;
    const float stepY = qAbs(pointB.y() - pointA.y()) / moveSteps;

    if (stepX < 1 && stepX > 0) {
        m_moveStepCount = qAbs(pointB.x() - pointA.x());
    } else if (stepY < 1 && stepY > 0) {
        m_moveStepCount = qAbs(pointB.y() - pointA.y());
    } else {
        m_moveStepCount = moveSteps;
    }

    m_currentMoveStep = 0;

    m_timer->start(qMin(duration / m_moveStepCount, 1));

    return m_pendingMove;
}

void QAMouseEngine::onMoveTimer()
{
    auto *interpolator = QVariantAnimationPrivate::getInterpolator(QMetaType::QPointF);
    float progress = static_cast<float>(m_currentMoveStep) / m_moveStepCount;

    QPointF pointMove = interpolator(&m_pointA, &m_pointB, progress).toPointF();

    sendMove(pointMove);
    if (m_currentMoveStep++ == m_moveStepCount) {
        m_timer->stop();

        sendMove(m_pointB);
        sendRelease(m_pointB, m_releaseAfterMoveDelay);

        if (m_pendingMove) {
            QMetaObject::invokeMethod(m_pendingMove, "setCompleted", Qt::QueuedConnection);
            m_pendingMove = nullptr;
        } else {
            qWarning() << Q_FUNC_INFO << "Pending move is null!";
        }
    }
}

void QAMouseEngine::sendPress(const QPointF &point)
{
    QTouchEvent::TouchPoint tp(++m_tpId);
    tp.setState(Qt::TouchPointPressed);

    QRectF rect(point.x() - 16, point.y() - 16, 32, 32);
    tp.setRect(rect);
    tp.setSceneRect(rect);
    tp.setScreenRect(rect);
    tp.setLastPos(point);
    tp.setStartPos(point);
    tp.setPressure(1);
    tp.setVelocity(QVector2D(0, 0));

    QTouchEvent *te = new QTouchEvent(QEvent::TouchBegin,
                   m_touchDevice,
                   Qt::NoModifier,
                   Qt::TouchPointPressed,
                   { tp });
    te->setTimestamp(m_eta->elapsed());

    emit touchEvent(te);
}

void QAMouseEngine::sendRelease(const QPointF &point)
{
    QTouchEvent::TouchPoint tp(m_tpId);
    tp.setState(Qt::TouchPointReleased);

    QRectF rect(point.x() - 16, point.y() - 16, 32, 32);
    tp.setRect(rect);
    tp.setSceneRect(rect);
    tp.setScreenRect(rect);
    tp.setLastPos(point);
    tp.setStartPos(point);
    tp.setPressure(0);
    tp.setVelocity(QVector2D(0, 0));

    QTouchEvent *te = new QTouchEvent(QEvent::TouchEnd,
                   m_touchDevice,
                   Qt::NoModifier,
                   Qt::TouchPointReleased,
                   { tp });
    te->setTimestamp(m_eta->elapsed());

    emit touchEvent(te);
}

void QAMouseEngine::sendRelease(const QPointF &point, int delay)
{
    QTimer::singleShot(delay, this, [this, point](){
        sendRelease(point);
    });
}

void QAMouseEngine::sendMove(const QPointF &point)
{
    QTouchEvent::TouchPoint tp(m_tpId);
    tp.setState(Qt::TouchPointMoved);

    QRectF rect(point.x() - 16, point.y() - 16, 32, 32);
    tp.setRect(rect);
    tp.setSceneRect(rect);
    tp.setScreenRect(rect);
    tp.setLastPos(point);
    tp.setStartPos(point);
    tp.setPressure(1);
    tp.setVelocity(QVector2D(0, 0));

    QTouchEvent *te = new QTouchEvent(QEvent::TouchUpdate,
                   m_touchDevice,
                   Qt::NoModifier,
                   Qt::TouchPointPressed,
                   { tp });
    te->setTimestamp(m_eta->elapsed());

    emit touchEvent(te);
}
