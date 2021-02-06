
#include <Patches/All/LuaPatches.h>
#include <Delegates/ILuaDelegate.h>
#include <Windows.h>

#include <spdlog/spdlog.h>
#include <sol/sol.hpp>

namespace DriverNG
{
    namespace Consts
    {
        static constexpr uintptr_t kCallLuaFunctionAddress  = 0x005E47A0;

        static constexpr uintptr_t kOpenScriptLoaderCallAddr = 0x005E7E04;
        static constexpr uintptr_t kOpenScriptLoaderOrigAddr = 0x0065A810;

        //static constexpr uintptr_t kStartLuaCallAddress     = 0x005E7E35;     // to the call CState_Frontend_Loading::Step
        //static constexpr uintptr_t kStartLuaOrigAddress     = 0x005E4C70;

        static constexpr uintptr_t kStepLuaCallAddress     = 0x0079B811;     // to the call CState_Main::Step
        static constexpr uintptr_t kStepLuaOrigAddress     = 0x005E4A70;
    }

    namespace Globals
    {
        static ILuaDelegate& g_luaDelegate = ILuaDelegate::GetInstance();
        static lua_State* g_gameLuaState = nullptr;

    	static void InitializeLuaStateBindings(lua_State* newState)
    	{
            g_gameLuaState = newState;
    		
    		// report to the game state
            auto callLuaFunction = (CallLuaFunction_t)Consts::kCallLuaFunctionAddress;

            g_luaDelegate.OnInitialised(callLuaFunction);
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

            // we do our own Lua initialization
            auto callLuaFunction = (CallLuaFunction_t)Consts::kCallLuaFunctionAddress;

            // then make to our bindings
            // make game exec our file
            callLuaFunction("dofile", "s", "plugins/DriverNGHook/scripts/game_autoexec.lua");

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

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void LuaPatches::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);

        m_openScriptLoaderHook->remove();
        m_stepLuaHook->remove();
    }
}