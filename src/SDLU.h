///
///  SDLU.h
///


#pragma once

#include "MTypes.h"
#if __APPLE__
#include "SDL.h"
#endif

void         SDLU_Init();
void         SDLU_GetMouse( MPoint* pt );
int          SDLU_Button();
void         SDLU_Yield();
void         SDLU_PumpEvents();
void         SDLU_StartWatchingTyping(bool showKeyboard = false);
void         SDLU_StopWatchingTyping();
MBoolean     SDLU_CheckASCIITyping(char* ascii);
MBoolean     SDLU_CheckSDLTyping(SDL_Keycode* sdlKey);
MBoolean     SDLU_IsForeground();
MBoolean     SDLU_AnyKeyIsPressed();
void         SDLU_Present();
void         SDLU_WaitForRelease();
MBoolean     SDLU_ControlKeyIsPressed();
MBoolean     SDLU_OptionKeyIsPressed();
MBoolean     SDLU_DeleteKeyIsPressed();
MBoolean     SDLU_EscapeKeyIsPressed();
void         SDLU_SetEscapeTap();        // signal escape from touch (e.g. triple-tap)
void         SDLU_SetMouseFromCanvas(int canvasX, int canvasY);  // update cursor pos from 1920x1080 canvas coords
void         SDLU_PrefetchLevelPackage(int levelID);  // fire-and-forget: start download, don't wait
void         SDLU_LoadLevelPackage(int levelID);      // start if needed, then wait until ready
void         SDLU_PrefetchWorldPackage(int world);   // world=1/2/3; fire-and-forget
void         SDLU_LoadWorldPackage(int world);       // start if needed, then wait until ready


extern SDL_Renderer*   g_renderer;
extern SDL_Window*     g_window;
extern SDL_Texture*    g_frontTexture;     // TEXTUREACCESS_TARGET; the main 1920x1080 game frame
extern SDL_Texture*    g_scratchTexture;   // TEXTUREACCESS_TARGET 1920x1080; reusable scratch for multi-pass GPU compositing
