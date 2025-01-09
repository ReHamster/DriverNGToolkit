local struct = require(DNGHookScriptPath.. "struct")
local function writeArray(file, arr)
	local written = {}
	for id,value in pairs(arr) do
		if id ~= nil and value ~= nil then
			table.insert(written, {id, value})
		end
	end

	file:write(struct.pack('i', #written))
	for i, v in ipairs(written) do
		file:write(struct.pack('iB', v[1], v[2] and 1 or 0))
	end
end

local function readArray(file)
	local arr = {}
	local count = struct.unpack('i', file:read(4))

	for i = 1,count do
		local id, value = struct.unpack('iB', file:read(5))
		arr[id] = value > 0
	end
	return arr
end

local nullSub = function(...) end

local function CreateCustomSaveData()
	return {
		missionUnlocked = {},
		missionOwned = {},
		missionAttempted = {},
		missionCompleted = {},
		missionNew = {},
		
		dareUnlocked = {},
		dareCompleted = {},
		dareNew = {},
		
		challengeUnlocked = {},
		challengeOwned = {},
		challengeAttempted = {},
		challengeCompleted = {},
		challengeNew = {},
		
		collectableUnlocked = {},
		collectableOwned = {},
	}
end

local nativeProfileSettings = ProfileSettings
local CustomProfileSettings = {
	customDataLoaded = false,
	data = CreateCustomSaveData()
}

-- map functions
for k,func in pairs(nativeProfileSettings) do
	CustomProfileSettings[k] = func
end

local AUTOSAVE_CUSTOM_PATH = "\\savegames\\autosave_custom.dat"
local SAVE_SIGNATURE = "DNGS"
local SAVE_VERSION = 1

local function TryLoadProfileSettings()
	if CustomProfileSettings.customDataLoaded then
		return
	end
	
	-- load file
	if localPlayer == nil or localPlayer.name == nil or localPlayer.name:len() == 0 then
		return
	end
	
	CustomProfileSettings.data = CreateCustomSaveData()
	CustomProfileSettings.customDataLoaded = true
	
	print("--------- LOAD CUSTOM PROFILE SETTINGS ---------")
	print("player name ".. localPlayer.name)
	
	local saveFileName = UserDocumentsPath..localPlayer.name..AUTOSAVE_CUSTOM_PATH
	local file = io.open(saveFileName, "rb")
	if file == nil then
		print("No custom profile save data found at "..saveFileName)
		return
	end

	local data = CustomProfileSettings.data
	local sig, version = struct.unpack('c4i', file:read(8))
	if sig ~= SAVE_SIGNATURE or version ~= SAVE_VERSION then
		print("Invalid save file "..saveFileName)
		return
	end
	
	xpcall(function()
		data.missionUnlocked     = readArray(file)
		data.missionOwned        = readArray(file)
		data.missionAttempted    = readArray(file)
		data.missionCompleted    = readArray(file)
		data.missionNew          = readArray(file)
		data.dareUnlocked        = readArray(file)
		data.dareCompleted       = readArray(file)
		data.dareNew             = readArray(file)
		data.challengeUnlocked   = readArray(file)
		data.challengeOwned      = readArray(file)
		data.challengeAttempted  = readArray(file)
		data.challengeCompleted  = readArray(file)
		data.challengeNew        = readArray(file)
		data.collectableUnlocked = readArray(file)
		data.collectableOwned	 = readArray(file)
	end,
	function(err)
		print(tostring(err))
		print(debug.traceback())
	end)
end

local function TrySaveProfileSettings()
	print("--------- SAVE CUSTOM PROFILE SETTINGS ---------")
	
	CustomProfileSettings.customDataLoaded = true
	
	local saveFileName = UserDocumentsPath..localPlayer.name..AUTOSAVE_CUSTOM_PATH
	local file = io.open(saveFileName, "wb")
	if file == nil then
		print("Cannot store custom profile save data at "..saveFileName)
		return
	end
	
	local data = CustomProfileSettings.data
	local packed = struct.pack('<c4i', SAVE_SIGNATURE, SAVE_VERSION)
	file:write(packed)
	
	writeArray(file, data.missionUnlocked)
	writeArray(file, data.missionOwned)
	writeArray(file, data.missionAttempted)
	writeArray(file, data.missionCompleted)
	writeArray(file, data.missionNew)
	writeArray(file, data.dareUnlocked)
	writeArray(file, data.dareCompleted)
	writeArray(file, data.dareNew)
	writeArray(file, data.challengeUnlocked)
	writeArray(file, data.challengeOwned)
	writeArray(file, data.challengeAttempted)
	writeArray(file, data.challengeCompleted)
	writeArray(file, data.challengeNew)
	writeArray(file, data.collectableUnlocked)
	writeArray(file, data.collectableOwned)
end

-- Triggers an autosave (if enabled)
CustomProfileSettings.TriggerAutoSave = function()
	-- save our stuff

	nativeProfileSettings.TriggerAutoSave()
	TrySaveProfileSettings()
end

-- Clears all progression data from the profile
CustomProfileSettings.ClearProgression = function()
	-- clear our stuff first
	CustomProfileSettings.data = CreateCustomSaveData()
	CustomProfileSettings.customDataLoaded = true
	return nativeProfileSettings.ClearProgression()
end

-- PARAM<1,MissionID>Get whether a mission has been unlocked or not
CustomProfileSettings.GetMissionUnlocked = function(missionId)
	TryLoadProfileSettings()
	if CustomProfileSettings.data.missionUnlocked[missionId] ~= nil then
		return CustomProfileSettings.data.missionUnlocked[missionId]
	end
	return nativeProfileSettings.GetMissionUnlocked(missionId)
end

-- PARAM<1,MissionID>Set mission unlocked (also marks as new if not already unlocked)
CustomProfileSettings.SetMissionUnlocked = function(missionId)
	TryLoadProfileSettings()
	if not CustomProfileSettings.data.missionUnlocked[missionId] then
		CustomProfileSettings.data.missionNew[missionId] = true
	end
	CustomProfileSettings.data.missionUnlocked[missionId] = true
end

-- PARAM<1,MissionID>Get whether a mission is owned or not
CustomProfileSettings.GetMissionOwned = function(missionId)
	TryLoadProfileSettings()
	if CustomProfileSettings.data.missionOwned[missionId] ~= nil then
		return CustomProfileSettings.data.missionOwned[missionId]
	end
	return nativeProfileSettings.GetMissionOwned(missionId)
end

-- PARAM<1,MissionID>Set mission owned
CustomProfileSettings.SetMissionOwned = function(missionId)
	CustomProfileSettings.data.missionOwned[missionId] = true
end

-- PARAM<1,MissionID>Get whether a mission has been attempted or not
CustomProfileSettings.GetMissionAttempted = function(missionId)
	TryLoadProfileSettings()
	if CustomProfileSettings.data.missionAttempted[missionId] ~= nil then
		return CustomProfileSettings.data.missionAttempted[missionId]
	end
	return nativeProfileSettings.GetMissionAttempted(missionId)
end

-- PARAM<1,MissionID>Set mission attempted
CustomProfileSettings.SetMissionAttempted = function(missionId)
	CustomProfileSettings.data.missionAttempted[missionId] = true
end

-- PARAM<1,MissionID>Get whether a mission has been completed or not
CustomProfileSettings.GetMissionCompleted = function(missionId)
	TryLoadProfileSettings()
	if CustomProfileSettings.data.missionCompleted[missionId] ~= nil then
		return CustomProfileSettings.data.missionCompleted[missionId]
	end
	return nativeProfileSettings.GetMissionCompleted(missionId)
end

-- PARAM<1,MissionID>Set mission completed
CustomProfileSettings.SetMissionCompleted = function(missionId)
	CustomProfileSettings.data.missionAttempted[missionId] = true
end

-- PARAM<1,MissionID>Get whether a mission is new or not
CustomProfileSettings.GetMissionNew = function(missionId)
	TryLoadProfileSettings()
	if CustomProfileSettings.data.missionNew[missionId] ~= nil then
		return CustomProfileSettings.data.missionNew[missionId]
	end
	return nativeProfileSettings.GetMissionNew(missionId)
end

-- PARAM<1,MissionID>Clears the newness of a mission
CustomProfileSettings.ClearMissionNew = function(missionId)
	CustomProfileSettings.data.missionNew[missionId] = false
end

-- PARAM<1,DareID>Get whether a dare has been unlocked or not
CustomProfileSettings.GetDareUnlocked = function(dareId)
	TryLoadProfileSettings()
	if CustomProfileSettings.data.dareUnlocked[dareId] ~= nil then
		return CustomProfileSettings.data.dareUnlocked[dareId]
	end
	return nativeProfileSettings.GetDareUnlocked(dareId)
end

-- PARAM<1,DareID>Set dare unlocked (also marks as new if not already unlocked)
CustomProfileSettings.SetDareUnlocked = function(dareId)
	if not CustomProfileSettings.data.dareUnlocked[dareId] then
		CustomProfileSettings.data.dareNew[dareId] = true
	end
	CustomProfileSettings.data.dareUnlocked[dareId] = true
end

-- PARAM<1,DareID>Get whether a dare has been completed or not
CustomProfileSettings.GetDareCompleted = function(dareId)
	TryLoadProfileSettings()
	if CustomProfileSettings.data.dareCompleted[dareId] ~= nil then
		return CustomProfileSettings.data.dareCompleted[dareId]
	end
	return nativeProfileSettings.GetDareCompleted(dareId)
end

-- PARAM<1,DareID>Set dare completed
CustomProfileSettings.SetDareCompleted = function(dareId)
	CustomProfileSettings.data.dareCompleted[dareId] = true
end

-- PARAM<1,DareID>Get whether a dare is new or not
CustomProfileSettings.GetDareNew = function(dareId)
	TryLoadProfileSettings()
	if CustomProfileSettings.data.dareNew[dareId] ~= nil then
		return CustomProfileSettings.data.dareNew[dareId]
	end
	return nativeProfileSettings.GetDareNew(dareId)
end

-- PARAM<1,DareID>Clear the newness of a dare
CustomProfileSettings.ClearDareNew = function(dareId)
	CustomProfileSettings.data.dareNew[dareId] = false
end

-- PARAM<1,ChallengeID>Get whether a challenge has been unlocked
CustomProfileSettings.GetChallengeUnlocked = function(challengeId)
	TryLoadProfileSettings()
	return nativeProfileSettings.GetChallengeUnlocked(challengeId)
end

-- PARAM<3,ChallengeID,bool Activity, bool Movie>Set challenge unlocked (also marks as new if not already unlocked)
CustomProfileSettings.SetChallengeUnlocked = function(challengeId, isActivity, isMovie)
	if not CustomProfileSettings.data.challengeUnlocked[challengeId] then
		CustomProfileSettings.data.challengeNew[challengeId] = true
	end
	CustomProfileSettings.data.challengeUnlocked[challengeId] = true
end

-- PARAM<1,ChallengeID>Get whether a challenge has been owned
CustomProfileSettings.GetChallengeOwned = function(challengeId)
	TryLoadProfileSettings()
	if CustomProfileSettings.data.challengeOwned[challengeId] ~= nil then
		return CustomProfileSettings.data.challengeOwned[challengeId]
	end
	return nativeProfileSettings.GetChallengeOwned(challengeId)
end

-- PARAM<1,ChallengeID>Set challenge owned
CustomProfileSettings.SetChallengeOwned = function(challengeId)
	CustomProfileSettings.data.challengeOwned[challengeId] = true
end

-- PARAM<1,ChallengeID>Get whether a challenge has been attempted or not
CustomProfileSettings.GetChallengeAttempted = function(challengeId)
	TryLoadProfileSettings()
	if CustomProfileSettings.data.challengeAttempted[challengeId] ~= nil then
		return CustomProfileSettings.data.challengeAttempted[challengeId]
	end
	return nativeProfileSettings.GetChallengeAttempted(challengeId)
end

-- PARAM<1,ChallengeID>Set challenge attempted
CustomProfileSettings.SetChallengeAttempted = function(challengeId)
	CustomProfileSettings.data.challengeAttempted[challengeId] = true
end

-- PARAM<1,ChallengeID>Get whether a challenge has been completed
CustomProfileSettings.GetChallengeCompleted = function(challengeId)
	TryLoadProfileSettings()
	return nativeProfileSettings.GetChallengeCompleted(challengeId)
end

-- PARAM<1,ChallengeID>Set challenge completed
CustomProfileSettings.SetChallengeCompleted = function(challengeId)
	CustomProfileSettings.data.challengeCompleted[challengeId] = true
end

-- PARAM<1,ChallengeID>Get whether a challenge is new or not
CustomProfileSettings.GetChallengeNew = function(challengeId)
	TryLoadProfileSettings()
	if CustomProfileSettings.data.challengeCompleted[challengeId] ~= nil then
		return CustomProfileSettings.data.challengeCompleted[challengeId]
	end
	return nativeProfileSettings.GetChallengeCompleted(challengeId)
end

-- PARAM<1,ChallengeID>Clear the newness of a challenge
CustomProfileSettings.ClearChallengeNew = function(challengeId)
	CustomProfileSettings.data.challengeNew[challengeId] = false
end

-- PARAM<1,CollectableID>Gets whether a collectable is unlocked or not
CustomProfileSettings.GetCollectableUnlocked = function(collectableID)
	TryLoadProfileSettings()
	if CustomProfileSettings.data.collectableUnlocked[collectableID] ~= nil then
		return CustomProfileSettings.data.collectableUnlocked[collectableID]
	end
	return nativeProfileSettings.GetCollectableUnlocked(collectableID)
end

-- PARAM<1,CollectableID>Sets a collectable as unlocked
CustomProfileSettings.SetCollectableUnlocked = function(collectableID)
	CustomProfileSettings.data.collectableUnlocked[collectableID] = true
end

-- PARAM<1,CollectableID>Gets whether a collectable is owned or not
CustomProfileSettings.GetCollectableOwned = function(collectableID)
	TryLoadProfileSettings()
	if CustomProfileSettings.data.collectableOwned[collectableID] ~= nil then
		return CustomProfileSettings.data.collectableOwned[collectableID]
	end
	return nativeProfileSettings.GetCollectableOwned(collectableID)
end

-- PARAM<1,CollectableID>Sets a collectable as owned
CustomProfileSettings.SetCollectableOwned = function(collectableID)
	CustomProfileSettings.data.collectableOwned[collectableID] = true
end

-- override
ProfileSettings = CustomProfileSettings


--[[
local function DrawProfileSettingsDebug()
	
	local ImGuiWindowFlags = ImGui.constant.WindowFlags
	
	local flags = ImGuiWindowFlags.NoTitleBar + ImGuiWindowFlags.NoResize + ImGuiWindowFlags.NoBringToFrontOnFocus

	if ImGui.Begin("ProfileSettings", true, flags) then
		
		ImGui.SetWindowSize(1024, 1024)
		ImGui.SetWindowPos(0, 24)

		for i,v in pairs(Development.TextList) do
			--ImGui.SetCursorScreenPos(v.position[0] * 512, v.position[1] * 512)
			ImGui.SetCursorPosX(v.position[0] * 256)
			ImGui.TextColored(v.color[0], v.color[1], v.color[2], v.color[3], v.text)
		end

		ImGui.End()
	end
end

ImGui_RemoveUpdateFunction("ProfileSettingsDebug")
ImGui_AddUpdateFunction("ProfileSettingsDebug", DrawProfileSettingsDebug, true)
]]