#include "Patches/All/InputDevicesPatches.h"
#include "Delegates/IInputDelegate.h"
#include "Delegates/ILuaDelegate.h"

#include <Windows.h>
#include "Logger.h"
#include "dinput_wrap.h"

namespace Consts
{
    enum DirectInput8AVFTableIndex : size_t
    {
        CreateDevice = 3,
    };

    static constexpr std::intptr_t k_DirectInput8CreateAddr = 0x00853268;
    static constexpr std::intptr_t kOriginalWndProcAddr = 0x008641C0;
    static constexpr std::intptr_t kRegisterClassAAddr = 0x00864FBE;
}

namespace Original
{
    typedef HRESULT(__stdcall* CreateDevice_t)(IDirectInput8A*, REFGUID, LPDIRECTINPUTDEVICE8A*, LPUNKNOWN);
    CreateDevice_t		originalCreateDevice;

}

namespace Globals
{
    static std::unique_ptr<DriverNG::IInputDelegate> g_pInputDelegate = nullptr;

    static WrapDirectInputDevice* g_pKeyboardDevice = nullptr;
    static WrapDirectInputDevice* g_pMouseDevice = nullptr;

    static std::unique_ptr<HF::Hook::VFHook<IDirectInput8A>> g_DirectInput8A_CreateDevice{ nullptr };

    static void SetupDirectInputFactoryHooks(IDirectInput8A* factory);
}

namespace Callbacks
{
    LRESULT WINAPI Hamster_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        typedef LRESULT(__stdcall* WndProc_t)(HWND, UINT, WPARAM, LPARAM);
        auto hamsterWndProc = (WndProc_t)Consts::kOriginalWndProcAddr;


        if (Globals::g_pInputDelegate)
        {
            switch (msg)
            {
                case WM_KEYDOWN:
                    Globals::g_pInputDelegate->setKeyState(static_cast<int>(wParam), true);
                    break;
                case WM_KEYUP:
                    Globals::g_pInputDelegate->setKeyState(static_cast<int>(wParam), false);
                    break;
                default: /* unhandled event */ break;
            }

            if(Globals::g_pInputDelegate->onWindowsEvent(hWnd, msg, wParam, lParam) == false)
            {
                if (Globals::g_pKeyboardDevice)
                    Globals::g_pKeyboardDevice->Enable(false);

                if (Globals::g_pMouseDevice)
                    Globals::g_pMouseDevice->Enable(false);
            	
                switch (msg)
                {
                case WM_INPUT:
                case WM_CHAR:
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_MOUSEMOVE:
                case WM_MOUSEWHEEL:
                case WM_MOUSEHWHEEL:
				case WM_LBUTTONDOWN:
				case WM_LBUTTONUP:
				case WM_LBUTTONDBLCLK:
				case WM_RBUTTONDOWN:
				case WM_RBUTTONUP:
				case WM_RBUTTONDBLCLK:
				case WM_MBUTTONDOWN:
				case WM_MBUTTONUP:
				case WM_MBUTTONDBLCLK:
                    return 0;
                }
            }
        }

        if (Globals::g_pKeyboardDevice)
            Globals::g_pKeyboardDevice->Enable(true);
    	
        if (Globals::g_pMouseDevice)
            Globals::g_pMouseDevice->Enable(true);

        // disable stupid clipping
        switch (msg)
        {
        case WM_MOVE:
        case WM_SIZE:
        case WM_ACTIVATE:
        case WM_EXITSIZEMOVE:
            return 0;
        }

        LRESULT result = hamsterWndProc(hWnd, msg, wParam, lParam);

        return result;
    }

    ATOM __stdcall RegisterClassA_Hooked(WNDCLASSA* wndClass)
    {
        MsgInfo("Window class registered! Event loop function hooked!\n");

        wndClass->lpfnWndProc = Hamster_WndProc;
        return RegisterClassA(wndClass);
    }

    HRESULT __stdcall DirectInput8Create_Hooked(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
    {
        void* ret = 0;
        
        HRESULT hr =  DirectInput8Create(hinst, dwVersion, riidltf, &ret, punkOuter);
        
        if (hr != S_OK)
            return hr;

        IDirectInput8A* realInterface = reinterpret_cast<IDirectInput8A*>(ret);

        Globals::SetupDirectInputFactoryHooks(realInterface);
        *ppvOut = realInterface;

        return hr;
    }

    HRESULT __stdcall DirectInput8A_CreateDevice(IDirectInput8A* factory, REFGUID guid, LPDIRECTINPUTDEVICE8A* ppReturnedDeviceInterface, LPUNKNOWN punkOuter)
    {
        if (guid != GUID_SysKeyboard && guid != GUID_SysMouse)
        {
            return Original::originalCreateDevice(factory, guid, ppReturnedDeviceInterface, punkOuter);
        }
        else 
        {
            IDirectInputDevice8A* device = nullptr;

            HRESULT hr = Original::originalCreateDevice(factory, guid, &device, punkOuter);
            if (hr != DI_OK)
                return hr;

            WrapDirectInputDevice* wrappedDevice = new WrapDirectInputDevice(device, guid);
        	
            *ppReturnedDeviceInterface = wrappedDevice;

        	if(guid == GUID_SysKeyboard)
        	{
				MsgInfo("New Wrap DirectInput Keyboard created %x\n", reinterpret_cast<std::intptr_t>(device));
                Globals::g_pKeyboardDevice = wrappedDevice;
        	}
            else if (guid == GUID_SysMouse)
            {
				MsgInfo("New Wrap DirectInput Mouse created %x\n", reinterpret_cast<std::intptr_t>(device));
                Globals::g_pMouseDevice = wrappedDevice;
            }
        	
            return DI_OK;
        }
    }

}

namespace Globals
{
    static void SetupDirectInputFactoryHooks(IDirectInput8A* factory)
    {
        g_DirectInput8A_CreateDevice = HF::Hook::HookVirtualFunction<IDirectInput8A, Consts::DirectInput8AVFTableIndex::CreateDevice>(factory, &Callbacks::DirectInput8A_CreateDevice);

        if (reinterpret_cast<DWORD>(Original::originalCreateDevice) != g_DirectInput8A_CreateDevice->getOriginalPtr())
            Original::originalCreateDevice = reinterpret_cast<Original::CreateDevice_t>(g_DirectInput8A_CreateDevice->getOriginalPtr());
    }
}

namespace DriverNG
{
    InputDevicesPatches::InputDevicesPatches(std::unique_ptr<IInputDelegate>&& delegate)
    {
        Globals::g_pInputDelegate = std::move(delegate);
    }

    std::string_view InputDevicesPatches::GetName() const { return "Input"; }

    bool InputDevicesPatches::Apply(const ModPack& modules)
    {
        if (auto process = modules.process.lock())
        {
            // Do not revert this patch!
            if (!HF::Hook::FillMemoryByNOPs(process, Consts::kRegisterClassAAddr, kRegisterClassAPatchSize))
            {
                MsgError("Failed to cleanup memory");
                return false;
            }

            m_registerClassAHook = HF::Hook::HookFunction<ATOM(__stdcall*)(WNDCLASSA*), kRegisterClassAPatchSize>(
                    process,
                    Consts::kRegisterClassAAddr,
                    &Callbacks::RegisterClassA_Hooked,
                    {},
                    {});

            if (!m_registerClassAHook->setup())
            {
				MsgError("Failed to setup patch to RegisterClassA!\n");
                return false;
            }

        	// hook DirectInput
            // Do not revert this patch!
            if (!HF::Hook::FillMemoryByNOPs(process, Consts::k_DirectInput8CreateAddr, kDirectInput8CreateSize))
            {
				MsgError("Failed to cleanup memory\n");
                return false;
            }

            m_directInput8CreateHook = HF::Hook::HookFunction<HRESULT (__stdcall*)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN), kDirectInput8CreateSize>(
                process,
                Consts::k_DirectInput8CreateAddr,
                &Callbacks::DirectInput8Create_Hooked,
                {},
                {});

            if (!m_directInput8CreateHook->setup())
            {
				MsgError("Failed to setup patch to DirectInputCreate8!\n");
                return false;
            }

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void InputDevicesPatches::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);

        m_registerClassAHook->remove();
        m_directInput8CreateHook->remove();

        Globals::g_pInputDelegate.reset(nullptr);
    }
}