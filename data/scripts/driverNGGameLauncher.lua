-- Override some functions
GameLauncher.GameState = "Game Started"
GameLauncher.StateChangeCallback = {}

function GameLauncher.SetStateChangeCallback(name, func)
	GameLauncher.StateChangeCallback[name] = func
end

function GameLauncher.GetGameState()
	print("$GAMELAUNCHER$<GAMESTATE>" .. GameLauncher.GameState)
end

function GameLauncher.SetGameState(strState)
	GameLauncher.GameState = strState
	print("$GAMELAUNCHER$<GAMESTATE>" .. GameLauncher.GameState)

	-- FIXME: replace this with something more hidden and reliable
	if Network.getConnection() == "Offline" then
	
		local callback = GameLauncher.StateChangeCallback[strState]
		if callback ~= nil then
			callback()
		end
	end
end
