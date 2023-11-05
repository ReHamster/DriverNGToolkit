function spawnVehicleEx(modelID, vehicleParams, characters)
  local function spawn()
    vehicleParams = vehicleParams or {}
    
    local params = {
      heading = 0,
      modelID = modelID or 0,
      snapToTerrain = vehicleParams.snapToTerrain or true,
      panelSet = mods.panelSetOverride or vehicleParams.panelSet or nil,
      shader = {
        [0] = vehicleParams.chosenShader or framework.random(0,8)
      },
    }
    local mt = {__index = vehicleParams, __newindex = function(t, k, v) rawset(vehicleParams, k, v) end}
    setmetatable(params, mt)
    if localPlayer.currentVehicle then
      params.position = localPlayer.position + -localPlayer.currentVehicle.matrix[0] * 5
      params.heading = localPlayer.heading
    else
      params.position = player.position + vec.vector(5, 0, 0, 1)
    end
    local vehicle = vehicleManager.spawnVehicle(params)
    vehicle.owner = "Script"
    if characters then
      for i = 1, #characters do
        GameVehicleResource.setCharacterSpoolingEntityIndex(vehicle.gameVehicle, i - 1, characters[i])
      end
    else
      -- clear passengers
      for i = 1, 3 do
        GameVehicleResource.setCharacterSpoolingEntityIndex(vehicle.gameVehicle, i, "-1")
      end
    end
    
    local rigID = 285
    local bigRigID = 286
    local trailerID = 123

    local trailerIDs = {
      123,
      131,
      289,
    }
    
    if modelID == bigRigID or modelID == rigID then
      local selTrailerId = trailerIDs[framework.random(1, 3)]
      
      -- make sure special trailers are spooled in
      if selTrailerId ~= trailerID then
        local trailerLoaded = TrafficSpooler.IsMissionVehicleLoaded(selTrailerId)
        local tries = 1
        
        while not trailerLoaded do
          if tries > 3 then
            print("**** COULD NOT SPAWN TRAILER WITH ID ".. selTrailerId .." - USING DEFAULT! ****")
            selTrailerId = trailerID
            break
          end
          
          TrafficSpooler.RequestMissionVehicle(selTrailerId)
          trailerLoaded = TrafficSpooler.IsMissionVehicleLoaded(selTrailerId)
          tries = tries + 1
        end
      end
      
      GameVehicleResource.createTrailerAndHookup({
        gameVehicle = vehicle.gameVehicle,
        trailerId = selTrailerId,
        panelSet = framework.random(0,8)
      })
    end
    
    return vehicle
  end
  local function waitForSpooling(modelID)
    TrafficSpooler.SetAsPlayerVehicle(modelID)
    return function()
      if TrafficSpooler.IsPlayerVehicleLoaded() then
        removeUserUpdateFunction("spawnVehicleEx")
        spawn()
      end
    end
  end
  if TrafficSpooler.IsMissionVehicleLoaded(modelID) then
    return spawn()
  elseif not spooling.requestToSpoolVehicle(modelID, spawn) then
    addUserUpdateFunction("spawnVehicleEx", waitForSpooling(modelID), 1)
  end
end

local function safeRun()
  if localPlayer.currentVehicle then
    local status = controlHandler:getStatus("Zap_ActiveVehicle_Two")
    if status == "JustPressed" then
      local vehicle = localPlayer.currentVehicle
      
      local newVehicle = spawnVehicleEx(vehicle.model_id)
      
      OneShotSound.PlayMenuSound("HUD_Garage_Vehicle_Repair")
      
      localPlayer:setPreviousVehicle()
      localPlayer:zapToAgent(newVehicle, {playerInstigated = true, disableZapFlash = true})
    end
    local status = controlHandler:getStatus("Zap_ActiveVehicle_One")
    if status == "JustPressed" then
      spawnVehicleEx(269)
    end
  end
end

local function waitForSimulation()
  -- wait for the pause menu to go away
  if simulation.getSpeed() ~= 0 then
    removeUserUpdateFunction("safeRunWait")
    addUserUpdateFunction("safeRunDelay", safeRun, 1)
  end
end

addUserUpdateFunction("safeRunWait", waitForSimulation, 0.1 * updates.stepRate)