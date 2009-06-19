// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#pragma warning(disable: 4996 4995 4324 4100 4101 4201)

#ifdef _WINDOWS

// The following macros define the minimum required platform.  The minimum required platform
// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run 
// your application.  The macros work by enabling all features available on platform versions up to and 
// including the version specified.

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.

#ifndef WINVER                          // Specifies that the minimum required platform is Windows Vista.
#define WINVER 0x0600           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE                       // Specifies that the minimum required platform is Internet Explorer 7.0.
#define _WIN32_IE 0x0700        // Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <atlbase.h>

#endif

// stdc

#include <math.h>
#include <time.h>
#include <intrin.h>

#include <string>
#include <vector>
#include <list>
#include <map>
#include <hash_map>

using namespace std;
using namespace stdext;

extern string format(const char* fmt, ...);

// syntactic sugar

// put these into vc9/common7/ide/usertype.dat to have them highlighted

typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint32;
typedef signed int int32;
typedef unsigned long long uint64;
typedef signed long long int64;

#define countof(a) (sizeof(a) / sizeof(a[0]))

#define EXPORT_C extern "C" __declspec(dllexport) void __stdcall
#define EXPORT_C_(type) extern "C" __declspec(dllexport) type __stdcall

#define ALIGN_STACK(n) __declspec(align(n)) int __dummy;

#ifndef RESTRICT
	#ifdef __INTEL_COMPILER
		#define RESTRICT restrict
	#elif _MSC_VER >= 1400 // TODO: gcc
		#define RESTRICT __restrict
	#else
		#define RESTRICT
	#endif
#endif

#if defined(_DEBUG) && defined(_MSC_VER)
	#include <assert.h>
	#define ASSERT assert
#else
	#define ASSERT(exp) ((void)0)
#endif

#ifdef __x86_64__
	#define _M_AMD64
#endif

#ifdef _WINDOWS

// directx

#include <ddraw.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3d10.h>
#include <d3dx10.h>
#include <d3d9.h>
#include <d3dx9.h>

#define D3DCOLORWRITEENABLE_RGBA (D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA)

// dxsdk beta missing these:
#define D3D11_SHADER_MACRO D3D10_SHADER_MACRO
#define ID3D11Blob ID3D10Blob

#endif 

// opengl

#include <GL/glew.h>
#include <GL/glut.h>
#include <CG/cg.h>
#include <CG/cgGL.h>

#ifdef _WINDOWS
#include <GL/wglew.h>
#endif

// sse

#if !defined(_M_SSE) && (defined(_M_AMD64) || defined(_M_IX86_FP) && _M_IX86_FP >= 2)

	#define _M_SSE 0x200

#endif

#if _M_SSE >= 0x200

	#include <xmmintrin.h>
	#include <emmintrin.h>

	#ifndef _MM_DENORMALS_ARE_ZERO
	#define _MM_DENORMALS_ARE_ZERO 0x0040
	#endif

	#define MXCSR (_MM_DENORMALS_ARE_ZERO | _MM_MASK_MASK | _MM_ROUND_NEAREST | _MM_FLUSH_ZERO_ON)

	#if _MSC_VER < 1500

	__forceinline __m128i _mm_castps_si128(__m128 a) {return *(__m128i*)&a;}
	__forceinline __m128 _mm_castsi128_ps(__m128i a) {return *(__m128*)&a;}
	__forceinline __m128i _mm_castpd_si128(__m128d a) {return *(__m128i*)&a;}
	__forceinline __m128d _mm_castsi128_pd(__m128i a) {return *(__m128d*)&a;}
	__forceinline __m128d _mm_castps_pd(__m128 a) {return *(__m128d*)&a;}
	__forceinline __m128 _mm_castpd_ps(__m128d a) {return *(__m128*)&a;}

	#endif

	#define _MM_TRANSPOSE4_SI128(row0, row1, row2, row3) \
	{ \
		__m128 tmp0 = _mm_shuffle_ps(_mm_castsi128_ps(row0), _mm_castsi128_ps(row1), 0x44); \
		__m128 tmp2 = _mm_shuffle_ps(_mm_castsi128_ps(row0), _mm_castsi128_ps(row1), 0xEE); \
		__m128 tmp1 = _mm_shuffle_ps(_mm_castsi128_ps(row2), _mm_castsi128_ps(row3), 0x44); \
		__m128 tmp3 = _mm_shuffle_ps(_mm_castsi128_ps(row2), _mm_castsi128_ps(row3), 0xEE); \
		(row0) = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0x88)); \
		(row1) = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0xDD)); \
		(row2) = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0x88)); \
		(row3) = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0xDD)); \
	}

#else

#error TODO: GSVector4 and GSRasterizer needs SSE2

#endif

#if _M_SSE >= 0x301

	#include <tmmintrin.h>

#endif

#if _M_SSE >= 0x401

	#include <smmintrin.h>

#endif
