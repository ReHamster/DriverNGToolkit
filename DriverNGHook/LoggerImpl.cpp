#include "Logger.h"
#include "LoggerImpl.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <Windows.h>

#pragma warning(disable:4996)

std::wstring GetExePath() 
{
	wchar_t buffer[260] = { 0 };
	GetModuleFileNameW(NULL, buffer, 260);
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	return std::wstring(buffer).substr(0, pos);
}

std::wstring g_logFilename;

bool g_logToFile = true;

// Default spew
void DefaultSpewFunc(SpewType_t type,const char* pMsg)
{
	puts( pMsg );
}

//Spew callback
static SpewFunc_fn g_fnConSpewFunc = DefaultSpewFunc;

void SetSpewFunction(SpewFunc_fn newfunc)
{
	g_fnConSpewFunc = newfunc;
}

void SpewMessageToOutput(SpewType_t spewtype,char const* pMsgFormat, va_list args)
{
	char staticBuf[2048];
	char* buff = staticBuf;

	/* Create the message.... */
	int len = vsnprintf( staticBuf, sizeof(staticBuf), pMsgFormat, args );

	if (len > sizeof(staticBuf))
	{
		buff = new char[len+1];
		len = vsnprintf(buff, len+1, pMsgFormat, args);
	}

	if (g_logToFile && g_logFilename.length() > 0)
	{
		FILE* g_logFile = _wfopen(g_logFilename.c_str(), L"a");

		if (g_logFile)
		{
			fputs(buff, g_logFile);
			fclose(g_logFile);
		}
	}

	(g_fnConSpewFunc)(spewtype, buff);

	if(staticBuf != buff)
		delete[] buff;
}

// developer message output
void DevMsg(SpewType_t type, const char* fmt, ...)
{
#ifdef _DEBUG
	va_list		argptr;

	va_start(argptr, fmt);
	SpewMessageToOutput(type, fmt, argptr);
	va_end(argptr);
#endif
}

// Simple messages
void Msg(const char *fmt,...)
{
	va_list		argptr;

	va_start (argptr,fmt);
	SpewMessageToOutput(SPEW_NORM,fmt,argptr);
	va_end (argptr);
}

// Error messages
void MsgError(const char *fmt,...)
{
	va_list		argptr;

	va_start (argptr,fmt);
	SpewMessageToOutput(SPEW_ERROR,fmt,argptr);
	va_end (argptr);
}

// Info messages
void MsgInfo(const char *fmt,...)
{
	va_list		argptr;

	va_start (argptr,fmt);
	SpewMessageToOutput(SPEW_INFO,fmt,argptr);
	va_end (argptr);
}

//Warning messages
void MsgWarning(const char *fmt,...)
{
	va_list		argptr;

	va_start (argptr,fmt);
	SpewMessageToOutput(SPEW_WARNING,fmt,argptr);
	va_end (argptr);
}

// Good messages
void MsgAccept(const char *fmt,...)
{
	va_list		argptr;

	va_start (argptr,fmt);
	SpewMessageToOutput(SPEW_SUCCESS,fmt,argptr);
	va_end (argptr);
}

#ifdef _WIN32
#include <conio.h>
#include "Windows.h"

static unsigned short g_InitialColor = 0xFFFF;
static unsigned short g_LastColor = 0xFFFF;
static unsigned short g_BadColor = 0xFFFF;
static WORD g_BackgroundFlags = 0xFFFF;

static void GetInitialColors( )
{
	// Get the old background attributes.
	CONSOLE_SCREEN_BUFFER_INFO oldInfo;
	GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &oldInfo );
	g_InitialColor = g_LastColor = oldInfo.wAttributes & (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	g_BackgroundFlags = oldInfo.wAttributes & (BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_INTENSITY);

	g_BadColor = 0;
	if (g_BackgroundFlags & BACKGROUND_RED)
		g_BadColor |= FOREGROUND_RED;
	if (g_BackgroundFlags & BACKGROUND_GREEN)
		g_BadColor |= FOREGROUND_GREEN;
	if (g_BackgroundFlags & BACKGROUND_BLUE)
		g_BadColor |= FOREGROUND_BLUE;
	if (g_BackgroundFlags & BACKGROUND_INTENSITY)
		g_BadColor |= FOREGROUND_INTENSITY;
}

static WORD SetConsoleTextColor( int red, int green, int blue, int intensity )
{
	WORD ret = g_LastColor;

	g_LastColor = 0;
	if( red )	g_LastColor |= FOREGROUND_RED;
	if( green ) g_LastColor |= FOREGROUND_GREEN;
	if( blue )  g_LastColor |= FOREGROUND_BLUE;
	if( intensity ) g_LastColor |= FOREGROUND_INTENSITY;

	// Just use the initial color if there's a match...
	if (g_LastColor == g_BadColor)
		g_LastColor = g_InitialColor;

	SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), g_LastColor | g_BackgroundFlags );
	return ret;
}

static void RestoreConsoleTextColor( WORD color )
{
	SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), color | g_BackgroundFlags );
	g_LastColor = color;
}

CRITICAL_SECTION g_SpewCS;
bool g_bSpewCSInitted = false;

void fnConDebugSpew(SpewType_t type,const char* text)
{
	// Hopefully two threads won't call this simultaneously right at the start!
	if ( !g_bSpewCSInitted )
	{
		InitializeCriticalSection( &g_SpewCS );
		g_bSpewCSInitted = true;
	}

	WORD old;
	EnterCriticalSection( &g_SpewCS );
	{
		if( type == SPEW_NORM )
		{
			old = SetConsoleTextColor( 1, 1, 1, 0 );
		}
		else if( type == SPEW_WARNING )
		{
			old = SetConsoleTextColor( 1, 1, 0, 1 );
		}
		else if( type == SPEW_SUCCESS )
		{
			old = SetConsoleTextColor( 0, 1, 0, 1 );
		}
		else if( type == SPEW_ERROR )
		{
			old = SetConsoleTextColor( 1, 0, 0, 1 );
		}
		else if( type == SPEW_INFO )
		{
			old = SetConsoleTextColor( 1, 1, 1, 0 );
		}
		else
		{
			old = SetConsoleTextColor( 1, 1, 1, 1 );
		}

		OutputDebugStringA( text );
		printf( "%s", text );

		RestoreConsoleTextColor( old );
	}
	LeaveCriticalSection( &g_SpewCS );
}

void Install_ConsoleSpewFunction()
{
	g_logFilename = GetExePath() + L"\\Driver.log";
	DeleteFileW(g_logFilename.c_str());

	SetSpewFunction(fnConDebugSpew);
	GetInitialColors();
}

void Remove_ConsoleSpewFunction()
{
	SetSpewFunction(DefaultSpewFunc);
	GetInitialColors();
}

#endif
