#include "ReHamster.h"
#include "Logger.h"
#include "DebugConsole.h"
#include "CrashHandlerReporter.h"
#include "Client.h"

#include <Windows.h>

namespace ReHamster
{
	Client::IClient* g_ClientInterface = nullptr;
	CrashHandlerReporter crashHandlerReporter;

	Client::IClient* CreateClientInterface()
	{
		return new DriverNG::Client();
	}

	void Core::Init()
	{
		const char* cmd = GetCommandLineA();

		if (strstr(cmd, "-console") != nullptr)
		{
			ReHamster::DebugConsole::Create("DriverNG Hook | Developer Console");
		}
		
		ReHamster::Logger::Setup();

		// Setup client core
		g_ClientInterface = CreateClientInterface();
		RH_ASSERT2(g_ClientInterface != nullptr, "Bad g_ClientInterface interface! Probably CreateClientInterface() failed!");
		if (!g_ClientInterface)
			return;// EXIT_FAILURE;

		const auto gameVersion = TryToDetectGameVersion();
		bool isOkVersion = false;

		switch (gameVersion)
		{
			case GameVersion::DriverSanFrancisco_PC_1_0_4:
				isOkVersion = true;
				break;
			case GameVersion::UnknownBuild:
				break;
		}

		if (!isOkVersion)
		{
			MessageBox(nullptr, "Sorry, but this game not supported yet", "DriverNGHook| Game not supported", MB_ICONEXCLAMATION | MB_OK);

			delete g_ClientInterface;
			g_ClientInterface = nullptr;

			ReHamster::Logger::Shutdown();
			return;
		}

		crashHandlerReporter.Install();

		if (!g_ClientInterface->OnAttach())
		{
			RH_ASSERT2(g_ClientInterface->OnAttach(), "g_ClientInterface->OnAttach() failed! See console for details!");

			delete g_ClientInterface;
			g_ClientInterface = nullptr;

			ReHamster::Logger::Shutdown();
		}
	}

	void Core::Shutdown()
	{
		if (g_ClientInterface)
		{
			g_ClientInterface->OnDestroy();
			MsgInfo("Core::EntryPoint() shutdown...\n");
			ReHamster::Logger::Shutdown();
		}
	}

	Core::GameVersion Core::TryToDetectGameVersion()
	{
		struct VersionDef
		{
			std::string Id;
			std::intptr_t StrAddr;
		};

		static constexpr size_t kMaxVersionLen = 32;
		static constexpr std::intptr_t kUnknownAddr = 0xDEADB33F;

		static std::pair<Core::GameVersion, VersionDef> PossibleGameVersions[] = {
			{ Core::GameVersion::DriverSanFrancisco_PC_1_0_4, { "1.04.1114", 0x00DD6480 } },
		};

		for (const auto& [gameVersion, versionInfo] : PossibleGameVersions)
		{
			if (versionInfo.StrAddr == kUnknownAddr) continue; // SKip, unable to check it

			const std::string valueInGame = reinterpret_cast<const char*>(versionInfo.StrAddr);
			if (valueInGame == versionInfo.Id)
				return gameVersion;
		}

		return Core::GameVersion::UnknownBuild;
	}

	int Core::EntryPoint(const void*)
	{
		if (!g_ClientInterface)
			return EXIT_FAILURE;

		g_ClientInterface->Run();

		while (g_ClientInterface->IsOnline())
		{
		}

		g_ClientInterface->OnDestroy();


		return EXIT_SUCCESS;
	}
}