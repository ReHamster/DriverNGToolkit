#pragma once

#include "Patches/BasicPatch.h"
#include "Delegates/ILuaDelegate.h"
#include <HF/HackingFramework.hpp>

namespace DriverNG
{
    class ILuaDelegate;

    class LuaPatches final : public BasicPatch
    {
        static constexpr size_t kOpenScriptLoaderPatchSize = 5;
        static constexpr size_t kStepLuaPatchSize = 5;
        static constexpr size_t kDeleteLuaStatePatchSize = 5;
        static constexpr size_t ksafe_vsprintfPatchSize = 5;
        static constexpr size_t kprofileNamePatchSize = 5;

        HF::Hook::TrampolinePtr<kOpenScriptLoaderPatchSize> m_openScriptLoaderHook;
        HF::Hook::TrampolinePtr<kDeleteLuaStatePatchSize> m_deleteLuaStateHook;
        HF::Hook::TrampolinePtr<kStepLuaPatchSize> m_stepLuaHook;
        HF::Hook::TrampolinePtr<ksafe_vsprintfPatchSize> m_safe_vsprintfHook;
        HF::Hook::TrampolinePtr<kprofileNamePatchSize> m_getProfileNameHook;
    public:
        LuaPatches() = default;

        const char* GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}