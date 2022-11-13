/**
 * @brief DLL Entry point for our project
 */
#include <Windows.h>

#include "ReHamster.h"
#include "CrashHandlerReporter.h"
#include "Logger.h"

#ifndef _WIN32
#error "Supported only x86"
#endif


class CDynInitializer
{
public:
	CDynInitializer()
	{
		ReHamster::Core::Init();
	}

	~CDynInitializer()
	{
		ReHamster::Core::Shutdown();
	}
};
static CDynInitializer s_initializer;

#if 0
DWORD g_dwWorkerThreadId = 0x0;
static constexpr auto kDefaultStackSize = 0x0;

DWORD WINAPI WorkerThread(void* arg)
{
	ReHamster::Core::Init();
    DWORD retVal = ReHamster::Core::EntryPoint(arg);
	ReHamster::Core::Shutdown();
	return retVal;
}
#endif
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
#if 0
		CreateThread(nullptr, kDefaultStackSize, (LPTHREAD_START_ROUTINE)WorkerThread, nullptr, 0, &g_dwWorkerThreadId);
#endif
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        FreeLibraryAndExitThread(hinstDLL, 0);
    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}