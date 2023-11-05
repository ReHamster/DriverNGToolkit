
local luaDocs = {}

function LuaDocumentation_Initialise()
  print("$luadoc$:init")
  luaDocs = {}
end
function LuaDocumentation_Shutdown()
  print("$luadoc$:shutdown")
  addUserUpdateFunction("LuaDocs", function()
    local blacklist = {
      ["_M"] = true,
      ["_NAME"] = true,
      ["_PACKAGE"] = true,
    }

    print("**** sorting docs")
    local docs = {}
    for k,v in pairs(luaDocs) do
      if k ~= "_G" then
        table.insert(docs, k)
      end
    end
    table.sort(docs)

    if luaDocs["_G"] ~= nil then
      table.insert(docs, 1, "_G")
    end
  
    local docfile = io.open("temp:luadocs.txt", "w+")
    
    print("**** writing docs")
    for i,libraryName in ipairs(docs) do
      local library = rawget(luaDocs, libraryName)
      local t = "table"
      if library._M and library._NAME and library._PACKAGE then
        t = "module"
      end
      docfile:write(string.format("[%s] : %s\n", libraryName, t))
      local sorted = {}
      for name,_ in pairs(library) do
        if not blacklist[name] then
          table.insert(sorted, name)
        end
      end
      table.sort(sorted)
      local typeMap = {
        "CFunction",
        "function",
        "property",
        "userdata",
        "table",
        "boolean",
        "number",
        "string",
        "nil",
      }
      local typeSorted = {}
      for _,t in ipairs(typeMap) do
        typeSorted[t] = {}
      end
      for _,name in ipairs(sorted) do
        local info = library[name]
        local m = ""
        local t = type(name)
        if t == "string" then
          if name:find(" ") ~= nil then
            m = string.format("[\"%s\"]", name)
          else
            m = name
          end
        else
          m = string.format("[%s]", tostring(name))
        end
        if string.len(info.docs) > 0 then
          m = m .." : ".. info.docs
        end
        table.insert(typeSorted[info.type], m)
      end
      for _,t in ipairs(typeMap) do
        local v = typeSorted[t]
        for _,m in ipairs(v) do
          local l = string.format("\t%s %s\n", t, m)
          docfile:write(l)
        end
      end
    end
    docfile:close()
    
    removeUserUpdateFunction("LuaDocs")
  end, 0.01 * updates.stepRate, true)
end
function LuaDocumentation_AddLuaLibrary(libraryName)
  if luaDocs[libraryName] == nil then
    luaDocs[libraryName] = {}
  end
end
function LuaDocumentation_AddLuaDocumentation(libraryName, docType, docName, docText)
  if luaDocs[libraryName] == nil then
    luaDocs[libraryName] = {}
  end
  if luaDocs[libraryName][docName] == nil then
    luaDocs[libraryName][docName] = { type = docType, docs = docText or "" }
  end
end
function LuaDocumentation_AddLuaDataDocumentation(libraryName, value, name, docs)
  local printTypes = {
    ["string"] = true,
    ["number"] = true,
    ["boolean"] = true,
  }
  local dataType = type(value)
  if printTypes[dataType] then
    local v = tostring(value)
    if dataType == "string" then
      v = '"'..v..'"'
    end
    if string.len(docs) > 0 then
      docs = v .." : ".. docs 
    else
      docs = v
    end
  end
  LuaDocumentation_AddLuaDocumentation(libraryName, dataType, name, docs)
end
function LuaDocumentation_AddLuaFunctionDocumentation(libraryName, functionName, functionDocs)
  LuaDocumentation_AddLuaDocumentation(libraryName, "function", functionName, functionDocs)
end
function LuaDocumentation_AddLuaPropertyDocumentation(libraryName, propertyName, propertyDocs)
  LuaDocumentation_AddLuaDocumentation(libraryName, "property", propertyName, propertyDocs)
end
function LuaDocumentation_RemoveLuaLibrary(libraryName)
  if luaDocs[libraryName] then
    table.remove(luaDocs, libraryName)
  end
end
function LuaDocumentation_ParseTable(name, values)
  local function parseTable(name, key, value)
    if key ~= name then
      LuaDocumentation_AddLuaDataDocumentation(name, value, key, "")
    end
  end
  local function parseGlobals(name, key, value)
    if type(value) == "table" then
      if key ~= "_G" then
        LuaDocumentation_ParseTable(key, value)
      end
    else
      LuaDocumentation_AddLuaDataDocumentation("_G", value, key, "")
    end
  end
  
  local parseIt = parseGlobals
  if string.len(name) > 0 then
    parseIt = parseTable
  end
  
  for key, value in next, values do
    parseIt(name, key, value)
  end
end