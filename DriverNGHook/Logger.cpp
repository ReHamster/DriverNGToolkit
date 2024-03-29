#include "Logger.h"
#include "LoggerImpl.h"

#include <Windows.h>

namespace ReHamster
{
    void Logger::Setup()
    {
		Install_ConsoleSpewFunction();
    }

    void Logger::Shutdown()
    {
		Remove_ConsoleSpewFunction();
    }

    void Logger::Assert(bool condition,
                        const std::string& function,
                        const std::string& file,
                        unsigned int line,
                        const std::string& message,
                        const std::string& condition_str)
    {
        if (!condition)
        {
			static char msg[4096];
            sprintf(msg,
                    "ASSERTION FAILED!\n\n"
                    "Condition: %s\n"
                    "Function: %s\n"
                    "Line: %d\n"
                    "File: %s\n",
                    condition_str.data(),
                    function.data(),
                    line,
                    file.data()
            );

            MessageBox(nullptr, msg, "ASSERTION FAILED!", MB_ICONEXCLAMATION | MB_OK);
            ExitProcess(EXIT_FAILURE);
        }
    }
}