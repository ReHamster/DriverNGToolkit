-- Hacking framework
usage "HF"
    includedirs {
		"HF/include"
	}

-- ImGui
project "imgui"
    kind "StaticLib"
    files {
        "imgui/imgui.cpp",
        "imgui/imgui.h",
        "imgui/imgui_draw.cpp",
        "imgui/imstb_truetype.h",
        "imgui/imstb_textedit.h",
        "imgui/imgui_tables.cpp",
        "imgui/imgui_widgets.cpp",
        "imgui/imstb_rectpack.h",
        "imgui/backends/imgui_impl_dx9.cpp",
        "imgui/backends/imgui_impl_win32.cpp"
	}
	includedirs {
		"imgui",
        "imgui/backends"
	}
    links {
        "d3d9"
    }

usage "imgui"
    includedirs {
        "imgui",
        "imgui/backends"
    }
    links "imgui"

-- Lua and stuff
project "lua"
	kind "StaticLib"
	files {
		"lua/src/*.c"
	}
	
	removefiles {
		"lua/src/lua.c",
	}

usage "lua"
	includedirs {
		"lua/src",
	}

	links { "lua" }

project "luafilesystem"
    kind "StaticLib"
    uses "lua"
    files {
        "luafilesystem/src/*.c"
    }

usage "luafilesystem"
	includedirs {
		"luafilesystem/src",
	}

	links { "luafilesystem" }

usage "sol2"
	includedirs {
		"sol2/include",
	}
