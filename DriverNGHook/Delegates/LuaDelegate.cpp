#include <Windows.h>
#include "Delegates/LuaDelegate.h"
#include "UI/DebugTools.h"
#include "IInputDelegate.h"

#include "Logger.h"

#include <lfs.h>
#include "lstate.h"

#define SOL_IMGUI_ENABLE_INPUT_FUNCTIONS

#include <imgui.h>
#include <sol_imgui/sol_imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>


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

	// TODO: proper exception handling for Lua funcs!
	template <typename ...Args>
	static sol::protected_function_result TryLuaFunction(sol::function aFunc, Args... aArgs)
	{
		sol::protected_function_result result{ };
		if (aFunc)
		{
			try
			{
				result = aFunc(aArgs...);
			}
			catch (std::exception& e)
			{
				MsgError("%s\n", e.what());
			}
			if (!result.valid())
			{
				const sol::error cError = result;
				MsgError("%s\n",cError.what());
			}
		}
		return result;
	}

    ILuaDelegate& ILuaDelegate::GetInstance()
    {
        static LuaDelegate* instance = nullptr;
        if (!instance)
        {
            instance = new LuaDelegate();
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
		sol::state_view sv(m_gameState);

		sol_ImGui::InitBindings(sv);

		const char* cmd = GetCommandLineA();
		if (strstr(cmd, "-tools") != nullptr)
		{
			m_allowDeveloperConsole = true;
		}

		// install our print function to the game
		sv["print"] = [](sol::variadic_args args, sol::this_state aState)
		{
			std::ostringstream oss;
			sol::state_view s(aState);
			auto toString = s["tostring"];

			MsgInfo("[Lua] ");
			
			for (auto it = args.cbegin(); it != args.cend(); ++it)
			{
				if (it != args.cbegin())
				{
					oss << " ";
					MsgInfo(" ");
				}

				std::string str = ((*it).get_type() == sol::type::string) ? (*it).get<std::string>() : toString((*it).get<sol::object>());

				MsgInfo(str.c_str());
				oss << str;
			}
			MsgInfo("\n");

			if(Globals::g_pDebugTools)
				Globals::g_pDebugTools->LogGameToConsole(oss.str());
		};

		// set folder
		sv["DNGHookScriptPath"] = Consts::luaScriptsPath;

		if (firstTimeInit)
		{
			// init other lua state
			m_luaState.open_libraries(sol::lib::base, sol::lib::string, sol::lib::io, sol::lib::math, sol::lib::package, sol::lib::os, sol::lib::table);
			//sol_ImGui::InitBindings(m_luaState);

			m_luaState["allowCustomGameScripts"] = [this](bool enable)
			{
				if (enable)
				{
					const char* messageStr = "Custom game scripts are allowed\n";
					MsgInfo(messageStr);

					if (Globals::g_pDebugTools)
						Globals::g_pDebugTools->LogGameToConsole(messageStr);
				}

				m_allowCustomGameScripts = enable;
			};

			// Driver NG hook internals
			{
				m_luaState.script_file(Consts::luaScriptsPath + "autoexec.lua");
			}
		}

		// little game hooks to allow console stuff
		if (m_allowDeveloperConsole)
		{
			try
			{
				sv.do_file(Consts::luaScriptsPath + "driverNGConsole.lua");
			}
			catch (sol::error& err)
			{
				MsgWarning("driverNGConsole.lua failed to load\n%s\n", err.what());
			}
		}

		// init game Lua hooks
		if (m_allowCustomGameScripts && m_callLuaFunc)
		{
			try
			{
				sv.do_file(Consts::luaScriptsPath + "game_autoexec.lua");
			}
			catch (sol::error& err)
			{
				MsgWarning("game_autoexec.lua failed to load\n%s\n", err.what());
			}
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

	bool LuaDelegate::IsValidLuaState() const
	{
		if (!m_gameState)
			return false;

		if (!m_gameState->l_G)
			return false;

		return true;
	}

    void LuaDelegate::DoCommands()
    {
		if (!IsValidLuaState())
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
		if (!IsValidLuaState())
			return false;

		sol::state_view state(m_gameState);

		// for now we using Lua
		sol::table networkLibTable = state["Network"];

		if (!networkLibTable.valid())
			return false;

		sol::function getConnectionFn = networkLibTable["getConnection"];

		if(!getConnectionFn.valid())
			return false;

		auto result = TryLuaFunction(getConnectionFn);
		if (result.valid())
		{
			std::string value = result;
			return value == "Online";
		}

		return false;
	}

	bool LuaDelegate::IsDeveloperConsoleAllowed()
	{
		return m_allowDeveloperConsole;
	}

	sol::protected_function_result LuaDelegate::ExecuteString(const std::string& code)
	{
		if (!IsValidLuaState())
			return sol::protected_function_result();

		if (IsOnlineGame() && !m_allowCustomGameScripts)
			return sol::protected_function_result();

		sol::state_view state(m_gameState);
		return state.do_string(code);
	}

	sol::protected_function_result LuaDelegate::ExecuteFile(const std::string& filename)
	{
		if (!IsValidLuaState())
			return sol::protected_function_result();

		if (IsOnlineGame() && !m_allowCustomGameScripts)
			return sol::protected_function_result();

		sol::state_view state(m_gameState);
		return state.do_file(filename);
	}

	int LuaDelegate::Push(std::function<int()> f)
    {
		return g_luaAsyncQueue.Push(f);
    }

    CallLuaFunction_t LuaDelegate::GetCallLuaFunction()
    {
        return m_callLuaFunc;
    }
};
