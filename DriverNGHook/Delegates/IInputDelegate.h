#pragma once

#include <cstdint>
#include <Windows.h>

namespace DriverNG
{
    class IInputDelegate
    {
    public:
        virtual ~IInputDelegate() noexcept = default;

        virtual void setGameInputBlocked(bool block) { m_inputBlocked = block; }
        virtual bool isGameInputBlocked() const { return m_inputBlocked; }

        virtual void setKeyState(int keyCode, int state) {}
        virtual void setMouseKeyState(int keyId, bool state) {}
        virtual void setMouseWheelState(float dt) {}

        virtual bool onWindowsEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return !m_inputBlocked; };

    protected:
        bool m_inputBlocked{ false };
    };
}