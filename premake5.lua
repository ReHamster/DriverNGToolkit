-- premake5.lua

require ".premake_modules/usage"
require ".premake_modules/asan"

-- Main workspace
workspace "DriverNGHook"
    language "C++"
	cppdialect "C++20"
    configurations { "Debug", "Release" }
	linkgroups 'On'
	characterset "MBCS"

	objdir "build"
	targetdir "bin/%{cfg.platform}/%{cfg.buildcfg}"
	location "project_%{_ACTION}"
	platforms "x86"
	
	filter "system:Windows"
		disablewarnings { "4996", "4554", "4244", "4101", "4838", "4309" }
		defines { 
			"NOMINMAX", 
			"_CRT_SECURE_NO_WARNINGS", "_CRT_SECURE_NO_DEPRECATE"
		}

    filter "configurations:Debug"
        defines { 
            "DEBUG"
        }
        symbols "On"

    filter "configurations:Release"
        defines {
            "NDEBUG",
        }
		optimize "On"
		symbols "On"
		-- enableASAN "On"

group "Dependencies"
		
-- dependencies are in separate configuration
include "src_dependency/premake5.lua"

group "Main"

project "DriverNGHook"
    kind "SharedLib"
	targetextension ".asi"	-- can be used with Ultimate ASI loader

    files {
        "DriverNGHook/**.cpp",
        "DriverNGHook/**.h",
	}

	includedirs {
		"DriverNGHook"
	}

	links {
		"d3d9", "dinput8", "dxguid"
	}

    uses { 
		"HF", 
		"lua", "luafilesystem", "sol2",
		"imgui"
	}