#pragma once

#include <Patches/BasicPatch.h>
#include <HF/HackingFramework.hpp>

namespace DriverNG
{
    class IInputDelegate;

    class InputDevicesPatches final : public BasicPatch
    {
        static constexpr size_t kRegisterClassAPatchSize = 6;
        static constexpr size_t kDirectInput8CreateSize = 5;

        HF::Hook::TrampolinePtr<kRegisterClassAPatchSize> m_registerClassAHook;
        HF::Hook::TrampolinePtr<kDirectInput8CreateSize> m_directInput8CreateHook;
    public:
        InputDevicesPatches() = default;
        explicit InputDevicesPatches(std::unique_ptr<IInputDelegate>&& delegate);

        std::string_view GetName() const override;
        bool Apply(const ModPack& modules) override;
        void Revert(const ModPack& modules) override;
    };
}