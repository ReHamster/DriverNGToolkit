local nullSub = function(...) end

Development = {

	TextList = {},
	Enabled = false,

	add3DText = nullSub,
	setDrawTextElement = nullSub,
	setDrawDevTextBatch = nullSub,
	get2DTextSize = nullSub,
	removeDevGraphicsBatch = nullSub,
	setDrawDevGraphicsBatch = nullSub,
	add2DGraphicsTransform = nullSub,
	setDrawGraphics = nullSub,
	addGraphicsHeading = nullSub,
	dropBatchModelList = nullSub,
	addGraphics = nullSub,
	addGraphicsTransform = nullSub,
	endDevGraphicsBatch = nullSub,
	useDevGraph = nullSub,
	add2DGraphicsHeading = nullSub,
	getUseDevGraph = nullSub,
	removeGraphics = nullSub,
	startDevGraphicsBatch = nullSub,
}

function Development:add2DText(index, text, position, color, scale, unknown)
	self.TextList[index] = {
		text = text, 
		position = position,
		color = color, 
		scale = scale,
		unknown = unknown
	}
end -- = add2DText(const unsigned int, const char *, const class MAv4, const class MAv4, const float, const float)

function Development:clearScreen(clearType)
	self.TextList = {}
end --  = void clearScreen(enum Dev::Text::clearType)


function Development:getUseDevText()
	return self.Enabled
end --  = bool getUseDevText()


function Development:useDevText(enable)
	self.Enabled = enable
end --  = void setUseDevText(bool)


function Development:endDevTextBatch()
end --  = void endBatch()


function Development:removeDevTextBatch(idx)
	self.TextList = {}
end --  = void removeBatch(unsigned int)


function Development:startDevTextBatch(idx)
	self.TextList = {}
end --  = void startBatch(unsigned int)


function Development:eraseText(idx)
	self.TextList[idx] = nil
end --  = bool eraseText(unsigned int)

local function DrawDevelopment()
	
	if #Development.TextList == 0 then
		return
	end
	
	local ImGuiWindowFlags = ImGui.constant.WindowFlags
	
	local flags = ImGuiWindowFlags.NoTitleBar + ImGuiWindowFlags.NoResize + ImGuiWindowFlags.NoBringToFrontOnFocus

	if ImGui.Begin("DevelopmentFrame", true, flags) then
		
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

ImGui_RemoveUpdateFunction("Development")
ImGui_AddUpdateFunction("Development", DrawDevelopment, true) -- true if you want to display it always on screen

--open("configuration\\gamesetupmenu.lua")
