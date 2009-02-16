
#ifndef _BASETYPES_H_
#define _BASETYPES_H_

//system defines
#ifdef __LINUX__
#include <gtk/gtk.h>
#else
#	define WINVER 0x0501
#	define _WIN32_WINNT 0x0501
#	include <windows.h>
#	include <mmsystem.h>
#	include <tchar.h>
#	include "resource.h"
#endif

#include <assert.h>

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <ctime>
#include <stdexcept>
#include <string>

using std::string;
using std::wstring;

//////////////////////////////////////////////////////////////////////////
// Override Win32 min/max macros with the STL's type safe and macro
// free varieties (much safer!)

#undef min
#undef max

#include <algorithm>

using std::min;
using std::max;

template< typename T >
static __forceinline void Clampify( T& src, T min, T max )
{
	src = std::min( std::max( src, min ), max );
}

template< typename T >
static __forceinline T GetClamped( T src, T min, T max )
{
	return std::min( std::max( src, min ), max );
}

extern void SysMessage(const char *fmt, ...);

//////////////////////////////////////////////////////////////
// Dev / Debug conditionals --
//   Consts for using if() statements instead of uglier #ifdef macros.
//   Abbreviated macros for dev/debug only consoles and msgboxes.

#ifdef SPU2X_DEVBUILD

#	define DevCon Console
#	define DevMsg MsgBox
	static const bool IsDevBuild = true;

#else

#	define DevCon 0&&Console
#	define DevMsg 
	static const bool IsDevBuild = false;

#endif

#ifdef _DEBUG

#	define DbgCon Console
	static const bool IsDebugBuild = true;

#else

#	define DbgCon 0&&Console
static const bool IsDebugBuild = false;

#endif

#endif
