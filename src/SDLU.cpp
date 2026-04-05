///
///  SDLU.c
///
///  SDL utilities.
///
///  John Stiles, 2002/10/12
///

#include "stdafx.h"
#ifdef __EMSCRIPTEN__
#include <unistd.h>
#endif

#include "SDLU.h"
#include "gameticks.h"
#include "music.h"
#include "gworld.h"
#include "main.h"
#include "Platform.h"
#include <deque>
#include <assert.h>

using std::deque;

// our SDL window and surface
SDL_Renderer*   g_renderer;
SDL_Window*     g_window;
SDL_Texture*    g_frontTexture;
SDL_Texture*    g_scratchTexture;


// for initsurface
SDL_Palette* g_grayscalePalette;

// for button and getmouse
static int          s_mouseButton;
static MPoint       s_mousePosition;

// for event loop
static MBoolean     s_isForeground = true;
 
// for checktyping
struct BufferedKey
{
    bool isASCII;
    
    union
    {
        char         ascii;
        SDL_Keycode  keycode;
    } value;
};


static MBoolean                s_interestedInTyping = false;
static std::deque<BufferedKey> s_keyBuffer;


int SDLUi_EventFilter(void*, SDL_Event *event)
{
    switch (event->type)
    {
        case SDL_TEXTINPUT:
        {
            // Put text input into a buffer.
            if (s_interestedInTyping)
            {
                for (char* asciiPtr = event->text.text; *asciiPtr; ++asciiPtr)
                {
                    BufferedKey key;
                    key.isASCII = true;
                    key.value.ascii = *asciiPtr;
                    s_keyBuffer.push_back(key);
                }
            }
            break;
        }
    
        case SDL_KEYDOWN:
        {
            // Put keydowns in a buffer
            if (s_interestedInTyping)
            {
                BufferedKey key;
                key.isASCII = false;
                key.value.keycode = event->key.keysym.sym;
                s_keyBuffer.push_back(key);
            }
            break;
        }
    
        // Get mouse state
        case SDL_MOUSEBUTTONDOWN:
        {
            if( event->button.button == SDL_BUTTON_LEFT )
                s_mouseButton = true;
            
            s_mousePosition.v = event->button.y;
            s_mousePosition.h = event->button.x;
            break;
        }
    
        case SDL_MOUSEBUTTONUP:
        {
            if( event->button.button == SDL_BUTTON_LEFT )
                s_mouseButton = false;
            
            s_mousePosition.v = event->button.y;
            s_mousePosition.h = event->button.x;
            break;
        }
    
        case SDL_MOUSEMOTION:
        {
            s_mousePosition.v = event->motion.y;
            s_mousePosition.h = event->motion.x;
            s_mouseButton = event->motion.state & SDL_BUTTON(1);
            break;
        }

        // Touch events (Emscripten). SDL synthesis is disabled so we handle these
        // directly. Position is in normalized 0-1 coords scaled to window pixels.
        case SDL_FINGERDOWN:
        {
            s_mouseButton = true;
            s_mousePosition.h = (short)(event->tfinger.x * 1920);
            s_mousePosition.v = (short)(event->tfinger.y * 1080);
            break;
        }

        case SDL_FINGERMOTION:
        {
            s_mousePosition.h = (short)(event->tfinger.x * 1920);
            s_mousePosition.v = (short)(event->tfinger.y * 1080);
            break;
        }

        case SDL_FINGERUP:
        {
            s_mouseButton = false;
            break;
        }

        case SDL_QUIT:
        {
            finished = true;
            break;
        }
            
        case SDL_WINDOWEVENT:
        {
            if (event->window.event == SDL_WINDOWEVENT_FOCUS_LOST && s_isForeground)
            {
                FreezeGameTickCount();
                EnableMusic(false);
                s_isForeground = false;
            }
            else if (event->window.event == SDL_WINDOWEVENT_FOCUS_GAINED && !s_isForeground)
            {
                UnfreezeGameTickCount();
                EnableMusic(musicOn);
                s_isForeground = true;
                
                if (DoFullRepaint != nullptr)
                {
                    DoFullRepaint();
                }
            }
            break;
        }
    }
    
    return 1;
}


void SDLU_Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        Platform_Error("SDL_Init failed");
    }
    atexit(SDL_Quit);

#ifdef __EMSCRIPTEN__
    // Our JS touch handlers (Platform_SetTouchTap / Platform_SetTouchTranslation) own
    // touch input; disable SDL's touch→mouse synthesis so there is only one source for
    // s_mousePosition and s_mouseButton on touch devices.
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");

    // SDL2's Emscripten backend calls emscripten_set_main_loop_timing internally,
    // which requires a main loop to be registered. We use ASYNCIFY instead of
    // emscripten_set_main_loop, so register a no-op to satisfy the precondition.
    // simulate_infinite_loop=0 means this returns immediately.
    emscripten_set_main_loop([](){}, 0, 0);

    // Expose the build ID so dynamically-injected package script URLs can be
    // cache-busted per build (same build = cache hit, new build = cache miss).
    EM_ASM({ window._ccBuildId = UTF8ToString($0); }, BUILD_ID);
#endif

    SDL_SetEventFilter(SDLUi_EventFilter, NULL);

    // Set up renderer and window.
    SDL_ShowCursor( SDL_DISABLE );
    
#ifdef __EMSCRIPTEN__
    // SDL_WINDOW_FULLSCREEN_DESKTOP hooks into the browser fullscreen API;
    // pressing Escape would exit fullscreen and resize the canvas mid-game.
    SDL_CreateWindowAndRenderer(1920, 1080, 0, &g_window, &g_renderer);
#else
//    SDL_CreateWindowAndRenderer(1920, 1080, 0, &g_window, &g_renderer);
    SDL_CreateWindowAndRenderer(1920, 1080, SDL_WINDOW_FULLSCREEN_DESKTOP, &g_window, &g_renderer);
#endif
    
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
    SDL_RenderSetLogicalSize(g_renderer, 1920, 1080);
    
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);
    SDL_RenderPresent(g_renderer);

    // Scratch texture: reusable TARGET buffer for multi-pass GPU compositing.
    g_scratchTexture = SDL_CreateTexture(g_renderer,
                                         SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_TARGET,
                                         1920, 1080);

    g_frontTexture = SDL_CreateTexture(g_renderer,
                                       SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_TARGET,
                                       1920, 1080);
    SDL_SetRenderTarget(g_renderer, g_frontTexture);

	// Initialize eight bit grayscale ramp palette.
	SDL_Color  grayscaleColors[256];
	for (int index = 0; index<256; index++)
	{
		grayscaleColors[index].r =
		grayscaleColors[index].g =
		grayscaleColors[index].b = index;
		grayscaleColors[index].a = 255;
	}

	g_grayscalePalette = SDL_AllocPalette(256);
	SDL_SetPaletteColors(g_grayscalePalette, grayscaleColors, 0, arrsize(grayscaleColors));
}

void SDLU_Yield()
{
    SDL_Delay( 2 );
    SDL_PumpEvents();
}

void SDLU_PumpEvents()
{
	static unsigned int  lastPump = 0;
	unsigned int  time = MTickCount();
	
	if( lastPump != time )
	{
        SDL_Event evt;
        while( SDL_PollEvent( &evt ) ) { }
		lastPump = time;
	}
}


MBoolean SDLU_IsForeground()
{
    return s_isForeground;
}


void SDLU_StartWatchingTyping(bool showKeyboard)
{
	s_interestedInTyping = true;
    s_keyBuffer.clear();
    if (showKeyboard)
        SDL_StartTextInput();
}


void SDLU_StopWatchingTyping()
{
	s_interestedInTyping = false;
    SDL_StopTextInput();
}


MBoolean SDLU_CheckASCIITyping(char* ascii)
{
    if (!s_keyBuffer.empty() && s_keyBuffer.front().isASCII)
	{
        *ascii = s_keyBuffer.front().value.ascii;
        s_keyBuffer.pop_front();
		return true;
	}

	*ascii = '\0';
	return false;
}


MBoolean SDLU_CheckSDLTyping(SDL_Keycode* sdlKey)
{
    if (!s_keyBuffer.empty() && !s_keyBuffer.front().isASCII)
    {
        *sdlKey = s_keyBuffer.front().value.keycode;
        s_keyBuffer.pop_front();
        return true;
    }
    
    *sdlKey = SDLK_UNKNOWN;
    return false;
}


static MPoint SDLUi_TranslatePointFromWindowToFrontSurface(MPoint pt)
{
    int windowWidth, windowHeight;
    SDL_GetWindowSize(g_window, &windowWidth, &windowHeight);
    
    pt.h = pt.h * g_frontSurface->w / windowWidth;
    pt.v = pt.v * g_frontSurface->h / windowHeight;

    return pt;
}


void SDLU_GetMouse( MPoint* pt )
{
	SDLU_PumpEvents();
	*pt = SDLUi_TranslatePointFromWindowToFrontSurface(s_mousePosition);
}


int SDLU_Button()
{
	SDLU_PumpEvents();
	return s_mouseButton;
}



void SDLU_Present()
{
    // Switch to the default (window backbuffer) to present.
    SDL_SetRenderTarget(g_renderer, nullptr);
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_frontTexture, NULL, NULL);
    SDL_RenderPresent(g_renderer);

    // Restore g_frontTexture as the active render target for the next frame.
    SDL_SetRenderTarget(g_renderer, g_frontTexture);
}


MBoolean SDLU_AnyKeyIsPressed( void )
{
    int arraySize;
    const Uint8* pressedKeys;
    
    SDLU_PumpEvents();
    pressedKeys = SDL_GetKeyboardState( &arraySize );
    
    for (int index = 0; index < arraySize; index++)
    {
        if (pressedKeys[index] &&
            index != SDL_SCANCODE_CAPSLOCK &&
            index != SDL_SCANCODE_NUMLOCKCLEAR &&
            index != SDL_SCANCODE_SCROLLLOCK)
        {
            return true;
        }
    }
    
    return false;
}


static bool s_escapeTap = false;

void SDLU_WaitForRelease( void )
{
    do
    {
        SDLU_Yield();
    }
    while( SDLU_AnyKeyIsPressed( ) || SDLU_Button() );
    SDLU_ResetEscapeState();
}


MBoolean SDLU_ControlKeyIsPressed( void )
{
    SDLU_PumpEvents();
    const Uint8* pressedKeys = SDL_GetKeyboardState( nullptr );
    return pressedKeys[SDL_SCANCODE_LCTRL] || pressedKeys[SDL_SCANCODE_RCTRL];
}

MBoolean SDLU_OptionKeyIsPressed( void )
{
    SDLU_PumpEvents();
    const Uint8* pressedKeys = SDL_GetKeyboardState( nullptr );
    return pressedKeys[SDL_SCANCODE_LALT] || pressedKeys[SDL_SCANCODE_RALT];
}

MBoolean SDLU_DeleteKeyIsPressed( void )
{
    SDLU_PumpEvents();
    const Uint8* pressedKeys = SDL_GetKeyboardState( nullptr );
    return pressedKeys[SDL_SCANCODE_BACKSPACE];
}

void SDLU_SetEscapeTap()
{
    s_escapeTap = true;
}

void SDLU_ResetEscapeState()
{
    s_escapeTap = false;
    Platform_ResetTapDetection();
}

void SDLU_SetMouseFromCanvas(int canvasX, int canvasY)
{
    s_mousePosition.h = (short)canvasX;
    s_mousePosition.v = (short)canvasY;
}

MBoolean SDLU_EscapeKeyIsPressed( void )
{
    SDLU_PumpEvents();
    const Uint8* pressedKeys = SDL_GetKeyboardState( nullptr );
    bool result = pressedKeys[SDL_SCANCODE_ESCAPE] || s_escapeTap;
    s_escapeTap = false;
    return result;
}


#ifdef __EMSCRIPTEN__
static bool s_pkgStarted[100] = {};
static bool s_worldPkgStarted[4] = {};  // index 1-3 used
#endif

void SDLU_PrefetchWorldPackage(int world)
{
#ifdef __EMSCRIPTEN__
    if (world < 1 || world > 3 || s_worldPkgStarted[world]) return;
    s_worldPkgStarted[world] = true;

    EM_ASM({
        var s = document.createElement('script');
        s.src = 'pkg_world' + $0 + '.js?v=' + (window._ccBuildId || '0');
        s.onerror = function() { Module._pkgFailed = Module._pkgFailed || {}; Module._pkgFailed['world' + $0] = true; };
        document.head.appendChild(s);
    }, world);
#endif
}

void SDLU_LoadWorldPackage(int world)
{
#ifdef __EMSCRIPTEN__
    SDLU_PrefetchWorldPackage(world);  // no-op if already started

    // Sentinel: first PICT of each world-complete bundle.
    static const char* kSentinels[4] = { nullptr,
        "CandyCrisisResources/PICT_600.jpg",
        "CandyCrisisResources/PICT_610.jpg",
        "CandyCrisisResources/PICT_630.jpg",
    };
    const char* sentinel = (world >= 1 && world <= 3) ? kSentinels[world] : nullptr;
    if (!sentinel) return;

    while (access(sentinel, F_OK) != 0)
    {
        if (EM_ASM_INT({ return (Module._pkgFailed && Module._pkgFailed['world' + $0]) ? 1 : 0; }, world))
        {
            Platform_Error("SDLU_LoadWorldPackage: package load failed");
            break;
        }
        emscripten_sleep(16);
    }
#endif
}

void SDLU_PrefetchLevelPackage(int levelID)
{
#ifdef __EMSCRIPTEN__
    int index = levelID - 5000;
    if (index < 0 || index >= 100) return;

    if (!s_pkgStarted[index])
    {
        s_pkgStarted[index] = true;

        // Inject the package loader script. onerror sets a flag so LoadLevelPackage
        // doesn't hang forever if the fetch fails.
        EM_ASM({
            var s = document.createElement('script');
            s.src = 'pkg_level_' + ($0 - 5000) + '.js?v=' + (window._ccBuildId || '0');
            s.onerror = function() { Module._pkgFailed = Module._pkgFailed || {}; Module._pkgFailed[$0] = true; };
            document.head.appendChild(s);
        }, levelID);

        // Chain world-complete prefetch: level 4 uses picture 3, level 8 uses picture 7,
        // level 12 uses picture 11. Start streaming the world-clear bundle alongside the
        // final level's character package so it's ready when the player clears the world.
        if (index == 3)       SDLU_PrefetchWorldPackage(1);
        else if (index == 7)  SDLU_PrefetchWorldPackage(2);
        else if (index == 11) SDLU_PrefetchWorldPackage(3);
    }

    // Prefetch the next level's package proactively to avoid hitches on transitions.
    // Always attempt this even if the current level was already started (e.g. prefetched
    // ahead of time), so the chain continues. Valid character indices are 0-11.
    int nextIndex = index + 1;
    if (nextIndex <= 11 && !s_pkgStarted[nextIndex])
    {
        s_pkgStarted[nextIndex] = true;
        EM_ASM({
            var s = document.createElement('script');
            s.src = 'pkg_level_' + ($0 - 5000) + '.js?v=' + (window._ccBuildId || '0');
            s.onerror = function() { Module._pkgFailed = Module._pkgFailed || {}; Module._pkgFailed[$0] = true; };
            document.head.appendChild(s);
        }, levelID + 1);
    }
#endif
}

void SDLU_LoadLevelPackage(int levelID)
{
#ifdef __EMSCRIPTEN__
    SDLU_PrefetchLevelPackage(levelID);  // no-op if already started

    // Yield until a known file inside the package directory appears in the
    // virtual FS.  Checking the directory itself is NOT sufficient: file_packager
    // emits FS_createPath() calls that run synchronously when the .js script
    // loads, so the directory exists before the data fetch completes.
    // Board.png is present in every PICT_5xxx character package.
    char filePath[80];
    snprintf(filePath, sizeof(filePath), "CandyCrisisResources/PICT_%d/Board.png", levelID);
    while (access(filePath, F_OK) != 0)
    {
        if (EM_ASM_INT({ return (Module._pkgFailed && Module._pkgFailed[$0]) ? 1 : 0; }, levelID))
        {
            Platform_Error("SDLU_LoadLevelPackage: package load failed");
            break;
        }
        emscripten_sleep(16);
    }
#endif
}
