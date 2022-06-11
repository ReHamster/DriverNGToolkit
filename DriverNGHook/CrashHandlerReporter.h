#pragma once

#include <cstdint>

namespace ReHamster
{
    class CrashHandlerReporter
    {
        std::intptr_t m_prevHandler;
    public:
        CrashHandlerReporter();
        ~CrashHandlerReporter();
    };
}