#pragma once

#include <Patches/BasicPatch.h>
#include <Delegates/ILuaDelegate.h>
#include <HF/HackingFramework.hpp>

namespace DriverNG
{
    class ILuaDelegate;

    class LuaPatches final : public BasicPatch
    {
        static constexpr size_t kStartLuaPatchSize = 5;
        static constexpr size_t kriLuaCallPatchSize = 5;

        HF::Hook::TrampolinePtr<kStartLuaPatchSize> m_startLuaHook;
        HF::Hook::TrampolinePtr<kriLuaCallPatchSize> m_riLuaCallHook;
    public:
        LuaPatches() = default;

        std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}