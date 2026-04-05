///
///  Input_Emscripten.cpp
///
///  Compiled instead of keyselect.cpp for WebAssembly builds, exactly as
///  Input_iOS.mm replaces keyselect.cpp for iOS/tvOS builds.
///
///  Owns:
///    - Web virtual controller state (mirrors g_iPadVirtualController)
///    - WASM exports for JS touch handlers to call into
///    - CheckKeys() implementation (keyboard + touch, same structure as Input_iOS.mm)

#include "stdafx.h"
#include "keyselect.h"
#include "Globals.h"
#include "Platform.h"
#include "SDLU.h"
#include <emscripten.h>
#include <cmath>


// ---------------------------------------------------------------------------
// Pause button state — set by Platform_SetTouchTap, consumed once per CheckKeys() call
// ---------------------------------------------------------------------------
static bool s_pauseButtonPressed = false;

// Triple-tap detection state (file scope so Platform_ResetTapDetection can clear it)
static uint32_t s_tapTimes[3] = {};
static int      s_tapIndex    = 0;


// ---------------------------------------------------------------------------
// Web virtual controller state
// Mirrors g_iPadVirtualController / iPadVirtualController in Globals.h
// ---------------------------------------------------------------------------
struct WebVirtualController
{
    float translation_x = 0;
    float translation_y = 0;
    int   tap           = 0; // consumed flag, mirrors m_tap
};
static WebVirtualController g_webController[2];


// ---------------------------------------------------------------------------
// WASM exports — called from the touch handlers in shell.html
// ---------------------------------------------------------------------------

// Called on touchend when the touch qualifies as a tap (brief, stationary).
// canvasX/Y are in 1920x1080 space. Taps on the pause button trigger pause;
// all others rotate (mirrors m_tap = 1 in CandyCrisisViewController.mm).
// Three taps within 600 ms anywhere on screen triggers escape (skips victory/puppet screens).
extern "C" EMSCRIPTEN_KEEPALIVE void Platform_SetTouchTap(int player, int canvasX, int canvasY)
{
    SDLU_SetMouseFromCanvas(canvasX, canvasY);

    if (Platform_IsPauseButtonTap(MPoint{ (short)canvasY, (short)canvasX }))
    {
        s_pauseButtonPressed = true;
    }
    else
    {
        // Triple-tap detection (600 ms window)
        uint32_t now = Platform_GetTicks();
        s_tapTimes[s_tapIndex % 3] = now;
        ++s_tapIndex;
        if (s_tapIndex >= 3 && (now - s_tapTimes[s_tapIndex % 3]) <= 600)
        {
            SDLU_SetEscapeTap();
            s_tapIndex = 0;  // reset so the same taps don't re-trigger
        }

        if (player >= 0 && player < 2)
            g_webController[player].tap = 1;
    }
}

// Called on touchmove with the cumulative translation from the touch's start point.
// Mirrors storing [panRecognizer translationInView:] into m_translation.
extern "C" EMSCRIPTEN_KEEPALIVE void Platform_SetTouchTranslation(int player, float x, float y)
{
    if (player >= 0 && player < 2)
    {
        g_webController[player].translation_x = x;
        g_webController[player].translation_y = y;
    }
}


// ---------------------------------------------------------------------------
// CheckKeys — mirrors the structure of CheckKeys() in Input_iOS.mm
// ---------------------------------------------------------------------------
void CheckKeys()
{
    SDLU_PumpEvents();
    const Uint8* pressedKeys = SDL_GetKeyboardState(nullptr);

    // Snapshot of currently-active actions (keyboard only for now).
    KeyList tempHitKey[2] = {};
    for (int player = 0; player < 2; player++)
    {
        if (pressedKeys[SDL_GetScancodeFromKey(playerKeys[player][0])]) tempHitKey[player].left   = 1;
        if (pressedKeys[SDL_GetScancodeFromKey(playerKeys[player][1])]) tempHitKey[player].right  = 1;
        if (pressedKeys[SDL_GetScancodeFromKey(playerKeys[player][2])]) tempHitKey[player].drop   = 1;
        if (pressedKeys[SDL_GetScancodeFromKey(playerKeys[player][3])]) tempHitKey[player].rotate = 1;
    }

    // Web virtual controllers — mirrors the iPad virtual controller block in Input_iOS.mm.
    for (int i = 0; i < 2; i++)
    {
        // Tap → rotate (consumed immediately, mirrors m_tap handling)
        if (g_webController[i].tap)
        {
            tempHitKey[i].rotate = 1;
            g_webController[i].tap = 0;
        }

        // Pan → left / right / drop, same axis-selection logic and thresholds as iOS
        float tx = g_webController[i].translation_x;
        float ty = g_webController[i].translation_y;

        if (fabs(tx) > fabs(ty))
        {
            if      (tx < -9) tempHitKey[i].left  = 1;
            else if (tx >  9) tempHitKey[i].right = 1;
        }
        else
        {
            if (ty > 12) tempHitKey[i].drop = 1;
        }
    }

    // Transfer to hitKey counters — mirrors the transfer block in Input_iOS.mm.
    for (int player = 0; player < 2; player++)
    {
        if (tempHitKey[player].left > tempHitKey[player].right)
            hitKey[player].left++;
        else
            hitKey[player].left = 0;

        if (tempHitKey[player].right > tempHitKey[player].left)
            hitKey[player].right++;
        else
            hitKey[player].right = 0;

        if (tempHitKey[player].drop)
            hitKey[player].drop++;
        else
        {
            if (!g_dropLockEnabled)
                hitKey[player].drop = 0;
        }

        if (tempHitKey[player].rotate)
            hitKey[player].rotate++;
        else
            hitKey[player].rotate = 0;
    }

    pauseKey = pressedKeys[SDL_SCANCODE_ESCAPE] || s_pauseButtonPressed;
    s_pauseButtonPressed = false;
}

void Platform_ResetTapDetection()
{
    s_tapTimes[0] = s_tapTimes[1] = s_tapTimes[2] = 0;
    s_tapIndex = 0;
}
