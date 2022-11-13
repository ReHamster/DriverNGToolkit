#pragma once

#include "Patches/BasicPatch.h"
#include <HF/HackingFramework.hpp>

namespace DriverNG
{
    class IDirect3DDelegate;

    class Direct3D9DevicePatches final : public BasicPatch
    {
    private:

        static constexpr size_t kDirect3DCreate9PatchSize = 5;

        HF::Hook::TrampolinePtr<10> m_ZDirect3DDevice_Constructor { nullptr };
        HF::Hook::TrampolinePtr<kDirect3DCreate9PatchSize> m_direct3DCreate9Hook;
    public:
        explicit Direct3D9DevicePatches(std::unique_ptr<IDirect3DDelegate>&& delegate);

        const char* GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };

}