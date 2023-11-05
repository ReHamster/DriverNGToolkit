local menuShown = false

local function dngHookMenu()
    local toggle = ImGui.IsKeyPressed(0x71) -- VK_F2
    
    if toggle then
        menuShown = not menuShown
    end

    if menuShown and ImGui.BeginMainMenuBar() then
		if ImGui.BeginMenu("File") then

			ImGui.EndMenu()
		end

		if ImGui.BeginMenu("View") then

			ImGui.EndMenu();
		end

        if ImGui.BeginMenu("World") then

            ImGui.EndMenu();
        end
        
        if ImGui.BeginMenu("Game") then

            ImGui.EndMenu()
        end

        if ImGui.BeginMenu("Utilities") then

            ImGui.EndMenu()
        end
		
		if ImGui.BeginMenu("Help") then
			if ImGui.MenuItem("API documentation") then
				documentationWindowShown = not documentationWindowShown
			end
			ImGui.EndMenu()
		end

		ImGui.EndMainMenuBar();
	end

    return menuShown
end

ImGui_AddUpdateFunction("Menu", dngHookMenu, true)