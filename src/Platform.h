///
///  Platform.h
///

#pragma once
#include <stdint.h>
#include "MTypes.h"

typedef int32_t Platform_Keycode;

void        Platform_Init();
const char* Platform_GetKeyName(Platform_Keycode k);
uint32_t    Platform_GetTicks();
void        Platform_FadeIn();
void        Platform_FadeOut();
[[noreturn]] void Platform_Error(const char* extra);
void        Platform_ReportAchievement(const char* achievementName, float percentComplete = 100.0f);
void        Platform_ReportLeaderboardScore(const char* leaderboardName, int score);
void        Platform_NotifyScreenLeftEdge(int leftEdge);
// pt in 1920x1080 canvas space. Just records the position; does not change visibility.
void        Platform_SetPauseButtonPosition(MPoint pt);
void        Platform_SetPauseButtonVisible(bool visible);
// On touch devices, prompts for a name via window.prompt() and writes it into
// name[0..maxLen-1]. No-op on non-touch or non-Emscripten builds.
void        Platform_PromptForHighScoreName(char* name, int maxLen, const char* message);
// Called at the end of Gfx_BlitFrontSurface to composite platform overlays
// (e.g. the touch pause button) on top of whatever was just blitted.
void        Platform_CompositeOverlays();
bool        Platform_IsTouchDevice();
bool        Platform_IsPauseButtonTap(MPoint canvasPt);
void        Platform_ResetTapDetection();

extern Platform_Keycode playerKeys[2][4];
extern const Platform_Keycode defaultPlayerKeys[2][4];

