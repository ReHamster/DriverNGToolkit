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

		-- type, name, docs
		table.insert(luaDocData[libraryName], {"function", functionName, functionDocs})
	end,

	AddLuaPropertyDocumentation = function(libraryName, propertyName, propertyDocs)
		if luaDocData[libraryName] == nil then
			luaDocData[libraryName] = {}
		end

		-- type, name, docs
		table.insert(luaDocData[libraryName], {"property", propertyName, propertyDocs} )
	end,
}

luaHelp = function()
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
	--[[if libraryName ~= nil then
		util.printObject(luaDocData[libraryName])
	else
		-- only library names
		local libraryNames = {}
		for k,v in pairs(luaDocData) do
			table.insert(libraryNames, "library '" .. k .. "'")
		end
		util.printObject(libraryNames)
	end]]
	
	-- restore
	if not Documentation.init then
		Documentation.init = true
		
		LuaDocumentation_Initialise = oldFuncs.LuaDocumentation_Initialise
		LuaDocumentation_AddLuaLibrary = oldFuncs.LuaDocumentation_AddLuaLibrary
		LuaDocumentation_AddLuaFunctionDocumentation = oldFuncs.LuaDocumentation_AddLuaFunctionDocumentation
		LuaDocumentation_AddLuaPropertyDocumentation = oldFuncs.LuaDocumentation_AddLuaPropertyDocumentation
	end
	
	local function DrawDocs()
		ImGui.SetNextWindowSize(500, 400, ImGuiCond.FirstUseEver)
		if ImGui.Begin("Lua documentation") then
			if ImGui.Button("Hide") then
				ImGui_RemoveUpdateFunction("LuaDocs")
			end
			for name,tbl in pairs(luaDocData) do
				if ImGui.CollapsingHeader(name) then
									
					if ImGui.BeginTable("table1", 3) then
					
						ImGui.TableSetupColumn("Type", 0, 0.1)
						ImGui.TableSetupColumn("Name", 0, 0.25)
						ImGui.TableSetupColumn("Description", 0, 1.25)
						ImGui.TableHeadersRow()
					
						for i,mem_desc in pairs(tbl) do		
							ImGui.TableNextRow();

							ImGui.TableSetColumnIndex(0)
							ImGui.Text(mem_desc[1])
							
							ImGui.TableSetColumnIndex(1)
							ImGui.Text(mem_desc[2])
							
							ImGui.TableSetColumnIndex(2)
							ImGui.Text(mem_desc[3])

						end
						ImGui.EndTable()
					end
				end
			end
			ImGui.End()
		end
	end

	ImGui_AddUpdateFunction("LuaDocs", DrawDocs) -- true if you want to display it always on screen
end

