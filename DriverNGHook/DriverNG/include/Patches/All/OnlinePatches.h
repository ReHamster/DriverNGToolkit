#pragma once

#include <Patches/BasicPatch.h>
#include <HF/HackingFramework.hpp>
#include <cereal.hpp>

namespace DriverNG
{
    class OnlinePatches final : public BasicPatch
    {
        static constexpr size_t kSandboxSelectorConstructorPatchSize = 10;

        HF::Hook::TrampolinePtr<kSandboxSelectorConstructorPatchSize> m_SandboxSelectorConstructor;
    public:
        OnlinePatches() = default;

        std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}