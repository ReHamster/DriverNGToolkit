#include <DLL/ReHamster.h>
#include <DLL/Logger.h>
#include <DLL/DebugConsole.h>
#include <DLL/CrashHandlerReporter.h>
#include <Client/IClient.h>
#include <Client.h>

#include <Windows.h>

#include <spdlog/spdlog.h>

namespace ReHamster
{
	Client::IClient* g_ClientInterface = nullptr;

	Client::IClient* CreateClientInterface()
	{
		return new DriverNG::Client();
	}

	Core::GameVersion Core::TryToDetectGameVersion()
	{
		struct VersionDef
		{
			std::string_view Id;
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

			const auto valueInGame = std::string_view{ reinterpret_cast<const char*>(versionInfo.StrAddr) };
			if (valueInGame == versionInfo.Id)
				return gameVersion;
		}

		return Core::GameVersion::UnknownBuild;
	}

	int Core::EntryPoint(const void*)
	{
		//ReHamster::DebugConsole::Create("DriverNG Hook | Developer Console");
		ReHamster::Logger::Setup();

		CrashHandlerReporter crashHandlerReporter;

		// Setup client core
		g_ClientInterface = CreateClientInterface();
		RH_ASSERT2(g_ClientInterface != nullptr, "Bad g_ClientInterface interface! Probably CreateClientInterface() failed!");
		if (!g_ClientInterface)
			return EXIT_FAILURE;

		spdlog::info("Core::EntryPoint() run ...");

		const auto gameVersion = TryToDetectGameVersion();
		bool isOkVersion = false;

		switch (gameVersion)
		{
		case GameVersion::DriverSanFrancisco_PC_1_0_4:
			spdlog::info("Core::EntryPoint() | Detected '1.04.1114' version. Version is OK");
			isOkVersion = true;
			break;
		case GameVersion::UnknownBuild:
			spdlog::error("Core::EntryPoint() Failed. Unknown or invalid game version");
			break;
		}

		if (!isOkVersion)
		{
			spdlog::error(" *** The game version was rejected. DriverNGHook will shutdown, the game will work normally! *** ");

			MessageBox(nullptr, "Sorry, but this game not supported yet", "DriverNGHook| Game not supported", MB_ICONEXCLAMATION | MB_OK);
			delete g_ClientInterface;
			ReHamster::Logger::Shutdown();
			return EXIT_FAILURE;
		}

		if (!g_ClientInterface->OnAttach())
		{
			RH_ASSERT2(g_ClientInterface->OnAttach(), "g_ClientInterface->OnAttach() failed! See console for details!");
			ReHamster::Logger::Shutdown();
			return EXIT_FAILURE;
		}

		g_ClientInterface->Run();

		while (g_ClientInterface->IsOnline())
		{
		}

		g_ClientInterface->OnDestroy();

		spdlog::info("Core::EntryPoint() shutdown...");

		ReHamster::Logger::Shutdown();

		return EXIT_SUCCESS;
	}
}