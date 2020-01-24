#ifndef WININJECTOR_HPP
#define WININJECTOR_HPP

#include <QString>
#include <string>
#include <windows.h>

class Injector
{
public:
    /**
    * Loads a DLL into the remote process
    * @Return true on sucess, false on failure
    */
    bool injectDll(qint64 processId, const char *dllPath);
private:
};

#endif // WININJECTOR_HPP
