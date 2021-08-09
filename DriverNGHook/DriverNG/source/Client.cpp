#include <Client.h>

#include <HF/HackingFramework.hpp>
#include <spdlog/spdlog.h>

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
		spdlog::info("----------[ DriverNG Hook initialization ]----------");

		if (!LocateModules()) 
			return false;

		// Register game configuration
		if (!RegisterGameConfigurationForHamster())
		{
			spdlog::error("Failed to register game configuration!");
			return false;
		}

		m_patches = std::make_shared<CommonPatches>(m_selfProcess, m_selfModule, m_d3d9Module);
		RegisterPatches();
		if (!m_patches->Setup())
		{
			spdlog::error("Failed to setup patches!");
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
				spdlog::error("BAD MODULE");
				return;
			}

			spdlog::info("Module: {} base address at {:08x} of size {:08x}", mod->getName(), mod->getBaseAddress(), mod->getSize());
		};

		m_selfProcess = std::make_shared<HF::Win32::Process>(kProcessName);

		if (!m_selfProcess->isValid())
		{
			spdlog::error("Failed to locate {} process", kProcessName);
			return false;
		}

		m_selfModule = m_selfProcess->getSelfModule();
		if (!m_selfModule)
		{
			spdlog::error("Failed to locate self module!");
			return false;
		}

		m_d3d9Module = m_selfProcess->getModule(kDirectX9DllName);
		if (!m_d3d9Module)
		{
			spdlog::error("Failed to locate {} module!", kDirectX9DllName);
			return false;
		}

		PrintModInfo(m_selfModule);
		PrintModInfo(m_d3d9Module);

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
		m_patches->RegisterPatch<Direct3D9DevicePatches>(std::make_unique<DX9Delegate>());
		m_patches->RegisterPatch<InputDevicesPatches>(std::make_unique<ImGuiInputDelegate>());
		m_patches->RegisterPatch<LuaPatches>();
		m_patches->RegisterPatch<OnlinePatches>();
	}
}