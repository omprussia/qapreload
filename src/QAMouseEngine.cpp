#include "QAMouseEngine.hpp"

#include <QMouseEvent>
#include <QTimer>
#include <private/qvariantanimation_p.h>
#include <QtMath>

#include <QDebug>

QAMouseEngine::QAMouseEngine(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, &QAMouseEngine::onMoveTimer);
}

bool QAMouseEngine::isRunning() const
{
    return m_timer->isActive();
}

void QAMouseEngine::press(const QPointF &point)
{
    sendPress(point);
}

void QAMouseEngine::release(const QPointF &point)
{
    sendRelease(point);
}

void QAMouseEngine::click(const QPointF &point)
{
    sendPress(point);
    sendRelease(point);
}

void QAMouseEngine::pressAndHold(const QPointF &point, int delay)
{
    sendPress(point);
    QTimer::singleShot(delay, this, [this, point]() {
        sendRelease(point);
    });
}

void QAMouseEngine::move(const QPointF &pointA, const QPointF &pointB, int duration, int moveSteps, int releaseDelay)
{
    if (duration < 100 || moveSteps < 1) {
        qWarning() << Q_FUNC_INFO << "QA ENGINEER IDIOT";
    }

    sendPress(pointA);

    m_pointA = pointA;
    m_pointB = pointB;
    m_releaseAfterMoveDelay = releaseDelay;

    const float stepX = qAbs(pointB.x() - pointA.x()) / moveSteps;
    const float stepY = qAbs(pointB.y() - pointA.y()) / moveSteps;

    if (stepX < stepY && stepX < 1) {
        m_moveStepCount = qAbs(pointB.y() - pointA.y());
    } else if (stepY < stepX && stepY < 1) {
        m_moveStepCount = qAbs(pointB.x() - pointA.x());
    } else {
        m_moveStepCount = moveSteps;
    }

    m_currentMoveStep = 0;

    m_timer->start(duration / m_moveStepCount);
}

void QAMouseEngine::sendPress(const QPointF &point)
{
    QMouseEvent event(QMouseEvent::MouseButtonPress,
                      point,
                      Qt::LeftButton,
                      Qt::NoButton,
                      Qt::NoModifier);

    emit triggered(&event);
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
    }
}

void QAMouseEngine::sendRelease(const QPointF &point)
{
    QMouseEvent event(QMouseEvent::MouseButtonRelease,
                      point,
                      Qt::LeftButton,
                      Qt::NoButton,
                      Qt::NoModifier);

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
                      Qt::LeftButton,
                      Qt::NoButton,
                      Qt::NoModifier);

    emit triggered(&event);
}
