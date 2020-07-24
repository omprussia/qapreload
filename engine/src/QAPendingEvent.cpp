// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "QAPendingEvent.hpp"

QAPendingEvent::QAPendingEvent(QObject *parent) : QObject(parent)
{

}

void QAPendingEvent::setCompleted()
{
    emit completed(this);
}
