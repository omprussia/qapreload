// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#ifndef QAMOUSEENGINE_HPP
#define QAMOUSEENGINE_HPP

#include <QObject>
#include <QPointF>
#include <QVariant>
#include <QTouchEvent>

class QAPendingEvent;
class QElapsedTimer;
class QTimer;
class QTouchDevice;
class QAMouseEngine : public QObject
{
    Q_OBJECT
public:
    explicit QAMouseEngine(QObject *parent = nullptr);
    bool isRunning() const;

    enum MouseMode {
        MouseEventMode,
        TouchEventMode,
    };
    MouseMode mode();
    void setMode(MouseMode mode);

    QAPendingEvent *click(const QPointF &point);
    QAPendingEvent *pressAndHold(const QPointF &point, int delay = 1200);
    QAPendingEvent *drag(const QPointF &pointA, const QPointF &pointB, int delay = 1200, int duration = 500, int moveSteps = 20, int releaseDelay = 600);
    QAPendingEvent *move(const QPointF &pointA, const QPointF &pointB, int duration = 500, int moveSteps = 20, int releaseDelay = 600);

    QAPendingEvent *performMultiAction(const QVariantList &multiActions);
    QAPendingEvent *performTouchAction(const QVariantList &actions);

    int getNextPointId();
    qint64 getEta();
    QTouchDevice *getTouchDevice();

signals:
    void touchEvent(const QTouchEvent &event);
    void mouseEvent(const QMouseEvent &event);

private slots:
    void onPressed(const QPointF point);
    void onMoved(const QPointF point);
    void onReleased(const QPointF point);

private:
    friend class TouchAction;
    QElapsedTimer *m_eta;

    QHash<QObject*, QTouchEvent::TouchPoint> m_touchPoints;

    MouseMode m_mode = MouseEventMode;

    QTouchDevice *m_touchDevice;
    int m_tpId = 0;
};

class EventWorker : public QObject
{
    Q_OBJECT
public:
    explicit EventWorker(const QVariantList &actions, QAMouseEngine *engine);
    virtual ~EventWorker();
    static EventWorker *PerformTouchAction(const QVariantList &actions, QAMouseEngine *engine);

public slots:
    void start();

private:
    void sendPress(const QPointF &point);
    void sendRelease(const QPointF &point);
    void sendRelease(const QPointF &point, int delay);
    void sendMove(const QPointF &point);
    void sendMove(const QPointF &previousPoint, const QPointF &point, int duration, int moveSteps);

    QVariantList m_actions;
    QAMouseEngine *m_engine = nullptr;

signals:
    void pressed(const QPointF point);
    void moved(const QPointF point);
    void released(const QPointF point);

    void finished();
};

#endif // QAMOUSEENGINE_HPP
