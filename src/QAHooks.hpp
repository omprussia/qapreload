#ifndef QAHOOKS_HPP
#define QAHOOKS_HPP

#include <QtGlobal>

extern quintptr Q_CORE_EXPORT qtHookData[];

class QObject;
class QAHooks
{
public:
    enum HookIndex {
        HookDataVersion = 0,
        HookDataSize = 1,
        QtVersion = 2,
        AddQObject = 3,
        RemoveQObject = 4,
        Startup = 5,
        TypeInformationVersion = 6,
        LastHookIndex
    };

    static void installHooks();
    static void removeHooks();

    static void applicationStarted();
    static void objectAdded(QObject *o);
    static void objectRemoved(QObject *o);

};

#endif // QAHOOKS_HPP
