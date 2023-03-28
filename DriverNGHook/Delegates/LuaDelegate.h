#pragma once

#include <Delegates/ILuaDelegate.h>
#include <sol/sol.hpp>

namespace DriverNG
{
    class LuaDelegate final : public ILuaDelegate
    {
    public:
        void                                OnInitialised(lua_State* gameState, CallLuaFunction_t callFunc) override;
        void                                OnDeleted() override;

        void                                DoCommands() override;
        bool                                IsOnlineGame() override;

		void								BeginRender() override;
		void								EndRender() override;

        bool                                IsDeveloperConsoleAllowed() override;

        sol::protected_function_result      ExecuteString(const std::string& code);
        sol::protected_function_result      ExecuteFile(const std::string& filename);

        int                                 Push(std::function<int()> f) override;

        CallLuaFunction_t                   GetCallLuaFunction() override;

        void                                PrintLuaStackTrace();

    protected:

        bool IsValidLuaState() const;

        void InitializeGameDevelopmentLib();

        CallLuaFunction_t m_callLuaFunc{ nullptr };

        lua_State* m_gameState;

        sol::state m_luaState;

        bool m_allowCustomGameScripts{ false };
        bool m_allowDeveloperConsole{ false };
    };
}