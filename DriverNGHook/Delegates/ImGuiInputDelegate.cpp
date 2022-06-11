#include "Delegates/ImGuiInputDelegate.h"
#include "UI/DebugTools.h"
#include "Logger.h"

#include <imgui.h>
#include <memory>


// Win32 message handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace DriverNG
{
    namespace Globals
    {
        extern std::unique_ptr<DebugTools> g_pDebugTools;
    }

    void ImGuiInputDelegate::setKeyState(int keyCode, int state)
    {
        if (keyCode == VK_F3 && !state && Globals::g_pDebugTools)
        {
            Globals::g_pDebugTools->ToggleVisibility();
        }
    }

    void ImGuiInputDelegate::setMouseKeyState(int keyId, bool state)
    {
        ImGuiIO& io = ImGui::GetIO();
        if (keyId >= 0 && keyId <= 1)
        {
            io.MouseDown[keyId] = state;
        }
    }

    void ImGuiInputDelegate::setMouseWheelState(float dt)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseWheel += dt;
    }

    bool ImGuiInputDelegate::onWindowsEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

    	if(Globals::g_pDebugTools)
            return !Globals::g_pDebugTools->IsVisible();
    
        return true;
    }
}