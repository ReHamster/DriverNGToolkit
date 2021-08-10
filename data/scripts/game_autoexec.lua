--------------------------------------------
-- DriverNG Game layer
--------------------------------------------

driverNGHook = {
	gamePaused = false,
	gameLaunched = false
}

UserLuaMediaPath = MediaPath .. "UserLuaScripts\\"
--LuaMediaPath = MediaPath .. "LuaScripts\\"

local function diskFileExists(filename)
	local f = io.open(filename, "r")
	if f ~= nil then
		io.close(f)
		return true
	end
	return false
end

-- loads the Lua file and executes it
function _open(filename)

	local doFileFunc = nil
	
	-- override files if found unpacked
	-- FIXME: check allowed files for Online?
	if diskFileExists(LuaMediaPath .. filename) then
		--print("[NOTE] loading '" .. filename .. "' from disk")
		doFileFunc = ScriptLoader.ri_dofilefromdisk
	else
		--print("[NOTE] loading '" .. filename .. "' from CHUNK")
		doFileFunc = ScriptLoader.ri_dofile
	end

	local error, f = doFileFunc(LuaMediaPath .. filename)
	
	local success = true
	
	if type(f) == "string" then
		print("\t" .. f)
		success = false		-- this for some reason is incorrect
	elseif f == nil then
		print("\t" .. error)
		success = false
	else
		f()
	end
	
	return success
end

--------------------------------------------------------------------------------

function user_fileExists(filename)
	local f = io.open(UserLuaMediaPath .. filename, "r")
	if f ~= nil then
		io.close(f)
		return true
	end
	return false
end

-- loads the Lua file and executes it the game way
function _user_open(filename)

	print("[NOTE] loading user file '" .. filename .. "'")

	local error, f = ScriptLoader.ri_dofilefromdisk(UserLuaMediaPath .. filename)
	
	if type(f) == "string" then
		print("\t" .. f)
		return true, nil
	elseif type(f) == "function" then
		f()
		return true, nil
	elseif type(f) == "table" then
		return true, f
	elseif f == nil then
		print("\t" .. error)
		return false, nil
	else
		print("Unknown return type???")
	end
	
	return false
end

-------------------------------------------------------------------------------------------
-- self-reloading

-- TODO: search for MODS folders and ModInit.lua
local bootstrapUserFileName = "bootstrap.lua"
local bootstrapUserFileTime = nil
local bootstrapper = nil

local function checkLoadBootstrapper()
	local t = file.getmodifytime(UserLuaMediaPath .. bootstrapUserFileName) or ""

	if t ~= bootstrapUserFileTime then
		bootstrapUserFileTime = t
		if bootstrapper ~= nil then
			bootstrapper.Shutdown()
		end
		
		-- load and init bootrapper
		local result, bs = user_open( bootstrapUserFileName )
		if result and bs ~= nil then
			bs.Init()
			
			bootstrapper = bs
		end
	end
end

local function overrideGameLauncher()
	dofile(DNGHookScriptPath .. "driverNGGameLauncher.lua")
	removeUserUpdateFunction("overrideGameLauncher")

	addUserUpdateFunction("userBootstrapper", checkLoadBootstrapper, 120)
end

--------------------------------------------------------------------------------
-- Strongly define hook overrides
-- Protect them from setting by the game

local _launchScripts = nil
local function hookLaunchScripts()
	if bootstrapper ~= nil then
		bootstrapper.Launch()
	end

	driverNGHook.gameLaunched = true

	_launchScripts()
end

local ReadOnly = {
	open = _open,
	user_open = _user_open,
	launchScripts = hookLaunchScripts
}

print("System overrides added")

local function check(tab, name, value)
	if name == "GameLauncher" then
		addUserUpdateFunction("overrideGameLauncher", overrideGameLauncher, 1)
	elseif name == "launchScripts" then
		_launchScripts = value
	end

	if rawget(ReadOnly, name) then
		-- TODO: we can always store original value here
		if name == "open" then
			-- don't message
			rawset(tab, name, _open)
		else
			print(name .." is a read only variable")
		end
		
	else
		rawset(tab, name, value)
	end
end

setmetatable(_G, {__index=ReadOnly, __newindex=check})
