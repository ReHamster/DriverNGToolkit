#include <lauxlib.h>
#include <ldo.h>
#include <lgc.h>
#include <lvm.h>

//------------------------------------
// Lua function shims that redirecting 
// to the Driver.exe native code
// this is only needed for LuaDelegate
//------------------------------------

static uintptr_t kluaCStepAddr = 0x005F2C30;
static uintptr_t kluaGetTopAddr = 0x005E9300;
static uintptr_t kluaSetTopAddr = 0x005E9310;
static uintptr_t kluaPCallAddr = 0x005F1CA0;		// riLuaCall but it's same as lua_pcall'
static uintptr_t kluaLRefAddr = 0x005EF810;
static uintptr_t kluaLRegisterAddr = 0x00602C60;
static uintptr_t kluaLCheckIntegerAddr = 0x005F6BE0;
static uintptr_t kluaLCheckNumberAddr = 0x005F6A90;
static uintptr_t kluaLCheckUdataAddr = 0x005F6870;
static uintptr_t kluaLTypeErrorAddr = 0x005F67C0;
static uintptr_t kluaLArgErrorAddr = 0x005F66E0;
static uintptr_t kluaLErrorAddr = 0x005F30E0;
static uintptr_t kluaLCheckOptionAddr = 0x006016D0;
static uintptr_t kluaLNewMetaTableAddr = 0x005F3150;
static uintptr_t kluaNewUserdataAddr = 0x005F6680;
static uintptr_t kluaSetMetaTableAddr = 0x005EDA90;
static uintptr_t kluaTTypeNamesAddr = 0x00DE71F0;
static uintptr_t kluaGetStackAddr = 0x005EA080;
static uintptr_t kluaGetInfoAddr = 0x005EFC40;
static uintptr_t kluaDProtectedParserAddr = 0x00607730;
static uintptr_t kluaPushCClosureAddr = 0x00601540;
static uintptr_t kluaSetFieldAddr = 0x005EF6D0;
static uintptr_t kluaGetFieldAddr = 0x005EF640;
static uintptr_t kluaOPushVFStringAddr = 0x005F1E40;
static uintptr_t kluaPushStringAddr = 0x005F2DD0;
static uintptr_t kluaPushFStringAddr = 0x005F2E70;
static uintptr_t kluaPushLStringAddr = 0x005F2D90;
static uintptr_t kluaPushBooleanAddr = 0x005E9660;
static uintptr_t kluaPushNumberAddr = 0x005E9620;
static uintptr_t kluaPushValueAddr = 0x005E93E0;
static uintptr_t kluaTypeAddr = 0x005E9410;
static uintptr_t kluaIndex2adrAddr = 0x005E91C0;	// for some reason it crashes game
static uintptr_t kluaToLStringAddr = 0x005F2D20;
static uintptr_t kluaVToNumberAddr = 0x005ED760;
static uintptr_t kluaToBooleanAddr = 0x005E94E0;
static uintptr_t kluaCreateTableAddr = 0x005F2EA0;
static uintptr_t kluaRawSetAddr = 0x005EDA20;

const TValue luaO_nilobject_ = { {NULL}, LUA_TNIL };

static TValue* index2adr(lua_State* L, int idx) {
	if (idx > 0) {
		TValue* o = L->base + (idx - 1);
		api_check(L, idx <= L->ci->top - L->base);
		if (o >= L->top) return cast(TValue*, luaO_nilobject);
		else return o;
	}
	else if (idx > LUA_REGISTRYINDEX) {
		api_check(L, idx != 0 && -idx <= L->top - L->base);
		return L->top + idx;
	}
	else switch (idx) {  /* pseudo-indices */
	case LUA_REGISTRYINDEX: return registry(L);
	case LUA_ENVIRONINDEX: {
		Closure* func = curr_func(L);
		sethvalue(L, &L->env, func->c.env);
		return &L->env;
	}
	case LUA_GLOBALSINDEX: return gt(L);
	default: {
		Closure* func = curr_func(L);
		idx = LUA_GLOBALSINDEX - idx;
		return (idx <= func->c.nupvalues)
			? &func->c.upvalue[idx - 1]
			: cast(TValue*, luaO_nilobject);
	}
	}
}

//static TValue* index2adr(lua_State* L, int idx)
//{
//	typedef TValue* (*index2adr_t)(lua_State*, int);
//	return ((index2adr_t)kluaIndex2adrAddr)(L, idx);
//}

LUA_API void luaC_step(lua_State* L)
{
	typedef int (*luaC_step_t)(lua_State*);
	((luaC_step_t)kluaCStepAddr)(L);
}

LUA_API int lua_gettop(lua_State* L)
{
	typedef int (*lua_gettop_t)(lua_State*);
	return ((lua_gettop_t)kluaGetTopAddr)(L);
}

LUA_API void lua_settop(lua_State* L, int idx)
{
	typedef void (*lua_settop_t)(lua_State*, int idx);
	((lua_settop_t)kluaSetTopAddr)(L, idx);
}

typedef TValue* StkId;

LUA_API void lua_call(lua_State* L, int nargs, int nresults)
{
	// we don't have lua_call and I'm lazy to reimplement it
	// let pcall do it's thing without error func
	typedef int(* lua_pcall_t)(lua_State*, int, int, int);
	((lua_pcall_t)kluaPCallAddr)(L, nargs, nresults, 0);
}

LUA_API int lua_pcall(lua_State* L, int nargs, int nresults, int errfunc)
{
	typedef int(* lua_pcall_t)(lua_State*, int, int, int);
	return ((lua_pcall_t)kluaPCallAddr)(L, nargs, nresults, errfunc);
}

LUA_API int lua_getstack(lua_State* L, int level, lua_Debug* ar)
{
	typedef int(* lua_getstack_t)(lua_State*, int, lua_Debug*);
	return ((lua_getstack_t)kluaGetStackAddr)(L, level, ar);
}

LUA_API int lua_getinfo(lua_State* L, const char* what, lua_Debug* ar)
{
	typedef int(* lua_getinfo_t)(lua_State*, const char*, lua_Debug*);
	return ((lua_getinfo_t)kluaGetInfoAddr)(L, what, ar);
}

typedef struct Lua_LoadS
{
	const char* s;
	size_t size;
} Lua_LoadS;

// direct reimpl
static const char* lua_load_getS(lua_State* L, void* ud, size_t* size)
{
	Lua_LoadS* ls = (Lua_LoadS*)ud;
	if (ls->size == 0)
		return NULL;
	*size = ls->size;
	ls->size = 0;
	return ls->s;
}

// direct reimpl
LUA_API void luaZ_init(lua_State* L, ZIO* z, lua_Reader reader, void* data)
{
	z->L = L;
	z->reader = reader;
	z->data = data;
	z->n = 0;
	z->p = NULL;
}

// almost direct reimpl
LUA_API int lua_load(lua_State* L, lua_Reader reader, void* data, const char* chunkname)
{
	typedef int(* luaD_protectedparser_t) (lua_State* L, ZIO* z, const char* name);

	ZIO z;
	int status;
	lua_lock(L);
	if (!chunkname) chunkname = "?";
	luaZ_init(L, &z, reader, data);
	status = ((luaD_protectedparser_t)kluaDProtectedParserAddr)(L, &z, chunkname);
	lua_unlock(L);
	return status;
}

// direct reimpl
LUALIB_API int luaL_loadbuffer(lua_State* L, const char* buff, size_t size, const char* name)
{
	Lua_LoadS ls;
	ls.s = buff;
	ls.size = size;
	return lua_load(L, lua_load_getS, &ls, name);
}

LUALIB_API int luaL_ref(lua_State* L, int t)
{
	typedef int(* luaL_ref_t)(lua_State*, int);
	return ((luaL_ref_t)kluaLRefAddr)(L, t);
}

LUALIB_API void luaL_register(lua_State* L, const char* libname, const luaL_Reg* l)
{
	typedef void(* luaL_register_t)(lua_State*, const char*, const luaL_Reg*);
	((luaL_register_t)kluaLRegisterAddr)(L, libname, l);
}

LUALIB_API lua_Integer luaL_checkinteger(lua_State* L, int numArg)
{
	typedef lua_Integer(* luaL_checkinteger_t)(lua_State*, int);
	return ((luaL_checkinteger_t)kluaLCheckIntegerAddr)(L, numArg);
}

LUALIB_API lua_Number luaL_checknumber(lua_State* L, int numArg)
{
	typedef lua_Number(* luaL_checknumber_t)(lua_State*, int);
	return ((luaL_checknumber_t)kluaLCheckNumberAddr)(L, numArg);
}

// direct reimpl
LUALIB_API lua_Number luaL_optnumber(lua_State* L, int narg, lua_Number def)
{
	return luaL_opt(L, luaL_checknumber, narg, def);
}

// direct reimpl
LUALIB_API lua_Integer luaL_optinteger(lua_State* L, int narg, lua_Integer def) 
{
	return luaL_opt(L, luaL_checkinteger, narg, def);
}

LUALIB_API int luaL_typerror(lua_State* L, int narg, const char* tname)
{
	typedef int(* luaL_typerror_t)(lua_State*, int, const char*);
	return ((luaL_typerror_t)kluaLTypeErrorAddr)(L, narg, tname);
}

LUALIB_API int luaL_argerror(lua_State* L, int narg, const char* extramsg)
{
	typedef int (*luaL_argerror_t)(lua_State* L, int, const char*);
	return ((luaL_argerror_t)kluaLArgErrorAddr)(L, narg, extramsg);
}


LUALIB_API int luaL_error(lua_State* L, const char* fmt, ...)
{
	typedef int (*luaL_error_t)(lua_State* L, const char* fmt, ...);
	luaL_error_t originalFunction = (luaL_error_t)kluaLErrorAddr;

	int result;
	va_list args;

	// Start processing variadic arguments
	va_start(args, fmt);
	__asm {
		// Save current stack frame
		push ebp
		mov ebp, esp

		// Prepare to call the original function
		mov eax, fmt             // Move fmt into eax (second parameter)
		push eax                 // Push fmt onto stack
		mov eax, L               // Move L into eax (first parameter)
		push eax                 // Push L onto stack

		// Forward variadic arguments
		lea eax, args            // Load address of args
		push eax                 // Push address of va_list onto stack

		// Call the original function
		mov eax, originalFunction
		call eax

		// Store the result
		mov result, eax

		// Restore the stack
		mov esp, ebp
		pop ebp
	}

	return result;
}

LUALIB_API int luaL_checkoption(lua_State* L, int narg, const char* def, const char* const lst[])
{
	typedef int (*luaL_checkoption_t)(lua_State*, int, const char*, const char* const[]);
	return ((luaL_checkoption_t)kluaLCheckOptionAddr)(L, narg, def, lst);
}

LUALIB_API int luaL_newmetatable(lua_State* L, const char* tname)
{
	typedef int (*luaL_newmetatable_t)(lua_State* L, const char*);
	return ((luaL_newmetatable_t)kluaLNewMetaTableAddr)(L, tname);
}

LUA_API void* lua_newuserdata(lua_State* L, size_t sz)
{
	typedef void* (*lua_newuserdata_t)(lua_State*, size_t sz);
	return ((lua_newuserdata_t)kluaNewUserdataAddr)(L, sz);
}

LUA_API int lua_setmetatable(lua_State* L, int idx)
{
	typedef int (*lua_setmetatable_t)(lua_State* L, int);
	return ((lua_setmetatable_t)kluaSetMetaTableAddr)(L, idx);
}

// direct reimpl
LUA_API const char* lua_typename(lua_State* L, int tp)
{
	const char* const* luaT_typenames_ptr = (const char* const*)kluaTTypeNamesAddr;
	return (tp == LUA_TNONE) ? "no value" : luaT_typenames_ptr[tp];
}

// direct reimpl
static void lua_tag_error(lua_State* L, int narg, int tag)
{
	luaL_typerror(L, narg, lua_typename(L, tag));
}

// direct reimpl
LUA_API const char* luaL_checklstring(lua_State* L, int narg, size_t* len)
{
	const char* s = lua_tolstring(L, narg, len);
	if (!s) lua_tag_error(L, narg, LUA_TSTRING);
	return s;
}

LUALIB_API void* luaL_checkudata(lua_State* L, int ud, const char* tname)
{
	typedef void* (*luaL_checkudata_t)(lua_State*, int, const char*);
	return ((luaL_checkudata_t)kluaLCheckUdataAddr)(L, ud, tname);
}

LUA_API void lua_pushcclosure(lua_State* L, lua_CFunction fn, int n)
{
	typedef void(* lua_pushcclosure_t)(lua_State*, lua_CFunction, int);
	((lua_pushcclosure_t)kluaPushCClosureAddr)(L, fn, n);
}

LUA_API void lua_setfield(lua_State* L, int idx, const char* k)
{
	typedef void(* lua_setfield_t)(lua_State*, int, const char*);
	((lua_setfield_t)kluaSetFieldAddr)(L, idx, k);
}

LUA_API void lua_getfield(lua_State* L, int idx, const char* k)
{
	typedef void(* lua_getfield_t)(lua_State*, int, const char*);
	((lua_getfield_t)kluaGetFieldAddr)(L, idx, k);
}

#define api_incr_top(L)   {api_check(L, L->top < L->ci->top); L->top++;}

LUA_API void lua_pushstring(lua_State* L, const char* s)
{
	typedef void(* lua_pushstring_t)(lua_State*, const char*);
	((lua_pushstring_t)kluaPushStringAddr)(L, s);
}

// almost direct reimpl
LUA_API const char* lua_pushfstring(lua_State* L, const char* fmt, ...)
{
	typedef const char* (*luaO_pushvfstring_t)(lua_State* L, const char* fmt, va_list argp);

	const char* ret;
	va_list argp;
	lua_lock(L);
	luaC_checkGC(L);
	va_start(argp, fmt);
	ret = ((luaO_pushvfstring_t)kluaOPushVFStringAddr)(L, fmt, argp);
	va_end(argp);
	lua_unlock(L);
	return ret;
}

LUA_API void lua_pushlstring(lua_State* L, const char* s, size_t l)
{
	typedef void(__cdecl* lua_pushlstring_t)(lua_State*, const char*, size_t);
	((lua_pushlstring_t)kluaPushLStringAddr)(L, s, l);
}

LUA_API void lua_pushboolean(lua_State* L, int b)
{
	typedef void(__cdecl* lua_pushboolean_t)(lua_State*, int);
	((lua_pushboolean_t)kluaPushBooleanAddr)(L, b);
}

LUA_API void lua_pushnumber(lua_State* L, lua_Number n)
{
	typedef void(__cdecl* lua_pushnumber_t)(lua_State*, lua_Number n);
	((lua_pushnumber_t)kluaPushNumberAddr)(L, n);
}

LUA_API void lua_pushinteger(lua_State* L, lua_Integer n)
{
	// totaly safe on 5.1 since LUAT_NUMBER is used internally
	lua_pushnumber(L, (lua_Number)n);
}

LUA_API void lua_pushvalue(lua_State* L, int idx)
{
	typedef void(__cdecl* lua_pushvalue_t)(lua_State*, int);
	((lua_pushvalue_t)kluaPushValueAddr)(L, idx);
}

// direct reimpl, hopefully safe
LUA_API void lua_pushnil(lua_State* L)
{
	lua_lock(L);
	setnilvalue(L->top);
	api_incr_top(L);
	lua_unlock(L);
}

LUA_API int lua_type(lua_State* L, int idx)
{
	typedef int(__cdecl* lua_type_t)(lua_State*, int);
	return ((lua_type_t)kluaTypeAddr)(L, idx);
}

LUA_API int lua_isstring(lua_State* L, int idx)
{
	int t = lua_type(L, idx);
	return (t == LUA_TSTRING || t == LUA_TNUMBER);
}

LUA_API const char* lua_tolstring(lua_State* L, int idx, size_t* len)
{
	typedef const char* (__cdecl* lua_tolstring_t)(lua_State*, int, size_t*);
	return ((lua_tolstring_t)kluaToLStringAddr)(L, idx, len);
}

LUA_API const TValue* luaV_tonumber(const TValue* obj, TValue* n)
{
	typedef const TValue* (*luaV_tonumber_t)(const TValue*, TValue*);
	return ((luaV_tonumber_t)kluaVToNumberAddr)(obj, n);
}

// direct reimpl
LUA_API void* lua_touserdata(lua_State* L, int idx)
{
	StkId o = index2adr(L, idx);
	switch (ttype(o)) {
	case LUA_TUSERDATA: return (rawuvalue(o) + 1);
	case LUA_TLIGHTUSERDATA: return pvalue(o);
	default: return NULL;
	}
}

// direct reimpl
LUA_API lua_Number lua_tonumber(lua_State* L, int idx)
{
	TValue n;
	const TValue* o = index2adr(L, idx);
	if (tonumber(o, &n))
		return nvalue(o);
	else
		return 0;
}

// direct reimpl
LUA_API int lua_toboolean(lua_State* L, int idx)
{
	const TValue* o = index2adr(L, idx);
	return !l_isfalse(o);
}

LUA_API void lua_createtable(lua_State* L, int narr, int nrec)
{
	typedef int(__cdecl* lua_createtable_t)(lua_State*, int, int);
	((lua_createtable_t)kluaCreateTableAddr)(L, narr, nrec);
}

LUA_API void lua_rawset(lua_State* L, int idx)
{
	typedef int(__cdecl* lua_rawset_t)(lua_State*, int);
	((lua_rawset_t)kluaRawSetAddr)(L, idx);
}
