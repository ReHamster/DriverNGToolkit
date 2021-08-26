#include <Client.h>

#include <HF/HackingFramework.hpp>
#include <cmdlib.h>

/// Delegates
#include <Delegates/DX9Delegate.h>
#include <Delegates/ImGuiInputDelegate.h>

/// Patches
#include <Patches/All/ZDirect3D9DevicePatches.h>
#include <Patches/All/InputDevicesPatches.h>
#include <Patches/All/LuaPatches.h>
#include <Patches/All/OnlinePatches.h>

namespace DriverNG
{
	static constexpr std::string_view kDirectX9DllName = "d3d9.dll";
	static constexpr std::string_view kProcessName = "Driver.exe";

	bool Client::OnAttach()
	{
		MsgInfo("----------[ DriverNG Hook initialization ]----------\n");

		if (!LocateModules()) 
			return false;

		// Register game configuration
		if (!RegisterGameConfigurationForHamster())
		{
			MsgError("Failed to register game configuration!\n");
			return false;
		}

		m_patches = std::make_shared<CommonPatches>(m_selfProcess, m_selfModule, m_d3d9Module);
		RegisterPatches();
		if (!m_patches->Setup())
		{
			MsgError("Failed to setup patches!\n");
			return false;
		}

		return true;
	}

	void Client::OnDestroy()
	{
		m_patches->Release();
		ReleaseModules();
	}

	bool Client::RegisterGameConfigurationForHamster()
	{
		return true;
	}

	bool Client::LocateModules()
	{
		auto PrintModInfo = [](const HF::Win32::Module::Ptr& mod)
		{
			if (!mod) {
				MsgError("BAD MODULE\n");
				return;
			}

			MsgInfo("Module: %s base address at 0x%x of size 0x%x\n", mod->getName().c_str(), mod->getBaseAddress(), mod->getSize());
		};

		m_selfProcess = std::make_shared<HF::Win32::Process>(kProcessName);

		if (!m_selfProcess->isValid())
		{
			MsgError("Failed to locate '%s' process\n", kProcessName.data());
			return false;
		}

		m_selfModule = m_selfProcess->getSelfModule();
		if (!m_selfModule)
		{
			MsgError("Failed to locate self module!\n");
			return false;
		}

		m_d3d9Module = m_selfProcess->getModule(kDirectX9DllName);
		if (!m_d3d9Module)
		{
			MsgError("Failed to locate '%s' module!\n", kDirectX9DllName.data());
			return false;
		}

#ifdef _DEBUG
		PrintModInfo(m_selfModule);
		PrintModInfo(m_d3d9Module);
#endif

		return true;
	}

	void Client::ReleaseModules()
	{
		m_selfModule = nullptr;
		m_d3d9Module = nullptr;
		m_selfProcess = nullptr;
	}

	void Client::RegisterPatches()
	{
		m_patches->RegisterPatch<LuaPatches>();
		m_patches->RegisterPatch<OnlinePatches>();

		const char* cmd = GetCommandLineA();

		if (strstr(cmd, "-tools") != nullptr)
		{
			m_patches->RegisterPatch<Direct3D9DevicePatches>(std::make_unique<DX9Delegate>());
			m_patches->RegisterPatch<InputDevicesPatches>(std::make_unique<ImGuiInputDelegate>());
		}
		else
		{
			m_patches->RegisterPatch<InputDevicesPatches>();
		}
	}
}