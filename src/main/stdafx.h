
#ifndef STDAFX_H_
#define STDAFX_H_

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently.
// ref: https://learn.microsoft.com/ja-jp/cpp/build/creating-precompiled-header-files?view=msvc-170


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <cstdint>
#include <d3d11.h>

#include <wrl.h>
#include <shellapi.h>

#include "SimpleMath.h"

#endif // STDAFX_H_
