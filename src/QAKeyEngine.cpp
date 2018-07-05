#include "QAKeyEngine.hpp"

#include <QKeyEvent>

QAKeyEngine::QAKeyEngine(QObject *parent)
    : QObject(parent)
{
}

void QAKeyEngine::pressEnter(int count)
{
    for (int i = 0; i < count; i++) {
        sendPress('\b', Qt::Key_Enter);
        sendRelease('\b', Qt::Key_Enter);
    }
}

void QAKeyEngine::pressBackspace(int count)
{
    for (int i = 0; i < count; i++) {
        sendPress('\b', Qt::Key_Backspace);
        sendRelease('\b', Qt::Key_Backspace);
    }
}

void QAKeyEngine::pressKeys(const QString &keys)
{
    for (const QChar key : keys) {
        sendPress(key);
        sendRelease(key);
    }
}

void QAKeyEngine::sendPress(const QChar &nkey, int key)
{
    QKeyEvent event(QKeyEvent::KeyPress,
                    key,
                    Qt::NoModifier,
                    QString(nkey));

    emit triggered(&event);
}

void QAKeyEngine::sendRelease(const QChar &nkey, int key)
{
    QKeyEvent event(QKeyEvent::KeyRelease,
                    key,
                    Qt::NoModifier,
                    QString(nkey));

    emit triggered(&event);
}
