--------------------------------------------
-- DriverNG Hook layer
--------------------------------------------

setAllowDeveloperConsole(true)
allowCustomGameScripts(false)

Development = {
	enableDevText = false,
	list2DText = {}
}

function Development:useDevText(use)
	self.enableDevText = use
end

function Development:add2DText(id, text)
	table.insert(self.list2DText, {id = id, text = text})
end

function Development:clearScreen(what)
	self.list2DText = {}
end

--[[
registerForEvent("onInit", function()
	print("Initialized")
end)

registerForEvent("onUpdate", function()
	if Development.enableDevText == false then
		return
	end
	
	-- draw development view from the game
	if ImGui.Begin("Development view") then
		ImGui.SetWindowFontScale(1)
		
		for i,val in ipairs(Development.list2DText) do
			ImGui.Text(v)
		end

	end
	ImGui.End()

end)]]