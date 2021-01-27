#include <Windows.h>
#include <Delegates/LuaDelegate.h>
#include <spdlog/spdlog.h>

#define WORK_PENDING_MARKER 0x1d1d0001


class LuaAsyncQueue
{
	friend class ShaderAPIGL;

public:
	LuaAsyncQueue()
	{
	}

	int Push(std::function<int()> f);
	int Run();
	
protected:

	struct work_t
	{
		work_t(std::function<int()> f, uint32_t id)
		{
			func = f;
			result = WORK_PENDING_MARKER;
			workId = id;
		}

		std::function<int()> func;
		volatile int result;
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
				if (work->result == WORK_PENDING_MARKER)
				{
					currentWork = work;
					break;
				}
			}

			// no work and haven't picked one?
			if (m_pendingWork.size() == 0 && !currentWork)
				break;
		}

		if (currentWork)
		{
			std::lock_guard<std::mutex> lock(m_workMutex);
			
			// get and quickly dispose
			work_t* cur = currentWork;
			currentWork = nullptr;

			// run work
			int result = cur->func();

			cur->result = result;

			m_pendingWork.remove(cur);
		}
	}

	return 0;
}

LuaAsyncQueue g_luaAsyncQueue;

namespace DriverNG
{	
    ILuaDelegate& ILuaDelegate::GetInstance()
    {
        static LuaDelegate* instance = nullptr;
        if (!instance)
        {
            instance = new LuaDelegate();
        }

        return *instance;
    }
	
    void LuaDelegate::OnInitialised(CallLuaFunction_t callFunc)
	{
        m_callLuaFunc = callFunc;

    	// init Lua hooks
    	if(m_callLuaFunc)
            m_callLuaFunc("driverNGHook_Init", ">");


		/*
		char* test = nullptr;
		callLuaFunction("driverNGHook_test", ">s", &test);

		spdlog::info("Lua test: {}", test);
		*/
		//callLuaFunction("driverNGHook_EvalHelper", "s", "printffffs(\"----------THIS IS A TEST EVALUATED MESSAGE FROM HOOK----------\" )");
    }
	
    void LuaDelegate::DoCommands()
    {
        static bool workFlag = false;

        if (workFlag) // to prevent stack overflowing
            return;
    	
        if (!m_callLuaFunc)
            return;
    	
        int numLogItems = 0;
        char* pString = nullptr;

        workFlag = true;

		g_luaAsyncQueue.Run();
    	
    	do
    	{
            m_callLuaFunc("driverNGHook_LogPopNext", ">is", &numLogItems, &pString);
    		
    		if(numLogItems && pString)
                printf("%s\n", pString);

        } while (numLogItems);

        workFlag = false;
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
