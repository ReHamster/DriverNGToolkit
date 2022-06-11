#pragma once

typedef enum 
{
	SPEW_NORM,
	SPEW_INFO,
	SPEW_WARNING,
	SPEW_ERROR,
	SPEW_SUCCESS,
}SpewType_t;

typedef void (*SpewFunc_fn)(SpewType_t,const char*);

void SetSpewFunction(SpewFunc_fn newfunc);

void Install_ConsoleSpewFunction();
void Remove_ConsoleSpewFunction();

extern bool g_logToFile;