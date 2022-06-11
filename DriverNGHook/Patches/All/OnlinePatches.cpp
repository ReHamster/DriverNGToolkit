#include "Logger.h"
#include "Patches/All/OnlinePatches.h"

#include <filesystem>
#include <iostream>
#include <fstream>

class SandboxSelector
{
public:
    char m_cOnlineConfigKey[64];
    char m_cSandboxKey[32];
    char m_cSandboxName[32];
    unsigned __int8 m_iSandboxTrackingID;
};


namespace Hermes
{
	enum LogChannel
	{
		LC_error = 0,
		LC_warning = 1,
		LC_info = 2,
		LC_sample = 3,
		LC_system_info = 4,
		LC_result = 5,
		LC_debug1 = 6,
		LC_debug2 = 7,
	};

	typedef void(__cdecl* LogCallback)(Hermes::LogChannel, const char*);
}

namespace DriverNG
{
    namespace Consts
    {
        static constexpr uintptr_t kHermesSetLogCallbackAddr = 0x004C4ACE;
		static constexpr uintptr_t kHermesLogCallbackAddr = 0x016DB558;
    }

    namespace Callbacks
    {
        using std::filesystem::directory_iterator;
        using std::filesystem::path;
        namespace fs = std::filesystem;

		void OnlineLogCallback(Hermes::LogChannel channel, const char* msg)
		{
			switch (channel)
			{
				case Hermes::LC_error:
					MsgError("[Hermes] [error] %s\n", msg);
					break;
				case Hermes::LC_warning:
					MsgWarning("[Hermes] [warn] %s\n", msg);
					break;
				case Hermes::LC_info:
				case Hermes::LC_sample:
				case Hermes::LC_system_info:
				case Hermes::LC_result:
					MsgInfo("[Hermes] %s\n", msg);
					break;
				case Hermes::LC_debug1:
					Msg("[Hermes] [DEBUG] %s\n", msg);
					break;
				case Hermes::LC_debug2:
					Msg("[Hermes] [DEBUG 2] %s\n", msg);
					break;
			}
		}

		Hermes::LogCallback __cdecl HermesSetLogCallback(Hermes::LogCallback fn)
        {
			// set ours log callback instead
			Hermes::LogCallback* hermesLogCallback = (Hermes::LogCallback*)Consts::kHermesLogCallbackAddr;

			*hermesLogCallback = OnlineLogCallback;
			return OnlineLogCallback;
        }
    }


    std::string_view OnlinePatches::GetName() const { return "Online patch"; }
	
    bool OnlinePatches::Apply(const ModPack& modules)
    {
        if (auto process = modules.process.lock())
		{
			// Do not revert this patch!
			if (!HF::Hook::FillMemoryByNOPs(process, Consts::kHermesSetLogCallbackAddr, kHermesSetLogCallbackPatchSize))
			{
				MsgError("Failed to cleanup memory\n");
				return false;
			}

            m_HermesSetLogCallback = HF::Hook::HookFunction<Hermes::LogCallback(__cdecl)(Hermes::LogCallback), kHermesSetLogCallbackPatchSize>(
                process,
                Consts::kHermesSetLogCallbackAddr,
                &Callbacks::HermesSetLogCallback,
                {},
                {});

            if (!m_HermesSetLogCallback->setup())
            {
				MsgError("Failed to initialise hook for Hermes::SetLogCallback\n");
                return false;
            }

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void OnlinePatches::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);

        m_HermesSetLogCallback->remove();
    }
}
