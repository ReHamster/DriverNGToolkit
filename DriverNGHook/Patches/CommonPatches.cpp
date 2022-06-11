#include "Patches/CommonPatches.h"
#include "Logger.h"
#include <HF/HackingFramework.hpp>

namespace DriverNG
{
    CommonPatches::CommonPatches(
            const HF::Win32::ProcessPtr& process,
            const HF::Win32::ModulePtr& selfModule,
            const HF::Win32::ModulePtr& d3d9
    )
        : m_process(process)
        , m_selfMod(selfModule)
        , m_d3d9Mod(d3d9)
    {
    }

    bool CommonPatches::Setup()
    {
        if (m_isInited)
        {
            MsgWarning("CommonPatches::Setup(). Module is already inited!\n");
            return false;
        }

        ModPack modPack { m_process.lock(), m_selfMod.lock(), m_d3d9Mod.lock() };
        for (const auto& patch : m_patches)
        {
            if (!patch->IsApplied())
            {
                if (!patch->Apply(modPack))
                {
					MsgWarning("Failed to apply patch '%s'\n", patch->GetName().data());
                }
                else
                {
                    MsgInfo("Patch '%s' applied!\n", patch->GetName().data());
                    if (!patch->IsApplied())
                    {
                        MsgError("BAD PATCH IMPLEMENTATION! PATCH '%s' MUST BE APPLIED BUT IT DOESN'T!\n", patch->GetName().data());
                        continue;
                    }
                }
            }
        }

        m_isInited = true;
        return true;
    }

    void CommonPatches::Release()
    {
        if (m_isInited)
        {
            ModPack modPack { m_process.lock(), m_selfMod.lock(), m_d3d9Mod.lock() };

            for (const auto& patch : m_patches)
            {
                if (patch->IsApplied())
                {
                    patch->Revert(modPack);
                }

                if (patch->IsApplied())
                {
                    MsgError("BAD PATCH IMPLEMENTATION! PATCH '%s' MUST BE REVERTED BUT IT DOESN'T!\n", patch->GetName().data());
                }
            }

            m_isInited = false;
        }
    }
}