
#include <Patches/All/LuaPatches.h>
#include <Delegates/ILuaDelegate.h>
#include <Windows.h>
#include <stdio.h>

#include <spdlog/spdlog.h>
#include <sol/sol.hpp>
#include <UI/DebugTools.h>

namespace DriverNG
{
    namespace Consts
    {
        static constexpr uintptr_t kCallLuaFunctionAddress  = 0x005E47A0;

        static constexpr uintptr_t kOpenScriptLoaderCallAddr = 0x005E7E04;
        static constexpr uintptr_t kOpenScriptLoaderOrigAddr = 0x0065A810;

        static constexpr uintptr_t kDeleteLuaStateCallAddress     = 0x0078E975;
        static constexpr uintptr_t kDeleteLuaStateOrigAddress     = 0x005E7E40;

        static constexpr uintptr_t kStepLuaCallAddress     = 0x0079B811;     // to the call CState_Main::Step
        static constexpr uintptr_t kStepLuaOrigAddress     = 0x005E4A70;

        static constexpr uintptr_t ksafe_vsprintfOrigAdress = 0x00903EBA;
    }

    namespace Globals
    {
        extern std::unique_ptr<DebugTools> g_pDebugTools;
        static ILuaDelegate& g_luaDelegate = ILuaDelegate::GetInstance();

    	static void InitializeLuaStateBindings(lua_State* newState)
    	{
    		// report to the game state
            auto callLuaFunction = (CallLuaFunction_t)Consts::kCallLuaFunctionAddress;

            g_luaDelegate.OnInitialised(newState, callLuaFunction);
    	}
    }

    namespace Callbacks
    {
        void OpenScriptLoader_Hooked(lua_State* state)
        {
            typedef void (*OpenScriptLoader)(lua_State*);
            auto origOpenScriptLoader = (OpenScriptLoader)Consts::kOpenScriptLoaderOrigAddr;

            // make game open chunk file
            origOpenScriptLoader(state);

            // get the lua state from the address
            Globals::InitializeLuaStateBindings(state);
        }

        void StepLua_Hooked(lua_State* state, bool paused)
        {
            typedef void (*StepLua_t)(lua_State*, bool);
            auto origStepLua = (StepLua_t)Consts::kStepLuaOrigAddress;

            origStepLua(state, paused);

            // do commands after
            Globals::g_luaDelegate.DoCommands();
        }

        void DeleteLuaState_Hooked(lua_State* state)
        {
            typedef void (*DeleteLuaState_t)(lua_State*);
            auto origDeleteLuaState = (DeleteLuaState_t)Consts::kDeleteLuaStateOrigAddress;

            Globals::g_luaDelegate.OnDeleted();

            origDeleteLuaState(state);
        }

        int __cdecl _vsnprintf_hooked(char* const _Buffer,size_t  const _BufferCount, char const* const _Format,va_list _ArgList )
        {
            int ret = _vsnprintf(_Buffer, _BufferCount, _Format, _ArgList);
        	
            // hook part
            if (*_Buffer == '$')
            {
                const char* endStr = strchr(_Buffer, '$');
                if (endStr != nullptr)
                {
                    auto callLuaFunction = (CallLuaFunction_t)Consts::kCallLuaFunctionAddress;

                    callLuaFunction("DSF_RPC_CALL", "s", _Buffer);

                    // TODO: check VEdit?
                    //spdlog::info(_Buffer);
                	//if(Globals::g_pDebugTools)
					//	Globals::g_pDebugTools->LogGameToConsole(_Buffer);
                }
            }
        	
            return ret;
        }

    }

    std::string_view LuaPatches::GetName() const { return "Lua Patch"; }
	
    bool LuaPatches::Apply(const ModPack& modules)
    {
        if (auto process = modules.process.lock())
		{
	        // Do not revert this patch!
	        if (!HF::Hook::FillMemoryByNOPs(process, Consts::kOpenScriptLoaderCallAddr, kOpenScriptLoaderPatchSize))
	        {
	        	spdlog::error("Failed to cleanup memory");
	        	return false;
	        }

	        m_openScriptLoaderHook = HF::Hook::HookFunction<void(*)(lua_State*), kOpenScriptLoaderPatchSize>(
                process,
                Consts::kOpenScriptLoaderCallAddr,
                &Callbacks::OpenScriptLoader_Hooked,
                {},
                {});

	        if (!m_openScriptLoaderHook->setup())
	        {
	        	spdlog::error("Failed to setup patch to OpenScriptLoader!");
	        	return false;
	        }

            //------------------------------------------

            // Do not revert this patch!
            if (!HF::Hook::FillMemoryByNOPs(process, Consts::kStepLuaCallAddress, kStepLuaPatchSize))
            {
                spdlog::error("Failed to cleanup memory");
                return false;
            }

            m_stepLuaHook = HF::Hook::HookFunction<void(*)(lua_State*, bool), kStepLuaPatchSize>(
                process,
                Consts::kStepLuaCallAddress,
                &Callbacks::StepLua_Hooked,
                {},
                {});

            if (!m_stepLuaHook->setup())
            {
                spdlog::error("Failed to setup patch to StepLua!");
                return false;
            }

            //------------------------------------------

            // Do not revert this patch!
            if (!HF::Hook::FillMemoryByNOPs(process, Consts::kDeleteLuaStateCallAddress, kDeleteLuaStatePatchSize))
            {
                spdlog::error("Failed to cleanup memory");
                return false;
            }

            m_deleteLuaStateHook = HF::Hook::HookFunction<void(*)(lua_State*), kDeleteLuaStatePatchSize>(
                process,
                Consts::kDeleteLuaStateCallAddress,
                &Callbacks::DeleteLuaState_Hooked,
                {},
                {});

            if (!m_deleteLuaStateHook->setup())
            {
                spdlog::error("Failed to setup patch to DeleteLuaState!");
                return false;
            }

        	//------------------------------------------

			// Do not revert this patch!
            if (!HF::Hook::FillMemoryByNOPs(process, Consts::ksafe_vsprintfOrigAdress, ksafe_vsprintfPatchSize))
            {
                spdlog::error("Failed to cleanup memory");
                return false;
            }
            
            m_safe_vsprintfHook = HF::Hook::HookFunction<int(__cdecl*)(char* const, size_t  const, char const* const, va_list), ksafe_vsprintfPatchSize>(
                process,
                Consts::ksafe_vsprintfOrigAdress,
                &Callbacks::_vsnprintf_hooked,
                {},
                {});

            if (!m_safe_vsprintfHook->setup())
            {
                spdlog::error("Failed to setup patch to safe_sprintf!");
                return false;
            }

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void LuaPatches::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);

        m_openScriptLoaderHook->remove();
        m_deleteLuaStateHook->remove();
        m_stepLuaHook->remove();
        m_safe_vsprintfHook->remove();
    }
}
