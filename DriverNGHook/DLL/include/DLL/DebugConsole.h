#pragma once

#include <string_view>

namespace ReHamster
{
    class DebugConsole
    {
        static volatile bool g_bIsCreated;
    public:
        static void Create(std::string_view caption);
    };
}