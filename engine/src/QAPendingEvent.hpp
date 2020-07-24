// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#ifndef QAPENDINGEVENT_HPP
#define QAPENDINGEVENT_HPP

#include <QObject>

class QAPendingEvent : public QObject
{
    Q_OBJECT
public:
    explicit QAPendingEvent(QObject *parent = nullptr);

signals:
    void completed(QAPendingEvent *pending);

public slots:
    void setCompleted();
};

#endif // QAPENDINGEVENT_HPP
