-------------------------------------------------------------
--	script utility functions
-------------------------------------------------------------

util = {}

-- splits the string
function string:split( inSplitPattern, outResults )
	if not outResults then
		outResults = { }
	end
	local theStart = 1
	local theSplitStart, theSplitEnd = string.find( self, inSplitPattern, theStart )
	while theSplitStart do
		table.insert( outResults, string.sub( self, theStart, theSplitStart-1 ) )
		theStart = theSplitEnd + 1
		theSplitStart, theSplitEnd = string.find( self, inSplitPattern, theStart )
	end
	table.insert( outResults, string.sub( self, theStart ) )
	return outResults
end

-- checks if string start matches substring
function string.starts(String,Start)
   return string.sub(String,1,string.len(Start))==Start
end

-- checks if string ending matches substring
function string.ends(String,End)
   return End=='' or string.sub(String,-string.len(End))==End
end

-- returns function arguments info
function util.funcArgs(fun)
	local args = {}
	local hook = debug.gethook()

	local argHook = function( ... )
		local info = debug.getinfo(3)
		if 'pcall' ~= info.name then return end

		for i = 1, math.huge do
			local name, value = debug.getlocal(2, i)
			if '(*temporary)' == name then
				debug.sethook(hook)
				error('')
				return
			end
			table.insert(args,name)
		end
	end

	debug.sethook(argHook, "c")
	pcall(fun)

	return args
end

-- prints object to the console output
function util.printObject( tt )

	local splr = ""
	if type(tt) == "userdata" then
		local meta = getmetatable(tt)
		splr = table.inspect(meta)
	else
		splr = table.inspect(tt)
	end

	local strings = splr:split('\n')
	
	for k,v in ipairs(strings) do
		print(v)
	end
end

-- inserts unique item into table
function table.insertUnique(tt, elem)
	for k,v in pairs(tt) do
		if v == elem then
			return false
		end
	end
	
	-- nothing found
	table.insert(tt,elem)
	
	return true
end

-- checks if table is array
function table.isArray(tbl)
	local max, n, arraylen = 0, 0, 0
	for k,v in pairs (tbl) do
		if k == 'n' and type(v) == 'number' then
			arraylen = v
			if v > max then
				max = v
			end
		else
			if type(k) ~= 'number' or k < 1 or math.floor(k) ~= k then
				return false
			end
			if k > max then
				max = k
			end
			n = n + 1
		end
	end
	if max > 10 and max > arraylen and max > n * 2 then
		return false -- don't create an array with too many holes
	end

	return true, max
end

-- returns table keys sorted by sortFunction
function table.sortedKeys(tbl, sortFunction)
	local keys = {}
	for key in pairs(tbl) do
		table.insert(keys, key)
	end

	table.sort(keys, function(a, b)
		return sortFunction(tbl[a], tbl[b])
	end)

	return keys
end

-- Print anything - including nested tables
-- returns string
function table.inspect(tt, indent, done, inspector)
	done = done or {}
	
	if inspector == nil then
		inspector = {}
		inspector.buffer = ""
		inspector.Concat = function( self, v )
			self.buffer = self.buffer..v
		end
	end

	indent = indent or 0
	if type(tt) == "table" then
		for key, value in pairs (tt) do
			inspector:Concat(string.rep (" ", indent)) -- indent it
			if type(value) == "table" and not done [value] then
				done [value] = true
				inspector:Concat(string.format("[%s] => table\n", tostring(key)))
				inspector:Concat(string.rep(" ", indent+4)) -- indent it
				inspector:Concat("(\n");
				table.inspect(value, indent + 7, done, inspector)
				inspector:Concat(string.rep(" ", indent+4)) -- indent it
				inspector:Concat(")\n");
			else
				inspector:Concat(string.format("[%s] => (%s) %s\n",tostring (key), type(value), tostring(value)))
			end
		end
	elseif type(tt) == "function" then
		inspector:Concat(string.format("[%s] => (%s) %s\n",tostring (tt), type(tt), tostring(tt)))
	else
		inspector:Concat(tostring(tt) .."\n")
	end

	return inspector.buffer
end

function table.combine(...) -- object.assign like from JS
    local combinedTable = {}
    local arg = {...}

    for k, v in pairs(arg) do
        if type(v) == 'table' then
			for tk, tv in pairs(v) do
				combinedTable[tk] = tv
                --table.insert(combinedTable, tv)
            end
        end
    end

    return combinedTable
end

table.assign = table.combine

function table.join(t1,t2)
    for i=1,#t2 do
        t1[#t1+i] = t2[i]
    end
    return t1
end