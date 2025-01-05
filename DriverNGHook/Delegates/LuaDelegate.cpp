#include <Windows.h>
#include "Delegates/LuaDelegate.h"
#include "UI/DebugTools.h"
#include "IInputDelegate.h"
#include "CrashHandlerReporter.h"

#include "Logger.h"

#include <lfs.h>
#include <fstream>

#include <imgui.h>
#include <imgui_lua_bindings.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

extern "C"
{
#include "lua.h"
#include "lstate.h"

// extern
LUALIB_API int luaL_loadbuffer(lua_State* L, const char* buff, size_t size, const char* name);
}

#define SOL_IMGUI_ENABLE_INPUT_FUNCTIONS

class LuaAsyncQueue
{
public:
	LuaAsyncQueue()
	{
	}

	int Push(std::function<int()> f);
	int Run();
	void Clear();
	
protected:

	struct work_t
	{
		work_t(std::function<int()> f, uint32_t id)
		{
			func = f;
			workId = id;
		}

		std::function<int()> func;
		uint32_t workId;
	};

	int WaitForResult(uint32_t workId);

	uint32_t m_workCounter;
	std::list<work_t*> m_pendingWork;
	std::mutex m_workMutex;
};

int LuaAsyncQueue::Push(std::function<int()> f)
{
	// set the new worker and signal to start...
	std::lock_guard<std::mutex> lock(m_workMutex);

	work_t* work = new work_t(f, m_workCounter++);
	m_pendingWork.push_back(work);

	return 0;
}

int LuaAsyncQueue::Run()
{
	work_t* currentWork = nullptr;

	// find some work
	while (!currentWork)
	{
		// search for work
		{
			std::lock_guard<std::mutex> lock(m_workMutex);

			for (auto const& work : m_pendingWork)
			{
				currentWork = work;
				break;
			}

			// no work and haven't picked one?
			if (m_pendingWork.size() == 0 && !currentWork)
				break;
		}

		if (currentWork)
		{
			work_t* cur = currentWork;
			{
				std::lock_guard<std::mutex> lock(m_workMutex);
				m_pendingWork.remove(cur);
			}

			// get and quickly dispose
			currentWork = nullptr;

			// run work
			cur->func();
			delete cur;
		}
	}

	return 0;
}

void LuaAsyncQueue::Clear()
{
	// set the new worker and signal to start...
	std::lock_guard<std::mutex> lock(m_workMutex);

	for (auto const& work : m_pendingWork)
	{
		delete work;
	}

	m_pendingWork.clear();
}

LuaAsyncQueue g_luaAsyncQueue;

//----------------------------------------------------------------------

namespace ReHamster
{
	extern ReHamster::CrashHandlerReporter crashHandlerReporter;
}

namespace DriverNG
{	
	namespace Consts
	{
		static const std::string luaScriptsPath = "plugins/DriverNGHook/scripts/";
	}

	namespace Globals
	{
		extern std::unique_ptr<DebugTools>		g_pDebugTools;
		extern std::unique_ptr<IInputDelegate>	g_pInputDelegate;

		extern IDirect3DDevice9*			g_d3dDevice;
		extern HWND							g_focusWindow;
		extern volatile ImGuiContext*		g_sharedImGui;
		extern volatile bool				g_dataDrawn;
	}

	static std::string GetPrintArgsString(lua_State* L)
	{
		const int n = lua_gettop(L);

		lua_getglobal(L, "tostring");

		std::string out;
		for (int i = 1; i <= n; i++)
		{
			lua_pushvalue(L, -1);  // function to be called
			lua_pushvalue(L, i);   // value to print
			lua_call(L, 1, 1);

			if (i > 1) out += ' ';
			size_t l;
			const char* s = lua_tostring(L, -1);  // get result
			out += s ? s : "(null)";
			lua_pop(L, 1);
		}
		return out;
	}

	static int LuaPrintFunc(lua_State* L)
	{
		std::string printArgs = GetPrintArgsString(L);
		MsgInfo("[Lua] ");
		MsgInfo(printArgs.c_str());
		MsgInfo("\n");
		
		if(Globals::g_pDebugTools)
			Globals::g_pDebugTools->LogGameToConsole(printArgs.c_str());
		return 0;
	}

	static bool LuaCheckError(int status, lua_State* L, const char* prefix)
	{
		if (status == 0)
			return true;

		const int errIdx = lua_gettop(L);
		const char* errorStr = lua_tolstring(L, errIdx, nullptr);
		MsgWarning("%s: %s\n", prefix, errorStr);
		lua_pop(L, 1);
		return false;
	}

	// TODO: proper exception handling for Lua funcs!
	//template <typename ...Args>
	//static sol::protected_function_result TryLuaFunction(sol::function aFunc, Args... aArgs)
	//{
	//	sol::protected_function_result result{ };
	//	if (aFunc)
	//	{
	//		try
	//		{
	//			result = aFunc(aArgs...);
	//		}
	//		catch (std::exception& e)
	//		{
	//			MsgError("%s\n", e.what());
	//		}
	//		if (!result.valid())
	//		{
	//			const sol::error cError = result;
	//			MsgError("%s\n",cError.what());
	//		}
	//	}
	//	return result;
	//}

    ILuaDelegate& ILuaDelegate::GetInstance()
    {
        static LuaDelegate* instance = nullptr;
        if (!instance)
        {
            instance = new LuaDelegate();

			ReHamster::crashHandlerReporter.AddCrashFunc([]() {
				instance->PrintLuaStackTrace();
			});
        }

        return *instance;
    }
	
	void LuaDelegate::OnDeleted()
	{
		m_callLuaFunc = nullptr;
		m_gameState = nullptr;
	}

    void LuaDelegate::OnInitialised(lua_State* gameState, CallLuaFunction_t callFunc)
	{
		bool firstTimeInit = m_gameState == nullptr;

		m_callLuaFunc = callFunc;
		m_gameState = gameState;

		luaopen_lfs(m_gameState);

		imGuilState = m_gameState;
		LoadImguiBindings();

		const char* cmd = GetCommandLineA();
		if (strstr(cmd, "-tools") != nullptr)
		{
			m_allowDeveloperConsole = true;
		}

		lua_pushcfunction(m_gameState, LuaPrintFunc);
		lua_setglobal(m_gameState, "print");

		// set folder
		lua_pushstring(m_gameState, Consts::luaScriptsPath.c_str());
		lua_setglobal(m_gameState, "DNGHookScriptPath");

		//if (firstTimeInit)
		//{
		//	// init other lua state
		//	m_luaState.open_libraries(sol::lib::base, sol::lib::string, sol::lib::io, sol::lib::math, sol::lib::package, sol::lib::os, sol::lib::table);
		//	//sol_ImGui::InitBindings(m_luaState);
		//
		//	m_luaState["allowCustomGameScripts"] = [this](bool enable)
		//	{
		//		if (enable)
		//		{
		//			const char* messageStr = "Custom game scripts are allowed\n";
		//			MsgInfo(messageStr);
		//
		//			if (Globals::g_pDebugTools)
		//				Globals::g_pDebugTools->LogGameToConsole(messageStr);
		//		}
		//
		//		m_allowCustomGameScripts = enable;
		//	};
		//
		//	// Driver NG hook internals
		//	{
		//		m_luaState.script_file(Consts::luaScriptsPath + "autoexec.lua");
		//	}
		//}

		// atm true
		m_allowCustomGameScripts = true;

		// little game hooks to allow console stuff
		if (m_allowDeveloperConsole)
		{
			ExecuteFile(Consts::luaScriptsPath + "driverNGConsole.lua");
		}

		// init game Lua hooks
		if (m_allowCustomGameScripts && m_callLuaFunc)
		{
			ExecuteFile(Consts::luaScriptsPath + "game_autoexec.lua");
		}
    }
		
	void LuaDelegate::BeginRender()
	{
		ImGui::SetCurrentContext((ImGuiContext*)Globals::g_sharedImGui);

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void LuaDelegate::EndRender()
	{
		bool showMouseCursor = false;

		if (Globals::g_pDebugTools)
		{
			showMouseCursor = showMouseCursor || Globals::g_pDebugTools->IsVisible();
			Globals::g_pDebugTools->Update();
		}

		ImGuiIO& io = ImGui::GetIO();
		io.MouseDrawCursor = showMouseCursor || Globals::g_pInputDelegate->isGameInputBlocked();

		ImGui::EndFrame();
		ImGui::Render();
	}

	bool LuaDelegate::IsValidLuaState(lua_State* gameState) const
	{
		if (!m_gameState || gameState && m_gameState != gameState)
			return false;

		return true;
	}

    void LuaDelegate::DoCommands(lua_State* gameState)
    {
		if (!IsValidLuaState(gameState))
			return;

		if (!m_allowCustomGameScripts)
		{
			g_luaAsyncQueue.Clear();
			return;
		}

		g_luaAsyncQueue.Run();
    }

	bool LuaDelegate::IsOnlineGame()
	{
		if (!IsValidLuaState(nullptr))
			return false;

		//sol::state_view state(m_gameState);
		//
		//// for now we using Lua
		//sol::table networkLibTable = state["Network"];
		//
		//if (!networkLibTable.valid())
		//	return false;
		//
		//sol::function getConnectionFn = networkLibTable["getConnection"];
		//
		//if(!getConnectionFn.valid())
		//	return false;
		//
		//auto result = TryLuaFunction(getConnectionFn);
		//if (result.valid())
		//{
		//	std::string value = result;
		//	return value == "Online";
		//}

		return false;
	}

	bool LuaDelegate::IsDeveloperConsoleAllowed()
	{
		return m_allowDeveloperConsole;
	}

	void LuaDelegate::ExecuteString(const std::string& code)
	{
		if (!IsValidLuaState(nullptr))
			return;

		if (IsOnlineGame() && !m_allowCustomGameScripts)
			return;

		const int bufStatus = luaL_loadbuffer(m_gameState, code.c_str(), code.size(), "ExecuteString");
		if (!LuaCheckError(bufStatus, m_gameState, "Lua parse error"))
			return;

		const int result = lua_pcall(m_gameState, 0, LUA_MULTRET, 0);
		if (!LuaCheckError(result, m_gameState, "Script error"))
			return;
	}

	void LuaDelegate::ExecuteFile(const std::string& filename)
	{
		if (!IsValidLuaState(nullptr))
			return;

		if (IsOnlineGame() && !m_allowCustomGameScripts)
			return;

		// load file
		std::ifstream fileStream(filename);
		std::string luaSourceStr((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

		const int bufStatus = luaL_loadbuffer(m_gameState, luaSourceStr.c_str(), luaSourceStr.size(), filename.c_str());
		if (!LuaCheckError(bufStatus, m_gameState, "Lua parse error"))
			return;

		const int result = lua_pcall(m_gameState, 0, LUA_MULTRET, 0);
		if (!LuaCheckError(result, m_gameState, "Script error"))
			return;
	}

	int LuaDelegate::Push(std::function<int()> f)
    {
		return g_luaAsyncQueue.Push(f);
    }

    CallLuaFunction_t LuaDelegate::GetCallLuaFunction()
    {
        return m_callLuaFunc;
    }

	void LuaDelegate::PrintLuaStackTrace()
	{
		if (!m_gameState)
			return;

		lua_State* L = m_gameState;
		lua_Debug ar;
		int depth = 0;

		Msg("\nLua stack trace:\n");

		while (lua_getstack(L, depth, &ar))
		{
			int status = lua_getinfo(L, "Sln", &ar);
			assert(status);

			Msg("\t %s:", ar.short_src);
			if (ar.currentline > 0)
				Msg("%d:", ar.currentline);
			if (*ar.namewhat != '\0')  /* is there a name? */
				Msg(" in function '%s'", ar.name);
			else
			{
				if (*ar.what == 'm')  /* main? */
					Msg(" in main chunk");
				else if (*ar.what == 'C' || *ar.what == 't')
					Msg(" ?");  /* C function or tail call */
				else
					Msg(" in function <%s:%d>",
						ar.short_src, ar.linedefined);
			}
			Msg("\n");
			depth++;
		}
		Msg("\n");
	}
};

