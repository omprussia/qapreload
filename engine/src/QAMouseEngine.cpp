#include "QAMouseEngine.hpp"
#include "QAPendingEvent.hpp"

#include <QMouseEvent>
#include <QTimer>
#include <private/qvariantanimation_p.h>
#include <QtMath>

#include <QElapsedTimer>

#include <QDebug>

QAMouseEngine::QAMouseEngine(QObject *parent)
    : QObject(parent)
    , m_eta(new QElapsedTimer())
    , m_timer(new QTimer(this))
{
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
    m_buttons |= Qt::LeftButton;
    QMouseEvent event(QMouseEvent::MouseButtonPress,
                      point,
                      point,
                      point,
                      Qt::LeftButton,
                      m_buttons,
                      Qt::NoModifier,
                      Qt::MouseEventSynthesizedByQt);
    event.setTimestamp(m_eta->elapsed());

    emit triggered(&event);
}

void QAMouseEngine::sendRelease(const QPointF &point)
{
    m_buttons &= ~Qt::LeftButton;
    QMouseEvent event(QMouseEvent::MouseButtonRelease,
                      point,
                      point,
                      point,
                      Qt::LeftButton,
                      m_buttons,
                      Qt::NoModifier,
                      Qt::MouseEventSynthesizedByQt);
    event.setTimestamp(m_eta->elapsed());

    emit triggered(&event);
}

void QAMouseEngine::sendRelease(const QPointF &point, int delay)
{
    QTimer::singleShot(delay, this, [this, point](){
        sendRelease(point);
    });
}

void QAMouseEngine::sendMove(const QPointF &point)
{
    QMouseEvent event(QMouseEvent::MouseMove,
                      point,
                      point,
                      point,
                      Qt::LeftButton,
                      m_buttons,
                      Qt::NoModifier,
                      Qt::MouseEventSynthesizedByQt);
    event.setTimestamp(m_eta->elapsed());

    emit triggered(&event);
}
