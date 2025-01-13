#pragma once
#include <functional>
#include <string>

struct lua_State;

namespace DriverNG
{
    typedef void (*CallLuaFunction_t)(const char* func, const char* sig, ...);
   
    class ILuaDelegate
    {
    public:
        static ILuaDelegate& GetInstance();
    	
        virtual ~ILuaDelegate() noexcept = default;
        virtual void OnInitialised(lua_State* gameState, CallLuaFunction_t callFunc, const char* profileName) = 0;
        virtual void  OnDeleted() = 0;

        virtual void DoCommands(lua_State* gameState) = 0;

		virtual void BeginRender() = 0;
		virtual void EndRender() = 0;

        virtual bool IsOnlineGame() = 0;

        virtual bool IsDeveloperConsoleAllowed() = 0;

        virtual void ExecuteString(const std::string& code) = 0;
        virtual void ExecuteFile(const std::string& filename) = 0;

        virtual int Push(std::function<int()> f) = 0;

        virtual CallLuaFunction_t GetCallLuaFunction() = 0;
    };
}