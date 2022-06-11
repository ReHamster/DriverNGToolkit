#pragma once

#include <memory>
#include <string_view>

#include "Patches/ModPack.h"

namespace DriverNG
{
    class BasicPatch
    {
    private:
        bool m_applied { false };
    public:
        using Ptr = std::shared_ptr<BasicPatch>;
        using Ref = std::weak_ptr<BasicPatch>;

        virtual ~BasicPatch() noexcept = default;

        virtual std::string_view GetName() const = 0;
        virtual bool Apply(const ModPack& modules) { m_applied = true; return true; }
        virtual void Revert(const ModPack& modules) { m_applied = false; return; }

        [[nodiscard]] bool IsApplied() const { return m_applied; }
    };
}