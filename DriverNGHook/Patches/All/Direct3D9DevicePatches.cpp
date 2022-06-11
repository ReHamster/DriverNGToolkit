#include "Direct3D9DevicePatches.h"
#include "Delegates/IDirect3DDelegate.h"
#include "d3d9_wrap.h"

#include "Logger.h"

//-------------------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------------------

namespace Consts
{
    enum D3DAPIVFTableIndex : size_t
    {
        CreateDevice = 16,
    };

    static constexpr std::intptr_t kDirect3DCreate9Addr = 0x00BB036F;
    static constexpr std::intptr_t kDirect3DCreate9JmpAddr = 0x00CE4E78;
    
}

namespace Globals
{
    static std::unique_ptr<DriverNG::IDirect3DDelegate> g_pDelegate = nullptr;

    static void SetupD3DFactoryHooks(IDirect3D9* factory);
}

namespace Original
{
    typedef HRESULT(__stdcall* D3DCreateDevice_t)(IDirect3D9*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);
    D3DCreateDevice_t			originalCreateDevice;

}

namespace Callbacks
{
    /*__declspec(nothrow)*/ HRESULT __stdcall D3D_CreateDevice(IDirect3D9* factory, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
    {
        IDirect3DDevice9* device = nullptr;

        HRESULT result = Original::originalCreateDevice(factory, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &device);

        if (Globals::g_pDelegate)
        {
            Globals::g_pDelegate->OnInitialised(device, hFocusWindow);
        }
    	
        WrappedD3DDevice9* wrappedDevice = new WrappedD3DDevice9(device, hFocusWindow, Globals::g_pDelegate.get());
        wrappedDevice->LazyInit();    // TODO this can be moved later probably
    	
        *ppReturnedDeviceInterface = wrappedDevice;
    	
        MsgInfo("New Wrap D3D Device created %x\n", reinterpret_cast<std::intptr_t>(device));
        	
        return result;
    }
	
    IDirect3D9* __stdcall Direct3DCreate9_Hooked(UINT SDKVersion)
    {
        typedef IDirect3D9* (__stdcall* D3DCreate9FuncType)(UINT);
        auto d3dCreate9Func = (D3DCreate9FuncType)Consts::kDirect3DCreate9JmpAddr;
    	
		MsgInfo("Direct3DCreate9 captured, SDK version: %x\n", SDKVersion);
        IDirect3D9* d3d9factory = d3dCreate9Func(SDKVersion);
    	
        // TODO hook to the CreateDevice
    	if(d3d9factory != nullptr)
    	{
            Globals::SetupD3DFactoryHooks(d3d9factory);
    	}
    	
        return d3d9factory;
    }
}

namespace Globals
{
    static std::unique_ptr<HF::Hook::VFHook<IDirect3D9>>        g_Direct3D_CreateDevice_Hook{ nullptr };

    static std::unique_ptr<HF::Hook::VFHook<IDirect3DDevice9>>  g_Direct3DDevice_Present_Hook{ nullptr };
    static std::unique_ptr<HF::Hook::VFHook<IDirect3DDevice9>>  g_Direct3DDevice_BeginScene_Hook { nullptr };
    static std::unique_ptr<HF::Hook::VFHook<IDirect3DDevice9>>  g_Direct3DDevice_EndScene_Hook { nullptr };
    static std::unique_ptr<HF::Hook::VFHook<IDirect3DDevice9>>  g_Direct3DDevice_Reset_Hook { nullptr };

    static void SetupD3DFactoryHooks(IDirect3D9* factory)
    {
        g_Direct3D_CreateDevice_Hook = HF::Hook::HookVirtualFunction<IDirect3D9, Consts::D3DAPIVFTableIndex::CreateDevice>(factory, &Callbacks::D3D_CreateDevice);

        if (reinterpret_cast<DWORD>(Original::originalCreateDevice) != g_Direct3D_CreateDevice_Hook->getOriginalPtr())
            Original::originalCreateDevice = reinterpret_cast<Original::D3DCreateDevice_t>(g_Direct3D_CreateDevice_Hook->getOriginalPtr());
    }

}

namespace DriverNG
{
    Direct3D9DevicePatches::Direct3D9DevicePatches(std::unique_ptr<IDirect3DDelegate>&& delegate)
    {
        Globals::g_pDelegate = std::move(delegate);
    }

    std::string_view Direct3D9DevicePatches::GetName() const
    {
        return "D3D9 Hooking Patch";
    }

    bool Direct3D9DevicePatches::Apply(const ModPack& modules)
    {
        using namespace HF::Hook;
    	
        if (auto process = modules.process.lock())
        {
            
            // Do not revert this patch!
            if (!HF::Hook::FillMemoryByNOPs(process, Consts::kDirect3DCreate9Addr, kDirect3DCreate9PatchSize))
            {
                MsgError("Failed to cleanup memory\n");
                return false;
            }

            m_direct3DCreate9Hook = HF::Hook::HookFunction<IDirect3D9*(__stdcall*)(UINT), kDirect3DCreate9PatchSize>(
                process,
                Consts::kDirect3DCreate9Addr,
                &Callbacks::Direct3DCreate9_Hooked,
                {},
                {});

            if (!m_direct3DCreate9Hook->setup())
            {
                //m_wintelMouseCtorHook->remove();
				MsgError("Failed to setup patch to Direct3DCreate9!\n");
                return false;
            }

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void Direct3D9DevicePatches::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);

        Globals::g_pDelegate = nullptr;
        Globals::g_Direct3D_CreateDevice_Hook.reset(nullptr);
        Globals::g_Direct3DDevice_BeginScene_Hook.reset(nullptr);
        Globals::g_Direct3DDevice_EndScene_Hook.reset(nullptr);
        Globals::g_Direct3DDevice_Reset_Hook.reset(nullptr);
    }
}