dofile(DNGHookScriptPath.. "driverNGUtil.lua")
dofile(DNGHookScriptPath.. "driverNGDocsUtil.lua")

-- this is called from the hook via CallLuaFunction
function driverNGHook_EvalHelper(luaExprStr)	

	print("> " .. luaExprStr)

	if luaExprStr:sub(1, 1) == "=" then
		luaExprStr = "util.printObject(" .. luaExprStr:sub(2) .. ")"
	end

	function evaluator()
		removeUserUpdateFunction("hookConsoleEvaluator")
		local f, err = loadstring(luaExprStr)
		if not f then
			print("ERROR: `" .. err .. "`")
		else
			local ran, errorMsg = pcall( f )
			
			if not ran then
				print("ERROR: " .. errorMsg)
				print(debug.traceback())
			end
		end
	end
	
	-- add to game thread to ensure safety
	addUserUpdateFunction("hookConsoleEvaluator", evaluator, 120)
end

-- called from hook
function driverNGHook_SwitchPause(enable)

	if driverNGHook.gameLaunched == false then
		return
	end
	
	if driverNGHook.gamePaused == enable then
		return
	end
	
	driverNGHook.gamePaused = enable
	
	-- game is not going to be paused in Online
	if Network.getConnection() == "Offline" then
		if enable then
			simulation.pause()
		else
			simulation.start()
		end
	end
end

-- called from hook
function driverNGHook_QuitGame()
	framework.quit()
end