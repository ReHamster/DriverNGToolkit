#include <stdio.h>

#include <cmdlib.h>
#include <Patches/All/OnlinePatches.h>
#include <details/helpers.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cereal.hpp>
#include <archives/json.hpp>

class SandboxSelector
{
public:
    // Default constructor in case if needed
    SandboxSelector()
    {
        strcpy_s(m_cOnlineConfigKey, 64, "885642bfde8842b79bbcf2c1f8102403");
        strcpy_s(m_cSandboxKey, 32, "w6kAtr3T");
        strcpy_s(m_cSandboxName, 32, "PC Sandbox PA");
        m_iSandboxTrackingID = 4;
    }

    char m_cOnlineConfigKey[64];
    char m_cSandboxKey[32];
    char m_cSandboxName[32];
    unsigned __int8 m_iSandboxTrackingID;
};

namespace Quazal
{
    struct String
    {
        char* m_szContent;
    };

    class RootObject
    {
    };

    class OnlineConfigClient : RootObject
    {
    public:
        virtual ~OnlineConfigClient()
        {
        }

        bool m_Initialized;
        void* m_cachedValues[9]; // must be a Quazal::qVector<Quazal::String>
        void/*Quazal::OnlineConfigClient::JobPopulateConfig*/* m_Job;
        Quazal::String m_OnlineConfigId;
        Quazal::String m_OnlineConfigServiceHost;
    };
}

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
    using namespace std;
    using namespace cereal;

    using std::filesystem::path;

    struct OnlineConfig
    {
        string ServiceUrl;
        string ConfigKey;
        string AccessKey;
        bool Use;

        // ReSharper disable CppInconsistentNaming
        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(CEREAL_NVP(ServiceUrl), CEREAL_NVP(ConfigKey), CEREAL_NVP(AccessKey), CEREAL_NVP(Use));
        }

        // ReSharper restore CppInconsistentNaming
    };

    namespace Consts
    {
        static constexpr uintptr_t kOnlineConfigServiceHostPRODAddress = 0x016DD358;
        static constexpr uintptr_t kSandboxSelectorConstructorAddr = 0x004CF530;
		static constexpr uintptr_t kHermesLogCallbackAddr = 0x016DB558;
        static constexpr auto CONFIG_NAME = "Orbit.json";
    }

    namespace Globals
    {
        using std::filesystem::path;
        template <class T>
        void DeserializeFromJsonFile(const path& file, T& data)
        {
            if (auto fs = std::ifstream(file, ios::in); fs)
            {
                JSONInputArchive ar(fs);
                ar(data);
                return;
            }

			MsgError("File read error: %s\n", file.string().c_str());
        }
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

        void __stdcall OnSandboxSelectorConstructor(SandboxSelector* self)
        {
            const auto currentPath = fs::current_path();
            const auto configPath = currentPath / path(Consts::CONFIG_NAME);

            // ReSharper disable CppRedundantQualifier
            bool error = false;

            if (fs::exists(configPath))
            {
                if (fs::is_empty(configPath))
                {
					MsgError("%s file not found!\n", Consts::CONFIG_NAME);
                    error = true;
                }
               
            }
            else
            {
				MsgError("%s file is empty!\n", Consts::CONFIG_NAME);
                error = true;
            }

            OnlineConfig config;
            if (!error)
            {
				try
				{
					NameValuePair<OnlineConfig&> onlineConfig("OnlineConfig", config);
					Globals::DeserializeFromJsonFile(configPath, onlineConfig);
				}
				catch (std::exception& ex)
				{
					MsgError("Error loading JSON: %s\n", ex.what());
					error = true;
				}
            }

            if (!error && config.Use)
            {
                char* str = *(char**)Consts::kOnlineConfigServiceHostPRODAddress;
                strcpy_s(str, 28, config.ServiceUrl.c_str());

                strcpy_s(self->m_cOnlineConfigKey, 64, config.ConfigKey.c_str());
                strcpy_s(self->m_cSandboxKey, 32, config.AccessKey.c_str());
                strcpy_s(self->m_cSandboxName, 32, "DriverMadness Sandbox A");

                self->m_iSandboxTrackingID = 4;
            }
            else
            {
                strcpy_s(self->m_cOnlineConfigKey, 64, "885642bfde8842b79bbcf2c1f8102403");
                strcpy_s(self->m_cSandboxKey, 32, "w6kAtr3T");
                strcpy_s(self->m_cSandboxName, 32, "PC Sandbox PA");
                self->m_iSandboxTrackingID = 4;
            }

            // ReSharper restore CppRedundantQualifier

			// also set ours OnlineLog callback
			Hermes::LogCallback* hermesLogCallback = (Hermes::LogCallback*)Consts::kHermesLogCallbackAddr;

			*hermesLogCallback = OnlineLogCallback;
        }
    }


    std::string_view OnlinePatches::GetName() const { return "Online patch"; }
	
    bool OnlinePatches::Apply(const ModPack& modules)
    {
        if (auto process = modules.process.lock())
		{

            //-------------------------------------------
            // disable strcpy calls
            HF::Hook::FillMemoryByNOPs(process, Consts::kSandboxSelectorConstructorAddr + 0xd, 5);
            HF::Hook::FillMemoryByNOPs(process, Consts::kSandboxSelectorConstructorAddr + 0x1f, 5);
            HF::Hook::FillMemoryByNOPs(process, Consts::kSandboxSelectorConstructorAddr + 0x31, 5);
            m_SandboxSelectorConstructor = HF::Hook::HookFunction<void(__stdcall)(SandboxSelector*), kSandboxSelectorConstructorPatchSize>(
                process,
                Consts::kSandboxSelectorConstructorAddr,
                &Callbacks::OnSandboxSelectorConstructor,
                {
                    HF::X86::PUSH_AD,
                    HF::X86::PUSH_FD,
                    HF::X86::PUSH_EAX
                },
                {
                    HF::X86::POP_FD,
                    HF::X86::POP_AD
                });

            if (!m_SandboxSelectorConstructor->setup())
            {
				MsgError("Failed to initialise hook for SandboxSelector constructor\n");
                return false;
            }

            return BasicPatch::Apply(modules);
        }

        return false;
    }

    void OnlinePatches::Revert(const ModPack& modules)
    {
        BasicPatch::Revert(modules);

        m_SandboxSelectorConstructor->remove();
    }
}
