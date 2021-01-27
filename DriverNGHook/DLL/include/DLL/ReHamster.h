#pragma once

namespace ReHamster
{
    class Core
    {
    public:
        static int EntryPoint(const void*);

    private:
        enum class GameVersion {
            DriverSanFrancisco_PC_1_0_4,
            // Not a version
            UnknownBuild = 0xDEAD
        };

        static GameVersion TryToDetectGameVersion();
    };
}