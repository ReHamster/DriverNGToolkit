
#include <Patches/All/LuaPatches.h>
#include <Delegates/ILuaDelegate.h>
#include <Windows.h>

#include <spdlog/spdlog.h>
#include <sol/sol.hpp>

namespace DriverNG
{
    namespace Consts
    {
        static constexpr intptr_t kriLuaCallAddress         = 0x005E48E5;
        static constexpr intptr_t kriLuaCallOrigAddress     = 0x005F1CA0;
        static constexpr intptr_t kCallLuaFunctionAddress   = 0x005E47A0;
        static constexpr intptr_t kStartLuaCallAddress      = 0x005E7E35;     // to the call CState_Frontend_Loading::Step
        static constexpr intptr_t kStartLuaOrigAddress      = 0x005E4C70;
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
        int riLuaCall_Hooked(lua_State* L, int nargs, int nresults, bool protect)
        {
            typedef int(* riLuaCall_t)(lua_State*, int, int, bool);

            auto riLuaCallOrig = (riLuaCall_t)Consts::kriLuaCallOrigAddress;
            int result = riLuaCallOrig(L, nargs, nresults, protect);

            // call console commands
        	//Globals::g_luaDelegate.DoCommands();
        	
            return result;
        }
    	
        void StartLua_Hooked(lua_State* state)
        {
            typedef void (* StartLua_t)(lua_State*);
            auto origStartLua = (StartLua_t)Consts::kStartLuaOrigAddress;

        	// make game bind it's bindings
            origStartLua(state);

        	// then hook to our bindings
        	if(Globals::g_gameLuaState != state)
        	{
                // get the lua state from the address
                Globals::InitializeLuaStateBindings(state);
        	}
        }
    }

    std::string_view LuaPatches::GetName() const { return "Lua Patch"; }
	
    bool LuaPatches::Apply(const ModPack& modules)
    {
        if (auto process = modules.process.lock())
		{
	        // Do not revert this patch!
	        if (!HF::Hook::FillMemoryByNOPs(process, Consts::kStartLuaCallAddress, kStartLuaPatchSize))
	        {
	        	spdlog::error("Failed to cleanup memory");
	        	return false;
	        }

	        m_startLuaHook = HF::Hook::HookFunction<void(*)(lua_State*), kStartLuaPatchSize>(
                process,
                Consts::kStartLuaCallAddress,
                &Callbacks::StartLua_Hooked,
                {},
                {});

	        if (!m_startLuaHook->setup())
	        {
	        	spdlog::error("Failed to setup patch to StartLua!");
	        	return false;
	        }

            // Do not revert this patch!
            if (!HF::Hook::FillMemoryByNOPs(process, Consts::kriLuaCallAddress, kriLuaCallPatchSize))
            {
                spdlog::error("Failed to cleanup memory");
                return false;
            }

            m_riLuaCallHook = HF::Hook::HookFunction<int(*)(lua_State*, int, int, bool), kriLuaCallPatchSize>(
                process,
                Consts::kriLuaCallAddress,
                &Callbacks::riLuaCall_Hooked,
                {},
                {});

            if (!m_riLuaCallHook->setup())
            {
                spdlog::error("Failed to setup patch to riLuaCall!");
                return false;
            }

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void LuaPatches::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);

        m_startLuaHook->remove();
    }
}