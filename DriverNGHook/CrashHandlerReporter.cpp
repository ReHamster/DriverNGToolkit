#include "CrashHandlerReporter.h"
#include <Windows.h>
#include <DbgHelp.h>
#include "Logger.h"

static std::vector<ReHamster::CrashFunc> s_crashFuncs;

static bool isExceptionRequierMiniDump(EXCEPTION_POINTERS* frame)
{
    if (frame->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT  ||
        frame->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP)
    {
        return false;
    }
    return true;
}

static void WriteMiniDump(EXCEPTION_POINTERS* exception = nullptr)
{
    //
    //	Credits https://stackoverflow.com/questions/5028781/how-to-write-a-sample-code-that-will-crash-and-produce-dump-file
    //
    auto hDbgHelp = LoadLibraryA("dbghelp");
    if (hDbgHelp == nullptr)
        return;
    auto pMiniDumpWriteDump = (decltype(&MiniDumpWriteDump))GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
    if (pMiniDumpWriteDump == nullptr)
        return;

    char name[MAX_PATH];
    {
        auto nameEnd = name + GetModuleFileNameA(GetModuleHandleA(0), name, MAX_PATH);
        SYSTEMTIME t;
        GetSystemTime(&t);

        wsprintfA(nameEnd - strlen(".exe"),
                  "_%4d%02d%02d_%02d%02d%02d.dmp",
                  t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    }

    auto hFile = CreateFileA(name, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return;

    MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
    exceptionInfo.ThreadId = GetCurrentThreadId();
    exceptionInfo.ExceptionPointers = exception;
    exceptionInfo.ClientPointers = FALSE;

    auto dumped = pMiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory),
            exception ? &exceptionInfo : nullptr,
            nullptr,
            nullptr);

    CloseHandle(hFile);
}

static void NotifyAboutException(EXCEPTION_POINTERS* exceptionInfoFrame)
{
    MessageBox(
        NULL,
        "We got an fatal error.\nMinidump will be saved near exe.\nMore details in dev console.",
        "Driver San Francisco | DriverNGHook",
        MB_ICONERROR | MB_OK
    );

    MsgError("******************************************************************************\n");
    MsgError("                                 FATAL ERROR!								  \n");
    MsgError("******************************************************************************\n");
    MsgError("Exception code   : 0x%x\n",   exceptionInfoFrame->ExceptionRecord->ExceptionCode);
    MsgError("Exception flags  : 0x%x\n",   exceptionInfoFrame->ExceptionRecord->ExceptionFlags);
    MsgError("Exception addr   : 0x%x\n", (DWORD)exceptionInfoFrame->ExceptionRecord->ExceptionAddress);
    MsgError("Registers:\n");
    MsgError("             EAX : 0x%x\n", exceptionInfoFrame->ContextRecord->Eax);
    MsgError("             EBX : 0x%x\n", exceptionInfoFrame->ContextRecord->Ebx);
    MsgError("             EDX : 0x%x\n", exceptionInfoFrame->ContextRecord->Edx);
    MsgError("             ECX : 0x%x\n", exceptionInfoFrame->ContextRecord->Ecx);
    MsgError("             EDI : 0x%x\n", exceptionInfoFrame->ContextRecord->Edi);
    MsgError("             ESI : 0x%x\n", exceptionInfoFrame->ContextRecord->Esi);
    MsgError("             EBP : 0x%x\n", exceptionInfoFrame->ContextRecord->Ebp);
    MsgError("             EIP : 0x%x\n", exceptionInfoFrame->ContextRecord->Eip);
    MsgError("             ESP : 0x%x\n", exceptionInfoFrame->ContextRecord->Esp);
    MsgError("******************************************************************************\n");

    // Extra stuff
    for (auto& func : s_crashFuncs)
    {
        func();
    }

    WriteMiniDump(exceptionInfoFrame);
}

static LONG WINAPI ExceptionFilterWin32(EXCEPTION_POINTERS* exceptionInfoFrame)
{
    if (exceptionInfoFrame->ExceptionRecord->ExceptionCode < 0x80000000)
    {
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    if (isExceptionRequierMiniDump(exceptionInfoFrame))
    {
        NotifyAboutException(exceptionInfoFrame);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

namespace ReHamster
{
    CrashHandlerReporter::CrashHandlerReporter()
    {
    }

    void PureCallhandler(void)
    {
        MessageBox(
            NULL,
            "We got an fatal error.\nMinidump will be saved near exe.\nMore details in dev console.",
            "Driver San Francisco | DriverNGHook",
            MB_ICONERROR | MB_OK
        );
    }

    void CrashHandlerReporter::Install()
    {
        _set_purecall_handler(PureCallhandler);
        m_vectored = reinterpret_cast<std::intptr_t>(AddVectoredExceptionHandler(0UL, ExceptionFilterWin32));
    }

    void CrashHandlerReporter::AddCrashFunc(CrashFunc onCrash)
    {
        s_crashFuncs.push_back(std::move(onCrash));
    }

    CrashHandlerReporter::~CrashHandlerReporter()
    {
        RemoveVectoredContinueHandler(reinterpret_cast<PVOID>(m_vectored));
    }
}