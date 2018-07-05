#include "QAPendingEvent.hpp"

QAPendingEvent::QAPendingEvent(QObject *parent) : QObject(parent)
{

}

void QAPendingEvent::setCompleted()
{
    emit completed(this);
}
