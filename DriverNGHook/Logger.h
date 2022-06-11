#pragma once

#include <string_view>

namespace ReHamster
{
    class Logger
    {
    public:
        static void Setup();
        static void Shutdown();
        static void Assert(bool condition, std::string_view function, std::string_view file, unsigned int line, std::string_view message, std::string_view condition_str);
    };
}

// simple message output
void Msg(const char* fmt, ...);

// Error messages
void MsgError(const char* fmt, ...);

// Info messages
void MsgInfo(const char* fmt, ...);

//Warning messages
void MsgWarning(const char* fmt, ...);

// Good messages
void MsgAccept(const char* fmt, ...);

#define RH_ASSERT2(condition, message) \
    do {                              \
        ReHamster::Logger::Assert((condition), __FUNCTION__, __FILE__, __LINE__, message, #condition); \
    } while (0);

#define RH_ASSERT(condition) \
    do {                     \
        ReHamster::Logger::Assert((condition), __FUNCTION__, __FILE__, __LINE__, "", #condition); \
    } while (0);