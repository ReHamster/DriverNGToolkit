#include <Delegates/DX9Delegate.h>
#include <UI/DebugTools.h>

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <cmdlib.h>

namespace DriverNG
{
	namespace Constants
	{
		constexpr int		WAIT_FRAMES = 10;
	}

    namespace Globals
    {
        std::unique_ptr<DebugTools> g_pDebugTools = nullptr;
		IDirect3DDevice9*			g_d3dDevice = nullptr;
		HWND						g_focusWindow = nullptr;
		volatile ImGuiContext*		g_sharedImGui = nullptr;

		volatile bool				g_dataDrawn = false;

		std::mutex					g_drawMutex[2];
    }

    void DX9Delegate::OnInitialised(IDirect3DDevice9* device, HWND focusWindow)
    {
		if (!focusWindow)
			throw std::runtime_error{ "Failed to get HWND!" };

		Globals::g_d3dDevice = device;
		Globals::g_focusWindow = focusWindow;

		Globals::g_sharedImGui = ImGui::CreateContext();
		
		Msg("ImGui CreateContext context: %x\n", Globals::g_sharedImGui);

		ImGuiIO& io = ImGui::GetIO();

		io.MouseDrawCursor = true;
		io.MousePos = ImVec2(0.f, 0.f);
		io.MousePosPrev = io.MousePos;
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(Globals::g_focusWindow);
		ImGui_ImplDX9_Init(Globals::g_d3dDevice);

        Globals::g_pDebugTools = std::make_unique<DebugTools>();
    }

    void DX9Delegate::OnBeginScene(IDirect3DDevice9* device)
    {
        (void)device;
    }

    void DX9Delegate::OnEndScene(IDirect3DDevice9* device)
    {
		{
			//std::lock_guard g(Globals::g_drawMutex[1]);

			int waitFrames = 0;

			while (Globals::g_dataDrawn && waitFrames < Constants::WAIT_FRAMES)
			{
				Sleep(1);
				waitFrames++;
			}
		}

		{
			std::lock_guard g(Globals::g_drawMutex[0]);

			ImGui::SetCurrentContext((ImGuiContext*)Globals::g_sharedImGui);
			auto drawData = ImGui::GetDrawData();

			if (drawData)
			{
				ImGui_ImplDX9_RenderDrawData(drawData);
				Globals::g_dataDrawn = true;
			}
		}
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