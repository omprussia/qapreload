// Copyright (c) 2020 Open Mobile Platform LLС.
#ifndef WININJECTOR_HPP
#define WININJECTOR_HPP

class Injector
{
public:
    /**
    * Loads a DLL into the remote process
    * @Return true on sucess, false on failure
    */
    static bool injectDll(long long processId, const char *dllPath);
private:
};

#endif // WININJECTOR_HPP
