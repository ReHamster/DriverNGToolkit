#pragma once

#include <memory>
#include <string>

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

        virtual const char* GetName() const = 0;
        virtual bool Apply(const ModPack& modules) { m_applied = true; return true; }
        virtual void Revert(const ModPack& modules) { m_applied = false; return; }

        bool IsApplied() const { return m_applied; }
    };
}