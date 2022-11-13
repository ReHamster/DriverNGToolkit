#pragma once

#include <cstdint>

namespace ReHamster
{
    class CrashHandlerReporter
    {
        std::intptr_t m_prevHandler{ 0 };
    public:
        CrashHandlerReporter();
        ~CrashHandlerReporter();

        void Install();
    };
}