#ifndef QAMOUSEENGINE_HPP
#define QAMOUSEENGINE_HPP

#include <QObject>
#include <QPointF>

class QAPendingEvent;
class QElapsedTimer;
class QTimer;
class QTouchEvent;
class QTouchDevice;
class QAMouseEngine : public QObject
{
    Q_OBJECT
public:
    explicit QAMouseEngine(QObject *parent = nullptr);
    bool isRunning() const;

    QAPendingEvent *press(const QPointF &point);
    QAPendingEvent *release(const QPointF &point);
    QAPendingEvent *click(const QPointF &point);
    QAPendingEvent *pressAndHold(const QPointF &point, int delay = 1200);
    QAPendingEvent *move(const QPointF &pointA, const QPointF &pointB, int duration = 20, int moveSteps = 20, int releaseDelay = 600);

signals:
    void touchEvent(const QTouchEvent &event);

private slots:
    void sendPress(const QPointF &point);
    void sendRelease(const QPointF &point);
    void sendRelease(const QPointF &point, int delay);
    void sendMove(const QPointF &point);

    void onMoveTimer();

private:
    QElapsedTimer *m_eta;

    // MOVE POLYANA
    QTimer *m_timer;
    QPointF m_pointA;
    QPointF m_pointB;
    int m_releaseAfterMoveDelay = 1000;
    float m_moveStepSize = 5.0f;
    int m_moveStepCount = 1;
    int m_currentMoveStep = 0;
    QAPendingEvent *m_pendingMove = nullptr;
    //

    QTouchDevice *m_touchDevice;
    int m_tpId = 0;
    qint64 m_previousEventTimestamp = 0;
    QPointF m_previousPoint;
    QPointF m_pressPoint;
};

#endif // QAMOUSEENGINE_HPP
