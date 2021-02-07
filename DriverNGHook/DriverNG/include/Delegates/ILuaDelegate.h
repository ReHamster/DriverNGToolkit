#pragma once
#include <functional>
#include <string>
#include <sol/sol.hpp>

struct lua_State;

namespace DriverNG
{
    typedef void (*CallLuaFunction_t)(const char* func, const char* sig, ...);
   
    class ILuaDelegate
    {
    public:
        static ILuaDelegate& GetInstance();
    	
        virtual ~ILuaDelegate() noexcept = default;
        virtual void OnInitialised(lua_State* gameState, CallLuaFunction_t callFunc) = 0;

        virtual void DoCommands() = 0;

        virtual sol::protected_function_result ExecuteString(const std::string& code) = 0;
        virtual sol::protected_function_result ExecuteFile(const std::string& filename) = 0;

        virtual int Push(std::function<int()> f) = 0;

        virtual CallLuaFunction_t GetCallLuaFunction() = 0;
    };
}