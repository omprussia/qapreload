// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "QAKeyEngine.hpp"
#include "QAPendingEvent.hpp"

#include <QChar>
#include <QKeyEvent>
#include <QLoggingCategory>
#include <QDebug>

namespace {

int seleniumKeyToQt(ushort key)
{
    if (key < 0xe000 || key > 0xe03d) {
        return 0;
    }

    key -= 0xe000;
    static int seleniumKeys[] {
        0, // u'\ue000'
        Qt::Key_Cancel,
        Qt::Key_Help,
        Qt::Key_Backspace,
        Qt::Key_Tab,
        Qt::Key_Clear,
        Qt::Key_Return,
        Qt::Key_Enter,
        Qt::Key_Shift,
        Qt::Key_Control,
        Qt::Key_Alt,
        Qt::Key_Pause,
        Qt::Key_Escape,
        Qt::Key_Space,
        Qt::Key_PageUp,
        Qt::Key_PageDown, // u'\ue00f'
        Qt::Key_End,
        Qt::Key_Home,
        Qt::Key_Left,
        Qt::Key_Up,
        Qt::Key_Right,
        Qt::Key_Down,
        Qt::Key_Insert,
        Qt::Key_Delete,
        Qt::Key_Semicolon,
        Qt::Key_Equal,
        Qt::Key_0,
        Qt::Key_1,
        Qt::Key_2,
        Qt::Key_3,
        Qt::Key_4,
        Qt::Key_5, // u'\ue01f'
        Qt::Key_6,
        Qt::Key_7,
        Qt::Key_8,
        Qt::Key_9,
        Qt::Key_Asterisk,
        Qt::Key_Plus,
        Qt::Key_Colon,
        Qt::Key_Minus,
        Qt::Key_Period,
        Qt::Key_Slash, // u'\ue029'
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        Qt::Key_F1, // u'\ue031'
        Qt::Key_F2,
        Qt::Key_F3,
        Qt::Key_F4,
        Qt::Key_F5,
        Qt::Key_F6,
        Qt::Key_F7,
        Qt::Key_F8,
        Qt::Key_F9,
        Qt::Key_F10,
        Qt::Key_F11,
        Qt::Key_F12,
        Qt::Key_Meta,
    };
    return seleniumKeys[key];
}

}

QAKeyEngine::QAKeyEngine(QObject *parent)
    : QObject(parent)
{
}

QAPendingEvent *QAKeyEngine::pressEnter(int count)
{
    QAPendingEvent *pending = new QAPendingEvent(this);

    for (int i = 0; i < count; i++) {
        sendPress('\n', Qt::Key_Enter);
        sendRelease('\n', Qt::Key_Enter);
    }

    QMetaObject::invokeMethod(pending, "setCompleted", Qt::QueuedConnection);
    return pending;
}

QAPendingEvent *QAKeyEngine::pressBackspace(int count)
{
    QAPendingEvent *pending = new QAPendingEvent(this);

    for (int i = 0; i < count; i++) {
        sendPress('\b', Qt::Key_Backspace);
        sendRelease('\b', Qt::Key_Backspace);
    }

    QMetaObject::invokeMethod(pending, "setCompleted", Qt::QueuedConnection);
    return pending;
}

QAPendingEvent *QAKeyEngine::pressKeys(const QString &keys)
{
    QAPendingEvent *pending = new QAPendingEvent(this);

    for (const QChar &key : keys) {
        sendPress(key);
        sendRelease(key);
    }

    QMetaObject::invokeMethod(pending, "setCompleted", Qt::QueuedConnection);
    return pending;
}

void QAKeyEngine::performChainActions(const QVariantList &actions)
{
    Qt::KeyboardModifiers mods;
    for (const QVariant &actionVar : actions.first().toMap().value(QStringLiteral("actions")).toList()) {
        const QVariantMap action = actionVar.toMap();
        QKeyEvent::Type eventType = QKeyEvent::KeyPress;
        const QString type = action.value(QStringLiteral("type")).toString();

        QString value = action.value(QStringLiteral("value")).toString();
        const int key = seleniumKeyToQt(value.at(0).unicode());
        bool keyUp = type == QLatin1String("keyUp");
        if (key) {
            value = QString();

            if (key == Qt::Key_Control) {
                mods ^= Qt::ControlModifier;
            } else if (key == Qt::Key_Alt) {
                mods ^= Qt::AltModifier;
            } else if (key == Qt::Key_Shift) {
                mods ^= Qt::ShiftModifier;
            } else if (key == Qt::Key_Meta) {
                mods ^= Qt::MetaModifier;
            } else if (key >= Qt::Key_0 && key <= Qt::Key_9) {
                mods ^= Qt::KeypadModifier;
            }
        }
        if (keyUp) {
            eventType = QKeyEvent::KeyRelease;
        }

        QKeyEvent event(eventType,
                        key,
                        mods,
                        value);

        emit triggered(&event);
    }
}

void QAKeyEngine::sendPress(const QChar &text, int key)
{
    QKeyEvent event(QKeyEvent::KeyPress,
                    key,
                    Qt::NoModifier,
                    QString(text));

    emit triggered(&event);
}

void QAKeyEngine::sendRelease(const QChar &text, int key)
{
    QKeyEvent event(QKeyEvent::KeyRelease,
                    key,
                    Qt::NoModifier,
                    QString(text));

    emit triggered(&event);
}
