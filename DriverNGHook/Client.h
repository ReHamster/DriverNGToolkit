#pragma once

#include <memory>
#include "IClient.h"
#include "Patches/CommonPatches.h"
#include <HF/HackingFrameworkFWD.h>

namespace DriverNG
{
    class Client : public ReHamster::Client::IClient
    {
    public:
        bool OnAttach() override;
        void OnDestroy() override;

    private:
        static bool RegisterGameConfigurationForHamster();

        bool LocateModules();
        void ReleaseModules();
        void RegisterPatches();

    private:
        HF::Win32::ProcessPtr m_selfProcess { nullptr };
        HF::Win32::ModulePtr m_selfModule { nullptr };
        HF::Win32::ModulePtr m_d3d9Module { nullptr };
        CommonPatches::Ptr m_patches { nullptr };
    };
}