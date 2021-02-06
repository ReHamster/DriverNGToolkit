#pragma once

#include <Patches/BasicPatch.h>
#include <Delegates/ILuaDelegate.h>
#include <HF/HackingFramework.hpp>

namespace DriverNG
{
    class ILuaDelegate;

    class LuaPatches final : public BasicPatch
    {
        static constexpr size_t kOpenScriptLoaderPatchSize = 5;
        static constexpr size_t kStepLuaPatchSize = 5;

        HF::Hook::TrampolinePtr<kOpenScriptLoaderPatchSize> m_openScriptLoaderHook;
        HF::Hook::TrampolinePtr<kStepLuaPatchSize> m_stepLuaHook;
    public:
        LuaPatches() = default;

        std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}