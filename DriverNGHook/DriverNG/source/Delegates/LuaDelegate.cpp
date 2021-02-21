#include <Windows.h>
#include <Delegates/LuaDelegate.h>
#include <spdlog/spdlog.h>
#include <UI/DebugTools.h>
#include <imgui.h>
#include <sol_imgui/sol_imgui.h>

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
	namespace Globals
	{
		extern std::unique_ptr<DebugTools> g_pDebugTools;
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
				spdlog::error(e.what());
			}
			if (!result.valid())
			{
				const sol::error cError = result;
				spdlog::error(cError.what());
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
		m_callLuaFunc = callFunc;
		m_gameState = gameState;

		sol::state_view sv(m_gameState);

		// install our print function to the game
		sv["print"] = [](sol::variadic_args aArgs, sol::this_state aState)
		{
			std::ostringstream oss;
			sol::state_view s(aState);

			for (auto it = aArgs.cbegin(); it != aArgs.cend(); ++it)
			{
				if (it != aArgs.cbegin())
				{
					oss << " ";
				}
				std::string str = s["tostring"]((*it).get<sol::object>());
				oss << str;
			}

			//spdlog::info(oss.str());

			Globals::g_pDebugTools->LogGameToConsole(oss.str());
		};

		// init other lua state
		m_luaState.open_libraries(sol::lib::base, sol::lib::string, sol::lib::io, sol::lib::math, sol::lib::package, sol::lib::os, sol::lib::table);
		sol_ImGui::InitBindings(m_luaState);

		m_luaState["registerForEvent"] = [this](const std::string& acName, sol::function aCallback)
		{
			if (acName == "onInit")
				m_onInit = aCallback;
			else if (acName == "onUpdate")
				m_onUpdate = aCallback;
			else
				spdlog::error("Tried to register an unknown event '{}'!", acName);
		};

		m_luaState["setAllowOnlineCheats"] = [this](bool enable)
		{
			if(enable)
				Globals::g_pDebugTools->LogGameToConsole("WARNING: Online cheats ARE ALLOWED");

			m_allowOnlineCheats = enable;
		};

		{
			m_luaState.script_file("plugins/DriverNGHook/scripts/autoexec.lua");
			TryLuaFunction(m_onInit);

			InitializeGameDevelopmentLib();
		}

		// init game Lua hooks
		if (m_callLuaFunc)
		{
			sv.do_file("plugins/DriverNGHook/scripts/game_autoexec.lua");
		}
    }

	void LuaDelegate::InitializeGameDevelopmentLib()
	{
		sol::state_view sv(m_gameState);

		sol::table developmentLibTable = sv.create_table();

		developmentLibTable["addGraphics"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["addGraphicsTransform"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["addGraphicsHeading"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["add2DGraphicsTransform"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["add2DGraphicsHeading"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["startDevGraphicsBatch"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["endDevGraphicsBatch"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["removeDevGraphicsBatch"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["dropBatchModelList"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["removeGraphics"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["useDevGraph"] = [this](const sol::table _this, bool enable)
		{
		};
		developmentLibTable["getUseDevGraph"] = [this](const sol::table _this)
		{
			return false;
		};
		developmentLibTable["setDrawDevGraphicsBatch"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["setDrawGraphics"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["add2DText"] = [this](const sol::table _this, int nID, const std::string& acLabel, const sol::userdata& udPosition, const sol::userdata& udColour, float scale, int unkn )
		{
			sol::table tbl = m_luaState["Development"];
			sol::function add2DTextFunc = tbl["add2DText"];

			TryLuaFunction(add2DTextFunc, tbl, nID, acLabel);

			//spdlog::warn("id: {} label: {}\n", nID, acLabel);
		};
		developmentLibTable["add3DText"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["get2DTextSize"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["eraseText"] = [this](const sol::table _this, int nID)
		{
			spdlog::warn("eraseText: {}\n", nID);
		};
		developmentLibTable["startDevTextBatch"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["endDevTextBatch"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["removeDevTextBatch"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["useDevText"] = [this](const sol::table _this, bool enable)
		{
		};
		developmentLibTable["getUseDevText"] = [this](const sol::table _this)
		{
			return false;
		};
		developmentLibTable["setDrawDevTextBatch"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["setDrawTextElement"] = [this](const sol::table _this, int nID)
		{
		};
		developmentLibTable["clearScreen"] = [this](const sol::table _this, const std::string& acID)
		{
			sol::table tbl = m_luaState["Development"];
			sol::function clearScreenFunc = tbl["clearScreen"];

			TryLuaFunction(clearScreenFunc, tbl, acID);
		};

		sv["HookDevelopment"] = developmentLibTable;

		// Development:addGraphics(pickerId, "sphere", pickerColor, pickerPosition, cameraHeading, pickerRadiusVec, -1)
		// Development:removeGraphics(pickerId)

		// Development:add2DText(textID, objectpicker.GetObject(objectIndex), vec.vector(objectListX, yPos, 0, 0), pickedObjectColor, 1, -1)
		// Development:clearScreen("all")
		// Development:add2DText(i, lanBrowserMenu.entries[i].name, menuEntry_position, i == selected and selectionColour or textColour, item_scale, -1)
		// Development:useDevText(true)
	}
	
	void LuaDelegate::DoRenderUpdate()
	{
		TryLuaFunction(m_onUpdate);
	}

    void LuaDelegate::DoCommands()
    {
		if (!m_gameState)
			return;

		if (IsOnlineGame() && !m_allowOnlineCheats)
		{
			g_luaAsyncQueue.Clear();
			return;
		}

		g_luaAsyncQueue.Run();
    }

	bool LuaDelegate::IsOnlineGame()
	{
		if (!m_gameState)
			return false;

		sol::state_view state(m_gameState);

		// for now we using Lua
		sol::table networkLibTable = state["Network"];
		sol::function getConnectionFn = networkLibTable["getConnection"];

		auto result = TryLuaFunction(getConnectionFn);
		if (result.valid())
		{
			std::string value = result;
			return value == "Online";
		}

		return false;
	}

	sol::protected_function_result LuaDelegate::ExecuteString(const std::string& code)
	{
		if (IsOnlineGame() && !m_allowOnlineCheats)
			return sol::protected_function_result();

		sol::state_view state(m_gameState);
		return state.do_string(code);
	}

	sol::protected_function_result LuaDelegate::ExecuteFile(const std::string& filename)
	{
		if (IsOnlineGame() && !m_allowOnlineCheats)
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
