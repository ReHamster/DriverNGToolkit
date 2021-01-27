#pragma once
#include <functional>

namespace DriverNG
{
    typedef void (*CallLuaFunction_t)(const char* func, const char* sig, ...);
	
    class ILuaDelegate
    {
    public:
        static ILuaDelegate& GetInstance();
    	
        virtual ~ILuaDelegate() noexcept = default;
        virtual void OnInitialised(CallLuaFunction_t callFunc) = 0;
        virtual void DoCommands() = 0;
        virtual int Push(std::function<int()> f) = 0;
    
        virtual CallLuaFunction_t GetCallLuaFunction() = 0;
    };
}