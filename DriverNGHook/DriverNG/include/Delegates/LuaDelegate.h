#pragma once

#include <Delegates/ILuaDelegate.h>

namespace DriverNG
{
    class LuaDelegate final : public ILuaDelegate
    {
    public:
        void    OnInitialised(CallLuaFunction_t callFunc) override;
        void    DoCommands() override;
        int     Push(std::function<int()> f) override;

        CallLuaFunction_t GetCallLuaFunction() override;

    protected:
        CallLuaFunction_t m_callLuaFunc{ nullptr };

        
    };
}