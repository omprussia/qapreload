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

QAPendingEvent *QAMouseEngine::drag(const QPointF &pointA, const QPointF &pointB, int delay, int duration, int moveSteps, int releaseDelay)
{
    if (m_pendingMove) {
        qWarning() << Q_FUNC_INFO << "Have pendingMove:" << m_pendingMove;
        return m_pendingMove;
    }
    m_pendingMove = new QAPendingEvent(this);

    if (duration < 1 || moveSteps < 1) {
        qWarning() << Q_FUNC_INFO << "QA ENGINEER IDIOT";
    }

    sendPress(pointA);

    m_pointA = pointA;
    m_pointB = pointB;
    m_releaseAfterMoveDelay = releaseDelay;

    const float stepX = qAbs(pointB.x() - pointA.x()) / moveSteps;
    const float stepY = qAbs(pointB.y() - pointA.y()) / moveSteps;

    if (stepX > 0 && stepX < m_moveStepSize) {
        m_moveStepCount = qAbs(qRound(pointB.x() - pointA.x())) / m_moveStepSize;
    } else if (stepY > 0 && stepY < m_moveStepSize) {
        m_moveStepCount = qAbs(qRound(pointB.y() - pointA.y())) / m_moveStepSize;
    } else {
        m_moveStepCount = moveSteps;
    }

    m_currentMoveStep = 0;
    m_timer->setInterval(duration / m_moveStepCount);

    QTimer *delayTimer = new QTimer;
    connect(delayTimer, &QTimer::timeout, m_timer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(delayTimer, &QTimer::timeout, delayTimer, &QObject::deleteLater);
    delayTimer->setSingleShot(true);
    delayTimer->start(delay);

    return m_pendingMove;
}

QAPendingEvent *QAMouseEngine::move(const QPointF &pointA, const QPointF &pointB, int duration, int moveSteps, int releaseDelay)
{
    if (m_pendingMove) {
        qWarning() << Q_FUNC_INFO << "Have pendingMove:" << m_pendingMove;
        return m_pendingMove;
    }
    m_pendingMove = new QAPendingEvent(this);

    if (duration < 1 || moveSteps < 1) {
        qWarning() << Q_FUNC_INFO << "QA ENGINEER IDIOT";
    }

    sendPress(pointA);

    m_pointA = pointA;
    m_pointB = pointB;
    m_releaseAfterMoveDelay = releaseDelay;

    const float stepX = qAbs(pointB.x() - pointA.x()) / moveSteps;
    const float stepY = qAbs(pointB.y() - pointA.y()) / moveSteps;

    if (stepX > 0 && stepX < m_moveStepSize) {
        m_moveStepCount = qAbs(qRound(pointB.x() - pointA.x())) / m_moveStepSize;
    } else if (stepY > 0 && stepY < m_moveStepSize) {
        m_moveStepCount = qAbs(qRound(pointB.y() - pointA.y())) / m_moveStepSize;
    } else {
        m_moveStepCount = moveSteps;
    }

    m_currentMoveStep = 0;

    m_timer->start(duration / m_moveStepCount);

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
            m_pendingMove->deleteLater();
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

    QTouchEvent te(QEvent::TouchBegin,
                   m_touchDevice,
                   Qt::NoModifier,
                   Qt::TouchPointPressed,
                   { tp });
    const quint64 timestamp = m_eta->elapsed();
    te.setTimestamp(timestamp);
    m_previousEventTimestamp = timestamp;
    m_previousPoint = point;
    m_pressPoint = point;

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
    tp.setLastPos(m_previousPoint);
    tp.setStartPos(m_pressPoint);
    tp.setPressure(0);

    const quint64 timestamp = m_eta->elapsed();
    const quint64 timeDelta = timestamp - m_previousEventTimestamp;

    if (timeDelta > 0) {
        QVector2D velocity;
        velocity.setX((point.x() - m_previousPoint.x()) / timeDelta * 1000);
        velocity.setY((point.y() - m_previousPoint.y()) / timeDelta * 1000);

        tp.setVelocity(velocity);
    }

    QTouchEvent te(QEvent::TouchEnd,
                   m_touchDevice,
                   Qt::NoModifier,
                   Qt::TouchPointReleased,
                   { tp });
    te.setTimestamp(timestamp);
    m_previousEventTimestamp = timestamp;
    m_previousPoint = point;

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
    tp.setLastPos(m_previousPoint);
    tp.setStartPos(m_pressPoint);
    tp.setPressure(1);

    const quint64 timestamp = m_eta->elapsed();
    const quint64 timeDelta = timestamp - m_previousEventTimestamp;

    if (timeDelta > 0) {
        QVector2D velocity;
        velocity.setX((point.x() - m_previousPoint.x()) / timeDelta * 1000);
        velocity.setY((point.y() - m_previousPoint.y()) / timeDelta * 1000);

        tp.setVelocity(velocity);
    }

    QTouchEvent te(QEvent::TouchUpdate,
                   m_touchDevice,
                   Qt::NoModifier,
                   Qt::TouchPointPressed,
                   { tp });
    te.setTimestamp(timestamp);
    m_previousEventTimestamp = timestamp;
    m_previousPoint = point;

    emit touchEvent(te);
}
