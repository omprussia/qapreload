#include "WinInjector.hpp"
#include <windows.h>
#include <QDebug>

bool Injector::injectDll(long long processId, const char *dllPath)
{
    HANDLE h_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);             //retrieving a handle to the target process

    char fullDLLPath[_MAX_PATH] = {0};                                                      //getting the full path of the dll file
    GetFullPathNameA(dllPath, _MAX_PATH, fullDLLPath, NULL);
    qDebug() << Q_FUNC_INFO << "Loading dll " << fullDLLPath;

    LPVOID DLLPath_addr = VirtualAllocEx(h_process, NULL, _MAX_PATH,
                          MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);                  //allocating memory in the target process

    if (DLLPath_addr == NULL) {
        qDebug() << Q_FUNC_INFO << "Couldn't allocate memory, please restart with administrator privileges\n";
        return false;
    }

    bool success = WriteProcessMemory(h_process, DLLPath_addr, fullDLLPath,
                       strlen(fullDLLPath), NULL);                                    //writing the dll path into that memory

    qDebug() << Q_FUNC_INFO << "WriteProcessMemory is: " << success << DLLPath_addr;

    LPVOID loadLibraryAddress = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

    qDebug() << Q_FUNC_INFO << "loadLibraryAddress is:" << loadLibraryAddress;


    HANDLE h_rThread = CreateRemoteThread(h_process, NULL, 0,                         //starting a remote execution thread at LoadLibraryA
                       (LPTHREAD_START_ROUTINE)loadLibraryAddress,
                                           DLLPath_addr, 0, NULL);                    //  and passing the dll path as an argument
    qDebug() << Q_FUNC_INFO << "RemoteThread is " << h_rThread;

    DWORD wait_result = WaitForSingleObject(h_rThread, INFINITE);
    qDebug() << wait_result;                                         //waiting for it to be finished

    DWORD exit_code = 0;

    BOOL have_exit_code = GetExitCodeThread(h_rThread, &exit_code);
    qDebug() << have_exit_code << exit_code;

    CloseHandle(h_rThread);                                                           //freeing the injected thread handle,
    VirtualFreeEx(h_process, DLLPath_addr, 0, MEM_RELEASE);                        //... and the memory allocated for the DLL path,
    CloseHandle(h_process);                                                           //... and the handle for the target process

    qDebug() << Q_FUNC_INFO << "Dll successfully loaded\n";
    return (HANDLE)exit_code;

}
