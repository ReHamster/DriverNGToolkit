#pragma once

#include <Delegates/ILuaDelegate.h>

struct lua_State;

namespace DriverNG
{
    class LuaDelegate final : public ILuaDelegate
    {
    public:
        void      OnInitialised(lua_State* gameState, CallLuaFunction_t callFunc) override;
        void      OnDeleted() override;

        void      DoCommands(lua_State* gameState) override;
        bool      IsOnlineGame() override;

		void	  BeginRender() override;
		void	  EndRender() override;

        bool      IsDeveloperConsoleAllowed() override;

        void      ExecuteString(const std::string& code);
        void      ExecuteFile(const std::string& filename);

        int                Push(std::function<int()> f) override;

        CallLuaFunction_t  GetCallLuaFunction() override;

        void               PrintLuaStackTrace();

    protected:

        bool IsValidLuaState(lua_State* gameState) const;

        void InitializeGameDevelopmentLib();

        CallLuaFunction_t m_callLuaFunc{ nullptr };

        lua_State* m_gameState;

        bool m_allowCustomGameScripts{ false };
        bool m_allowDeveloperConsole{ false };
    };
}