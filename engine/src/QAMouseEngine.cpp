// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "QAEngine.hpp"
#include "QAMouseEngine.hpp"
#include "QAPendingEvent.hpp"
#include "IEnginePlatform.hpp"

#include <QMouseEvent>
#include <QTimer>
#include <private/qvariantanimation_p.h>
#include "qpa/qwindowsysteminterface_p.h"
#include <QtMath>

#include <QElapsedTimer>

#include <QDebug>
#include <QThread>

QAMouseEngine::QAMouseEngine(QObject *parent)
    : QObject(parent)
    , m_eta(new QElapsedTimer())
    , m_touchDevice(new QTouchDevice())
{
    m_touchDevice->setCapabilities(QTouchDevice::Position | QTouchDevice::Area);
    m_touchDevice->setMaximumTouchPoints(10);
    m_touchDevice->setType(QTouchDevice::TouchScreen);
    m_touchDevice->setName(QStringLiteral("qainput"));

    QWindowSystemInterface::registerTouchDevice(m_touchDevice);

    m_eta->start();
}

bool QAMouseEngine::isRunning() const
{
    return m_touchPoints.size() > 0;
}

QAMouseEngine::MouseMode QAMouseEngine::mode()
{
    return m_mode;
}

void QAMouseEngine::setMode(QAMouseEngine::MouseMode mode)
{
    m_mode = mode;
}

QAPendingEvent *QAMouseEngine::click(const QPointF &point)
{
    QVariantList action = {
        QVariantMap{
            {"action", "press"},
            {"options",
                QVariantMap{
                    {"x", point.x()},
                    {"y", point.y()},
                },
            },
        },
        QVariantMap{
            {"action", "release"},
        },
    };

    return performTouchAction(action);
}

QAPendingEvent *QAMouseEngine::pressAndHold(const QPointF &point, int delay)
{
    QVariantList action = {
        QVariantMap{
            {"action", "longPress"},
            {"options",
                QVariantMap{
                    {"x", point.x()},
                    {"y", point.y()},
                },
            },
        },
        QVariantMap{
            {"action", "wait"},
            {"options",
                QVariantMap{
                    {"ms", delay},
                },
            },
        },
        QVariantMap{
            {"action", "release"},
        },
    };

    return performTouchAction(action);
}

QAPendingEvent *QAMouseEngine::drag(const QPointF &pointA, const QPointF &pointB, int delay, int duration, int moveSteps, int releaseDelay)
{
    QVariantList action = {
        QVariantMap{
            {"action", "longPress"},
            {"options",
                QVariantMap{
                    {"x", pointA.x()},
                    {"y", pointA.y()},
                },
            },
        },
        QVariantMap{
            {"action", "wait"},
            {"options",
                QVariantMap{
                    {"ms", delay},
                },
            },
        },
        QVariantMap{
            {"action", "moveTo"},
            {"options",
                QVariantMap{
                    {"x", pointB.x()},
                    {"y", pointB.y()},
                    {"duration", duration},
                    {"steps", moveSteps},
                },
            },
        },
        QVariantMap{
            {"action", "wait"},
            {"options",
                QVariantMap{
                    {"ms", releaseDelay},
                },
            },
        },
        QVariantMap{
            {"action", "release"},
        },
    };

    return performTouchAction(action);
}

QAPendingEvent *QAMouseEngine::move(const QPointF &pointA, const QPointF &pointB, int duration, int moveSteps, int releaseDelay)
{
    QVariantList action = {
        QVariantMap{
            {"action", "press"},
            {"options",
                QVariantMap{
                    {"x", pointA.x()},
                    {"y", pointA.y()},
                },
            },
        },
        QVariantMap{
            {"action", "moveTo"},
            {"options",
                QVariantMap{
                    {"x", pointB.x()},
                    {"y", pointB.y()},
                    {"duration", duration},
                    {"steps", moveSteps},
                },
            },
        },
        QVariantMap{
            {"action", "wait"},
            {"options",
                QVariantMap{
                    {"ms", releaseDelay},
                },
            },
        },
        QVariantMap{
            {"action", "release"},
        },
    };

    return performTouchAction(action);
}

QAPendingEvent *QAMouseEngine::performMultiAction(const QVariantList &multiActions)
{
    QAPendingEvent *event = new QAPendingEvent(this);
    event->setProperty("finishedCount", 0);
    QReadWriteLock *lock = new QReadWriteLock();

    const int actionsSize = multiActions.size();
    for (const QVariant &multiActionVar : multiActions) {
        const QVariantList actions = multiActionVar.toList();

        EventWorker *worker = EventWorker::PerformTouchAction(actions, this);
        connect(worker, &EventWorker::pressed, this, &QAMouseEngine::onPressed);
        connect(worker, &EventWorker::moved, this, &QAMouseEngine::onMoved);
        connect(worker, &EventWorker::released, this, &QAMouseEngine::onReleased);
        connect(worker, &EventWorker::finished, event, [lock, event, actionsSize]() {
            lock->lockForWrite();
            int finishedCount = event->property("finishedCount").toInt();
            event->setProperty("finishedCount", ++finishedCount);
            lock->unlock();
            if (finishedCount == actionsSize) {
                QMetaObject::invokeMethod(event, "setCompleted", Qt::QueuedConnection);
                event->deleteLater();
                delete lock;
            }
        });
    }

    if (m_mode == TouchEventMode) {
        m_touchPoints.clear();
    }

    return event;
}

QAPendingEvent *QAMouseEngine::performTouchAction(const QVariantList &actions)
{
    QAPendingEvent *event = new QAPendingEvent(this);
    EventWorker *worker = EventWorker::PerformTouchAction(actions, this);
    connect(worker, &EventWorker::pressed, this, &QAMouseEngine::onPressed);
    connect(worker, &EventWorker::moved, this, &QAMouseEngine::onMoved);
    connect(worker, &EventWorker::released, this, &QAMouseEngine::onReleased);
    connect(worker, &EventWorker::finished, event, [event]() {
        QMetaObject::invokeMethod(event, "setCompleted", Qt::QueuedConnection);
        event->deleteLater();
    });

    if (m_mode == TouchEventMode) {
        m_touchPoints.clear();
    }

    return event;
}

int QAMouseEngine::getNextPointId()
{
    return ++m_tpId;
}

qint64 QAMouseEngine::getEta()
{
    return m_eta->elapsed();
}

QTouchDevice *QAMouseEngine::getTouchDevice()
{
    return m_touchDevice;
}

void QAMouseEngine::onPressed(const QPointF point)
{
    if (m_mode == TouchEventMode) {
        int pointId = getNextPointId();

        QTouchEvent::TouchPoint tp(pointId);
        tp.setState(Qt::TouchPointPressed);

        QRectF rect(point.x() - 16, point.y() - 16, 32, 32);
        tp.setRect(rect);
        tp.setSceneRect(rect);
        tp.setScreenRect(rect);
        tp.setLastPos(point);
        tp.setStartPos(point);
        tp.setPressure(1);
        m_touchPoints.insert(sender(), tp);

        Qt::TouchPointStates states = tp.state();
        QEvent::Type type = QEvent::TouchBegin;
        if (m_touchPoints.size() > 1) {
            type = QEvent::TouchUpdate;
            states |= Qt::TouchPointStationary;
        }

        QTouchEvent te(type,
                       m_touchDevice,
                       Qt::NoModifier,
                       states,
                       m_touchPoints.values());
        te.setTimestamp(m_eta->elapsed());

        tp.setState(Qt::TouchPointStationary);
        m_touchPoints.insert(sender(), tp);

        emit touchEvent(te);
    } else {
        QMouseEvent me(QEvent::MouseButtonPress,
                       point,
                       Qt::LeftButton,
                       Qt::LeftButton,
                       Qt::NoModifier);
        emit mouseEvent(me);
    }
}

void QAMouseEngine::onMoved(const QPointF point)
{
    if (m_mode == TouchEventMode) {
        QTouchEvent::TouchPoint tp = m_touchPoints.value(sender());

        tp.setState(Qt::TouchPointMoved);

        QRectF rect(point.x() - 16, point.y() - 16, 32, 32);
        tp.setRect(rect);
        tp.setSceneRect(rect);
        tp.setScreenRect(rect);
        tp.setLastPos(tp.pos());
        tp.setStartPos(point);
        tp.setPressure(1);
        m_touchPoints.insert(sender(), tp);

        Qt::TouchPointStates states = tp.state();
        QEvent::Type type = QEvent::TouchUpdate;
        if (m_touchPoints.size() > 1) {
            states |= Qt::TouchPointStationary;
        }

        QTouchEvent te(type,
                       m_touchDevice,
                       Qt::NoModifier,
                       states,
                       m_touchPoints.values());
        te.setTimestamp(m_eta->elapsed());

        tp.setState(Qt::TouchPointStationary);
        m_touchPoints.insert(sender(), tp);

        emit touchEvent(te);
    } else {
        QMouseEvent me(QEvent::MouseMove,
                       point,
                       Qt::LeftButton,
                       Qt::LeftButton,
                       Qt::NoModifier);
        emit mouseEvent(me);
    }
}

void QAMouseEngine::onReleased(const QPointF point)
{
    if (m_mode == TouchEventMode) {
        QTouchEvent::TouchPoint tp = m_touchPoints.value(sender());

        tp.setState(Qt::TouchPointReleased);

        QRectF rect(point.x() - 16, point.y() - 16, 32, 32);
        tp.setRect(rect);
        tp.setSceneRect(rect);
        tp.setScreenRect(rect);
        tp.setLastPos(tp.pos());
        tp.setStartPos(point);
        tp.setPressure(0);
        m_touchPoints.insert(sender(), tp);

        Qt::TouchPointStates states = tp.state();
        QEvent::Type type = QEvent::TouchEnd;
        if (m_touchPoints.size() > 1) {
            type = QEvent::TouchUpdate;
            states |= Qt::TouchPointStationary;
        }

        QTouchEvent te(type,
                       m_touchDevice,
                       Qt::NoModifier,
                       states,
                       m_touchPoints.values());
        te.setTimestamp(m_eta->elapsed());

        m_touchPoints.remove(sender());

        emit touchEvent(te);
    } else {
        QMouseEvent me(QEvent::MouseButtonRelease,
                       point,
                       Qt::LeftButton,
                       Qt::NoButton,
                       Qt::NoModifier);
        emit mouseEvent(me);
    }
}

EventWorker::EventWorker(const QVariantList &actions, QAMouseEngine *engine)
    : QObject(nullptr)
    , m_actions(actions)
    , m_engine(engine)
{
    qDebug() << "Created EventWorker" << this;
}

EventWorker::~EventWorker()
{
    qDebug() << "Deleted EventWorker" << this;
}

EventWorker* EventWorker::PerformTouchAction(const QVariantList &actions, QAMouseEngine *engine)
{
    EventWorker *worker = new EventWorker(actions, engine);
    QThread *thread = new QThread;
    connect(thread, &QThread::started, worker, &EventWorker::start);
    connect(worker, &EventWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &EventWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    worker->moveToThread(thread);
    thread->start();
    return worker;
}

void EventWorker::start()
{
    QPointF previousPoint;

    for (const QVariant &actionVar : m_actions) {
        const QVariantMap actionMap = actionVar.toMap();
        const QString action = actionMap.value(QStringLiteral("action")).toString();
        const QVariantMap options = actionMap.value(QStringLiteral("options")).toMap();

        if (action == QLatin1String("wait")) {
            const int delay = options.value(QStringLiteral("ms")).toInt();
            qDebug() << "waiting for" << delay;
            QEventLoop loop;
            QTimer::singleShot(delay, &loop, &QEventLoop::quit);
            loop.exec();
        } else if (action == QLatin1String("longPress")) {
            const int posX = options.value(QStringLiteral("x")).toInt();
            const int posY = options.value(QStringLiteral("y")).toInt();
            QPoint point(posX, posY);

            if (options.contains(QStringLiteral("element"))) {
                auto platform = QAEngine::instance()->getPlatform();
                if (auto item = platform->getObject(options.value(QStringLiteral("element")).toString())) {
                    point = platform->getAbsPosition(item);
                }
            }

            sendPress(point);
            previousPoint = point;

            const int delay = options.value(QStringLiteral("duration")).toInt();
            qDebug() << "waiting for" << delay;
            QEventLoop loop;
            QTimer::singleShot(delay, &loop, &QEventLoop::quit);
            loop.exec();
        } else if (action == QLatin1String("press")) {
            const int posX = options.value(QStringLiteral("x")).toInt();
            const int posY = options.value(QStringLiteral("y")).toInt();
            QPoint point(posX, posY);

            if (options.contains(QStringLiteral("element"))) {
                auto platform = QAEngine::instance()->getPlatform();
                if (auto item = platform->getObject(options.value(QStringLiteral("element")).toString())) {
                    point = platform->getAbsPosition(item);
                }
            }

            sendPress(point);
            previousPoint = point;
        } else if (action == QLatin1String("moveTo")) {
            const int posX = options.value(QStringLiteral("x")).toInt();
            const int posY = options.value(QStringLiteral("y")).toInt();
            QPoint point(posX, posY);

            if (options.contains(QStringLiteral("element"))) {
                auto platform = QAEngine::instance()->getPlatform();
                if (auto item = platform->getObject(options.value(QStringLiteral("element")).toString())) {
                    point = platform->getAbsPosition(item);
                }
            }

            const int duration = options.value(QStringLiteral("duration"), 500).toInt();
            const int steps = options.value(QStringLiteral("steps"), 20).toInt();

            sendMove(previousPoint, point,  duration, steps);
            previousPoint = point;

        } else if (action == QLatin1String("release")) {
            sendRelease(previousPoint);
        } else if (action == QLatin1String("tap")) {
            const int posX = options.value(QStringLiteral("x")).toInt();
            const int posY = options.value(QStringLiteral("y")).toInt();
            QPoint point(posX, posY);

            if (options.contains(QStringLiteral("element"))) {
                auto platform = QAEngine::instance()->getPlatform();
                if (auto item = platform->getObject(options.value(QStringLiteral("element")).toString())) {
                    point = platform->getAbsPosition(item);
                }
            }

            const int count = options.value(QStringLiteral("count")).toInt();
            for (int i = 0; i < count; i++) {
                sendPress(point);
                sendRelease(point);
            }
            previousPoint = point;
        } else {
            qWarning() << Q_FUNC_INFO << action;
        }
    }

    emit finished();
}

void EventWorker::sendPress(const QPointF &point)
{
    qWarning()
        << Q_FUNC_INFO << point;

    emit pressed(point);
}

void EventWorker::sendRelease(const QPointF &point)
{
    qWarning()
        << Q_FUNC_INFO << point;

    emit released(point);
}

void EventWorker::sendRelease(const QPointF &point, int delay)
{
    qWarning()
        << Q_FUNC_INFO
        << point << delay;

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, [this, point]() {
        sendRelease(point);
    });
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(delay);
    loop.exec();
}

void EventWorker::sendMove(const QPointF &point)
{
    qWarning()
        << Q_FUNC_INFO
        << point;

    emit moved(point);
}

void EventWorker::sendMove(const QPointF &previousPoint, const QPointF &point, int duration, int moveSteps)
{
    qWarning()
        << Q_FUNC_INFO
        << previousPoint << point << duration << moveSteps;

    float stepSize = 5.0f;

    const float stepX = qAbs(point.x() - previousPoint.x()) / moveSteps;
    const float stepY = qAbs(point.y() - previousPoint.y()) / moveSteps;

    if (stepX > 0 && stepX < stepSize) {
        moveSteps = qAbs(qRound(point.x() - previousPoint.x())) / stepSize;
    } else if (stepY > 0 && stepY < stepSize) {
        moveSteps = qAbs(qRound(point.y() - previousPoint.y())) / stepSize;
    }

    auto *interpolator = QVariantAnimationPrivate::getInterpolator(QMetaType::QPointF);

    QPointF pointA = previousPoint;
    for (int currentMoveStep = 0; currentMoveStep < moveSteps; currentMoveStep++) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(&timer, &QTimer::timeout, this, [this, interpolator, &pointA, currentMoveStep, moveSteps, point, previousPoint]() {
            float progress = static_cast<float>(currentMoveStep) / moveSteps;

            QPointF pointB = interpolator(&previousPoint, &point, progress).toPointF();
            sendMove(pointB);
            pointA = pointB;
        });
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(duration / moveSteps);
        loop.exec();
    }
}
