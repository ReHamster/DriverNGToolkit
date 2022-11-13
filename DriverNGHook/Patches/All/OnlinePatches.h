#pragma once

#include "Patches/BasicPatch.h"
#include <HF/HackingFramework.hpp>

namespace DriverNG
{
    class OnlinePatches final : public BasicPatch
    {
        static constexpr size_t kHermesSetLogCallbackPatchSize = 5;

        HF::Hook::TrampolinePtr<kHermesSetLogCallbackPatchSize> m_HermesSetLogCallback;
    public:
        OnlinePatches() = default;

        const char* GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}