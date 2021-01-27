#pragma once

#include <Delegates/IInputDelegate.h>

namespace DriverNG
{
    class ImGuiInputDelegate final : public IInputDelegate
    {
    public:
        void setKeyState(int keyCode, int state) override;
        void setMouseKeyState(int keyId, bool state) override;
        void setMouseWheelState(float dt) override;

        bool onWindowsEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
    };
}