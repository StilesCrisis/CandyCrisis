///
///  stdafx.h
///

#pragma once

#if _WIN32

#define _CRT_SECURE_NO_WARNINGS 1

// strlcpy is a BSD extension not available on Windows
#include <string.h>
static inline size_t strlcpy(char* dst, const char* src, size_t size)
{
    size_t srcLen = strlen(src);
    if (size > 0) {
        size_t copyLen = (srcLen < size - 1) ? srcLen : size - 1;
        memcpy(dst, src, copyLen);
        dst[copyLen] = '\0';
    }
    return srcLen;
}

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX                        // Suppress min/max macros from windows.h
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <algorithm>

#include "SDL.h"
#include "SDL_image.h"

#endif
#if __APPLE__

#define SDL_MAIN_HANDLED  // SDL must not redefine main() on Apple platforms

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>

#include "SDL.h"
#include "SDL_image.h"

#endif

#ifdef __EMSCRIPTEN__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>

#include "SDL.h"
#include "SDL_image.h"

#include <emscripten.h>
#include <emscripten/html5.h>

#endif