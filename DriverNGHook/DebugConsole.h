#pragma once

#include <string>

namespace ReHamster
{
    class DebugConsole
    {
        static volatile bool g_bIsCreated;
    public:
        static void Create(const std::string& caption);
    };
}