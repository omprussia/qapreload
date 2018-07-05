#ifndef QAMOUSEENGINE_HPP
#define QAMOUSEENGINE_HPP

#include <QObject>
#include <QPointF>

class QElapsedTimer;
class QTimer;
class QMouseEvent;
class QAMouseEngine : public QObject
{
    Q_OBJECT
public:
    explicit QAMouseEngine(QObject *parent = nullptr);
    bool isRunning() const;

    void press(const QPointF &point);
    void release(const QPointF &point);
    void click(const QPointF &point);
    void pressAndHold(const QPointF &point, int delay = 1200);
    void move(const QPointF &pointA, const QPointF &pointB, int duration = 100, int moveSteps = 100, int releaseDelay = 600);

signals:
    void triggered(QMouseEvent *event);

public slots:

private slots:
    void sendPress(const QPointF &point);
    void sendRelease(const QPointF &point);
    void sendRelease(const QPointF &point, int delay);
    void sendMove(const QPointF &point);

    void onMoveTimer();

private:
    Qt::MouseButtons m_buttons = Qt::NoButton;
    QElapsedTimer *m_eta;

    // MOVE POLYANA
    QTimer *m_timer;
    QPointF m_pointA;
    QPointF m_pointB;
    int m_releaseAfterMoveDelay = 1000;
    float m_moveStepSize = 5.0f;
    int m_moveStepCount = 1;
    int m_currentMoveStep = 0;
    //
};

#endif // QAMOUSEENGINE_HPP
