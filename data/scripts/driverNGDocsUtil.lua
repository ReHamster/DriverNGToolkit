local luaDocData

if Documentation ~= nil then
	Documentation.init = false
end

local EngineFuncs = {
	-- called from engine
	Initialise = function()
		luaDocData = {}
	end,

	AddLuaLibrary = function(libraryName)
		luaDocData[libraryName] = {}
	end,

	AddLuaFunctionDocumentation = function(libraryName, functionName, functionDocs)
		if luaDocData[libraryName] == nil then
			luaDocData[libraryName] = {}
		end

		table.insert(luaDocData[libraryName], "function " .. functionName .. "   :  " .. functionDocs)
	end,

	AddLuaPropertyDocumentation = function(libraryName, propertyName, propertyDocs)
		if luaDocData[libraryName] == nil then
			luaDocData[libraryName] = {}
		end

		table.insert(luaDocData[libraryName], "property " .. propertyName .. "  :  " .. propertyDocs)
	end,
}

luaHelp = function( libraryName )
	local oldFuncs = {}

	-- prepare
	if not Documentation.init then
		oldFuncs.LuaDocumentation_Initialise = LuaDocumentation_Initialise
		oldFuncs.LuaDocumentation_AddLuaLibrary = LuaDocumentation_AddLuaLibrary
		oldFuncs.LuaDocumentation_AddLuaFunctionDocumentation = LuaDocumentation_AddLuaFunctionDocumentation
		oldFuncs.LuaDocumentation_AddLuaPropertyDocumentation = LuaDocumentation_AddLuaPropertyDocumentation
	
		-- init functions
		LuaDocumentation_Initialise = EngineFuncs.Initialise
		LuaDocumentation_AddLuaLibrary = EngineFuncs.AddLuaLibrary
		LuaDocumentation_AddLuaFunctionDocumentation = EngineFuncs.AddLuaFunctionDocumentation
		LuaDocumentation_AddLuaPropertyDocumentation = EngineFuncs.AddLuaPropertyDocumentation
		
		-- collect
		Documentation.collectLuaDocumentation()
	end
	
	-- print
	if libraryName ~= nil then
		util.printObject(luaDocData[libraryName])
	else
		-- only library names
		local libraryNames = {}
		for k,v in pairs(luaDocData) do
			table.insert(libraryNames, "library '" .. k .. "'")
		end
		util.printObject(libraryNames)
	end
	
	-- restore
	if not Documentation.init then
		Documentation.init = true
		
		LuaDocumentation_Initialise = oldFuncs.LuaDocumentation_Initialise
		LuaDocumentation_AddLuaLibrary = oldFuncs.LuaDocumentation_AddLuaLibrary
		LuaDocumentation_AddLuaFunctionDocumentation = oldFuncs.LuaDocumentation_AddLuaFunctionDocumentation
		LuaDocumentation_AddLuaPropertyDocumentation = oldFuncs.LuaDocumentation_AddLuaPropertyDocumentation
	end
end