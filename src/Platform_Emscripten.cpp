///
///  Platform_Emscripten.cpp
///
///  Emscripten-specific platform implementations.
///

#ifdef __EMSCRIPTEN__

#include "stdafx.h"
#include "Platform.h"
#include "main.h"
#include "gworld.h"
#include "graphics.h"
#include "SDLU.h"
#include <cmath>
#include <cstdlib>

// Notifies shell.html of the current screen left edge (mirrors g_iPadScreenLeftEdge).
// JS uses this to left-align the canvas during the tutorial so the chalkboard
// isn't cropped on narrow (4:3) screens.
void Platform_NotifyScreenLeftEdge(int leftEdge)
{
    EM_ASM({ if (typeof candyCrisis_setScreenLeftEdge === 'function') candyCrisis_setScreenLeftEdge($0); }, leftEdge);
}

// ---------------------------------------------------------------------------
// Pause button — composited onto g_frontTexture via Platform_CompositeOverlays
// ---------------------------------------------------------------------------

// Button size in 1920x1080 canvas space: 60pt in 768pt → 84 canvas units at 1080p.
static const int kPauseButtonSize = 84;

static MPoint         s_buttonPt    = {};
static CC_RGBSurface* s_pauseIcon   = nullptr;
static bool           s_buttonShown = false;

void Platform_Init()
{
    if (Platform_IsTouchDevice())
        s_pauseIcon = LoadPICTAsRGBSurface(picPauseIcon);
}

void Platform_SetPauseButtonPosition(MPoint pt)
{
    s_buttonPt = pt;
}

void Platform_SetPauseButtonVisible(bool visible)
{
    s_buttonShown = visible && Platform_IsTouchDevice();
}

void Platform_CompositeOverlays()
{
    if (!s_buttonShown || !s_pauseIcon) return;

    int cx   = s_buttonPt.h;
    int cy   = s_buttonPt.v;
    int half = kPauseButtonSize / 2;

    SDL_Rect iconSrc = { 0, 0, s_pauseIcon->w, s_pauseIcon->h };
    SDL_Rect btnDst  = { cx - half, cy - half, kPauseButtonSize, kPauseButtonSize };

    SDL_Texture* prev = SDL_GetRenderTarget(g_renderer);
    SDL_SetRenderTarget(g_renderer, g_frontTexture);
    SDL_RenderCopy(g_renderer, s_pauseIcon->texture, &iconSrc, &btnDst);
    SDL_SetRenderTarget(g_renderer, prev);
}

bool Platform_IsPauseButtonTap(MPoint pt)
{
    if (!s_buttonShown) return false;
    int half = kPauseButtonSize / 2;
    return std::abs(pt.h - s_buttonPt.h) <= half &&
           std::abs(pt.v - s_buttonPt.v) <= half;
}

void Platform_PromptForHighScoreName(char* name, int maxLen, const char* message)
{
    if (!Platform_IsTouchDevice()) return;

    char* result = (char*)EM_ASM_PTR({
        var current = UTF8ToString($0);
        var msg = UTF8ToString($1) + "\n\nEnter your name:";
        var entered = window.prompt(msg, current);
        if (!entered) return 0;
        var len = lengthBytesUTF8(entered) + 1;
        var buf = _malloc(len);
        stringToUTF8(entered, buf, len);
        return buf;
    }, name, message);

    if (result)
    {
        strncpy(name, result, maxLen - 1);
        name[maxLen - 1] = '\0';
        free(result);
    }
}

bool Platform_IsTouchDevice()
{
    static bool isTouchDevice = EM_ASM_INT({ return window.matchMedia('(pointer: coarse)').matches ? 1 : 0; });
    return isTouchDevice;
}

#endif // __EMSCRIPTEN__
