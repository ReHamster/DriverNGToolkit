#pragma once

#include <cstdint>
#include <functional>

namespace ReHamster
{
    using CrashFunc = std::function<void()>;

    class CrashHandlerReporter
    {
        std::intptr_t m_vectored{ 0 };
    public:
        CrashHandlerReporter();
        ~CrashHandlerReporter();

        void Install();

        void AddCrashFunc(CrashFunc onCrash);
    };
}