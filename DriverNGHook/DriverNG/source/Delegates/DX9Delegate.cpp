#include <Delegates/DX9Delegate.h>
#include <UI/DebugTools.h>

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <spdlog/spdlog.h>

namespace DriverNG
{
    namespace Globals
    {
        std::unique_ptr<DebugTools> g_pDebugTools = nullptr;
    }

    void DX9Delegate::OnInitialised(IDirect3DDevice9* device, HWND focusWindow)
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        io.MouseDrawCursor = true;
        io.MousePos = ImVec2(0.f, 0.f);
        io.MousePosPrev = io.MousePos;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        ImGui::StyleColorsDark();

        if (!focusWindow)
            throw std::runtime_error { "Failed to get HWND!" };

        ImGui_ImplWin32_Init(focusWindow);
        ImGui_ImplDX9_Init(device);

        Globals::g_pDebugTools = std::make_unique<DebugTools>();
    }

    void DX9Delegate::OnBeginScene(IDirect3DDevice9* device)
    {
        (void)device;
    }

    void DX9Delegate::OnEndScene(IDirect3DDevice9* device)
    {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (Globals::g_pDebugTools)
        {
            const bool isDebugToolsVisible = Globals::g_pDebugTools->IsVisible();

			Globals::g_pDebugTools->Update();

            ImGuiIO& io = ImGui::GetIO();
            io.MouseDrawCursor = isDebugToolsVisible;
        }

        ImGui::EndFrame();

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    void DX9Delegate::OnDeviceLost()
    {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }

    void DX9Delegate::OnReset(IDirect3DDevice9* device)
    {
        (void)device;
    }

    void DX9Delegate::OnDeviceRestored(IDirect3DDevice9* device)
    {
        ImGui_ImplDX9_CreateDeviceObjects();
    }
}