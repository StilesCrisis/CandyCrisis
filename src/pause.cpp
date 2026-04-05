// pause.cpp

// All of this code is fugly. I really needed a dialog manager, but I didn't know it at the time,
// and instead I cobbled this together. It is just barely good enough to work. Fortunately it looks
// decent to the end user...


#include "stdafx.h"


#include "SDLU.h"

#include <algorithm>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "main.h"
#include "gameticks.h"
#include "blitter.h"
#include "graphics.h"
#include "grays.h"
#include "gworld.h"
#include "pause.h"
#include "players.h"
#include "random.h"
#include "font.h"
#include "music.h"
#include "soundfx.h"
#include "keyselect.h"
#include "level.h"
#include "victory.h"
#include "hiscore.h"
#include "score.h"
#include "UI_GameOverScreen.h"
#include "UI_VictoryScreen.h"
#include "UI_PuppetShow.h"
#include "tutorial.h"
#include "Platform.h"

using std::min;
using std::max;

const char kEscapeKey = 0x1B;

CC_RGBSurface*   backSurface;
CC_RGBSurface*   drawSurface;

static CC_RGBSurface* s_logoBakedSurface = nullptr;

// Konami cheat sequence state (up up down down left right left right)
static int    s_konamiProgress = 0;
static bool   s_swipeActive    = false;
static MPoint s_swipeStart     = {};

// Pause menu capsule UI state (valid only during kPauseDialog)
static int          s_pauseHighlight[6] = {};   // hover 0-255: [Resume, Quit, Controls, MusicOn, MusicOff, Skip]
static SDL_Texture* s_pauseCapsuleTex   = nullptr;  // 660×90 for large buttons
static SDL_Texture* s_pauseSmallCapTex  = nullptr;  // 172×72 for Music On/Off
static SDL_Texture* s_pauseSkipCapTex   = nullptr;  // 380×72 for Skip Level
static SDL_Texture* s_musicLabelPillTex = nullptr;  // 262×60 for Music label backdrop
const  int          kPauseCapR = 45, kPauseCapG = 73, kPauseCapB = 146;

// Controls dialog capsule UI state (valid only during kControlsDialog)
static int          s_ctrlHighlight[2] = {};    // hover 0-255: [OK, Reset]
static SDL_Texture* s_ctrlCapsuleTex   = nullptr;  // 260×72 shared for both buttons
static const MRect  kCtrlBtnRects[2] = {
    { 805,  670, 877,  930 },  // OK:    260×72
    { 805,  990, 877, 1250 },  // Reset: 260×72
};

// Controls dialog layout constants.
// All x/y values are in original 640×480 coordinate space and scaled at runtime
// via _HDW (x) and _HD (y) macros to 1920×1080.
static const int kCtrlRowTop      = 210;  // y of first row's label text
static const int kCtrlRowSpacing  = 16;   // vertical distance between row-pairs
static const int kCtrlDashOffsetY = 16;   // y offset from row top to dashed line
static const int kCtrlKeyOffsetY  = 2;    // y offset from row top to key name
static const int kCtrlCol0LabelX  = 130;  // x of left-column label
static const int kCtrlCol1LabelX  = 325;  // x of right-column label
static const int kCtrlCol0DashX   = 225;  // x start of left-column dashed line
static const int kCtrlCol1DashX   = 420;  // x start of right-column dashed line
static const int kCtrlCol0KeyX    = 269;  // center x of left-column key name
static const int kCtrlCol1KeyX    = 464;  // center x of right-column key name
static const int kCtrlHitH        = 24;   // hit-rect height per row
static const int kCtrlHitW        = 175;  // hit-rect width per row

// Button rects for the pause layout (top, left, bottom, right in 1920×1080 space).
// Logo: y=185–435 (250px tall, centered at x=960).
// Large buttons: 660×72, centered at x=960 (left=630, right=1290), ~20px gaps.
// Skip Level: 380×72, to the right of Quit at x=1332.
// Music row: 60px tall, matching TV storyboard x-positions, 40px below Controls.
static const MRect kPauseBtnRects[6] = {
    {  465,  630,  537, 1290 },  // 0: Resume Game        (660×72)
    {  557,  630,  629, 1290 },  // 1: Quit to Main Menu  (660×72)
    {  649,  630,  721, 1290 },  // 2: Controls           (660×72)
    {  761,  919,  821, 1091 },  // 3: Music On           (172×60)
    {  761, 1114,  821, 1286 },  // 4: Music Off          (172×60)
    {  557, 1332,  629, 1712 },  // 5: Skip Level         (380×72, right of Quit)
};
static const MRect kMusicLabelRect = { 761, 634, 821, 896 };  // 262×60

SkittlesFont*   smallFont;
SkittlesFont*   bigFont;
SkittlesFont*   dashedLineFont;
SkittlesFont*   continueFont;
SkittlesFont*   tinyFont;
MBoolean        continueTimeOut;

static int dialogType, dialogStage, dialogTimer, dialogShade, dialogItem;
static MRect logoRect = {0, 0, _HD(111), _HD(246)};
static MBoolean dialogStageComplete;
static MBoolean timeToRedraw = false;

// for the controls dialog
static int controlToReplace = -1;

// for the high score dialog
static char highScoreName[256];
static const char *highScoreText;
static const char *highScoreRank;

static void ItsTimeToRedraw()
{
	timeToRedraw = true;
}

enum
{
	kTextRainbow,
	kTextBrightRainbow,
	kTextWhite,
	kTextBlueGlow,
	kTextGray,
	kTextAlmostWhite
};

static MPoint DrawRainbowText( SkittlesFont* font, const char *line, MPoint dPoint, float wave, int bright, float scale = 1.0f )
{
	int   current;
	int   r,g,b;
	float s;
	
	current = 0;
	
	switch( bright )
	{	
			case kTextGray:
				r = g = b = 96;
				break;
				
			case kTextBlueGlow:
				s = sin(wave);
				r = (int)(88.0 + 120.0 * s * s);
				g = r;
				b = 255;
				break;
				
			case kTextWhite:
				r = g = b = 255;
				break;
				
			case kTextAlmostWhite:
            default:
				r = g = b = 224;
				break;
				
	}

	while( line[current] )
	{
		switch( bright )
		{
			case kTextBrightRainbow:
				r = (int)(208.0 + 40.0 * sin(wave                    ));
				g = (int)(208.0 + 40.0 * sin(wave + ((2.*pi) * 1./3.)));
				b = (int)(208.0 + 40.0 * sin(wave + ((2.*pi) * 2./3.)));
				break;

			case kTextRainbow:
				r = (int)(128.0 + 96.0 * sin(wave                    ));
				g = (int)(128.0 + 96.0 * sin(wave + ((2.*pi) * 1./3.)));
				b = (int)(128.0 + 96.0 * sin(wave + ((2.*pi) * 2./3.)));
				break;
		}

		float dx = (float)dPoint.h;
		SurfaceBlitScaledCharacter( font, line[current], &dx, (float)dPoint.v, scale, r, g, b, 1, 255 );
		dPoint.h = (int)dx;

		wave += 0.2;
		current++;
	}

	return dPoint;
}

enum
{
	kOpening = 0, 
	kClosing
};

static void DrawDialogCursor()
{
    MPoint p;
    SDLU_GetMouse( &p );
    SurfaceBlitCursor( p, drawSurface );
}

static void DrawDialogLogo( MRect *pauseRect, int shade )
{
	MRect drawRect;
	drawRect.left   = (pauseRect->left + ((pauseRect->right - pauseRect->left) * 1 / 2)) - (logoRect.right / 2);
	drawRect.top    = 185;
	drawRect.bottom = drawRect.top  + logoRect.bottom;
	drawRect.right  = drawRect.left + logoRect.right;

	int alpha = (shade > 63) ? 255 : (shade * 4);

	if (s_logoBakedSurface && s_logoBakedSurface->texture)
	{
		Gfx_AcquireSurface( drawSurface );
		SDL_Rect srcR = { 0, 0, logoRect.right, logoRect.bottom };
		SDL_Rect dstR = { drawRect.left, drawRect.top, logoRect.right, logoRect.bottom };
		SDL_SetTextureAlphaMod(s_logoBakedSurface->texture, (uint8_t)alpha);
		SDL_SetTextureColorMod(s_logoBakedSurface->texture, (uint8_t)alpha, (uint8_t)alpha, (uint8_t)alpha);
		SDL_RenderCopy(g_renderer, s_logoBakedSurface->texture, &srcR, &dstR);
		SDL_SetTextureAlphaMod(s_logoBakedSurface->texture, 255);
		SDL_SetTextureColorMod(s_logoBakedSurface->texture, 255, 255, 255);
		Gfx_ReleaseSurface( drawSurface );
	}
}


enum
{ 
	kNothing = -1,
	
// main pause screen (kEndGame is reused in continue and register)
	kMusic = 0,		kEndGame,
	kControls,		kResume,
	kWarp,

// continue screen
    kContinue,      
    
// controls screen
    k1PLeft,        k2PLeft,
    k1PRight,       k2PRight,
    k1PDrop,        k2PDrop,
    k1PRotate,      k2PRotate,
    kControlsOK,    kControlsReset,
};

static void DrawContinueContents( int *item, int shade )
{
	char line[4][50] = { "Do you want to continue?",
	                     "Yes",
	                     "No",
	                     "" };	                 
	MPoint dPoint[4] = { {_HD(233), _HDW(210)}, {_HD(280), _HDW(220)}, {_HD(280), _HDW(400)}, {_HD(335), _HDW(400)} }, hPoint = {_HD(255), _HDW(320)};
	static int lastCountdown = 0;
	int index, countdown, fade;
	int r, g, b;
	                 
	snprintf( line[3], sizeof(line[3]), "%d credit%c", credits, (credits != 1)? 's': ' ' );

	Gfx_AcquireSurface( drawSurface );

	for( index=0; index<4; index++ )
	{	
		DrawRainbowText( smallFont, line[index], dPoint[index], (0.25 * index) + (0.075 * shade), 
						 ( (index == 0)                          ||
						  ((index == 1) && (*item == kContinue)) ||
						  ((index == 2) && (*item == kEndGame ))    )? kTextBrightRainbow: kTextRainbow );
	}
	
	countdown = shade / 100;
	if( countdown < 10 )
	{
		continueTimeOut = false;
		
		if( (countdown != 0) && (countdown != lastCountdown) )
		{
			PlayMono( kContinueSnd );
		}
		lastCountdown = countdown;
		
		if( countdown < 5 )
		{
			r = (countdown * 31) / 5;
			g = 31;
		}
		else
		{
			r = 31;
			g = ((10 - countdown) * 31) / 5;
		}
			
		fade = shade % 100;
		if( fade > 50 ) fade = 50;
		r = ((31 * (49 - fade)) + (r * fade)) / 49;
		g = ((31 * (49 - fade)) + (g * fade)) / 49;
		b = ((31 * (49 - fade))) / 49;
		
		countdown = '9' - countdown;
		hPoint.h -= continueFont->width[countdown] / 2;

		for( shade = 4; shade > 0; shade-- )
		{
			float hx = (float)(hPoint.h + _HD(2 * shade));
			int   hy = hPoint.v + _HD(2 * shade);

            int weight = 20 - 4*shade;

			SurfaceBlitWeightedCharacter( continueFont, countdown, &hx, hy, 0, 0, 0, _5TO8(weight) );
		}

		SurfaceBlitCharacter( continueFont, countdown, &hPoint, _5TO8(r), _5TO8(g), _5TO8(b), 0 );
	}
	else
	{
		continueTimeOut = true;
	}
	
	Gfx_ReleaseSurface( drawSurface );
}

static void DrawHiScoreContents( int *item, int shade )
{
	MPoint       dPoint[3] = { {_HD(240), 2 * _HDW(320)}, {_HD(260), 2 * _HDW(320)}, {_HD(335), _HDW(400)} }, hPoint = {_HD(294), _HDW(145)};
	MPoint       dashedLinePoint = { _HD(320), _HDW(140) };
	int          index;
	int          nameLength;
    const char*  line[3];
	
	(void) item; // is unused

	line[0] = highScoreText;
	line[1] = Platform_IsTouchDevice() ? "Tap to save your score:" : "Please enter your name and press return:";
	line[2] = highScoreRank;

	for( index=0; index<2; index++ )
		dPoint[index].h = (short)((1920 - GetTextWidth( smallFont, line[index] )) / 2);	
		
	Gfx_AcquireSurface( drawSurface );	

	while( dashedLinePoint.h < _HDW(490) )
	{
		SurfaceBlitCharacter( dashedLineFont, '.', &dashedLinePoint, 0, 0, 0, 0 );
	}
	
	nameLength = int(strlen(highScoreName));
	for( index = 0; index < nameLength; index++ )
	{
		SurfaceBlitCharacter( bigFont, highScoreName[index], &hPoint, 255, 255, 255, 1 );
		if( hPoint.h >= _HDW(475) )
		{
			highScoreName[index] = '\0';
			break;
		}
	}

	if( !Platform_IsTouchDevice() )
	{
		index = (int)(( 1.0 + sin( MTickCount() / 7.5 ) ) * 120.0);
		SurfaceBlitCharacter( bigFont, '|', &hPoint, index, index, 255, 1 );
	}

	for( index=0; index<3; index++ )
	{	
		DrawRainbowText( smallFont, line[index], dPoint[index], (0.25 * index) + (0.075 * shade), (index != 2)? kTextBrightRainbow: kTextRainbow );
	}
	
	Gfx_ReleaseSurface( drawSurface );	
}

// Draw a capsule at a fixed color and alpha (no hover).
static void DrawCapsuleFlat(SDL_Texture* tex, const MRect& r, int cr, int cg, int cb, int alpha)
{
    if (!tex) return;
    SDL_Rect dst = { r.left, r.top, r.right - r.left, r.bottom - r.top };
    SDL_SetTextureColorMod(tex, (Uint8)cr, (Uint8)cg, (Uint8)cb);
    SDL_SetTextureAlphaMod(tex, (Uint8)alpha);
    SDL_RenderCopy(g_renderer, tex, nullptr, &dst);
    SDL_SetTextureColorMod(tex, 255, 255, 255);
    SDL_SetTextureAlphaMod(tex, 255);
}

// Draw a capsule button texture at the given rect with hover animation.
static void DrawCapsuleButton(SDL_Texture* tex, const MRect& r, int baseAlpha, int hv)
{
    if (!tex) return;
    int alpha = baseAlpha + (255 - baseAlpha) * hv / 255;
    int cr = kPauseCapR + (255 - kPauseCapR) * hv / 255;
    int cg = kPauseCapG + (255 - kPauseCapG) * hv / 255;
    int cb = kPauseCapB + (255 - kPauseCapB) * hv / 255;
    float scale = 1.0f + 0.025f * (hv / 255.0f);
    int ctrX = (r.left + r.right)  / 2;
    int ctrY = (r.top  + r.bottom) / 2;
    int w = (int)((r.right  - r.left) * scale);
    int h = (int)((r.bottom - r.top)  * scale);
    SDL_Rect dstR = { ctrX - w/2, ctrY - h/2, w, h };
    SDL_SetTextureColorMod(tex, (Uint8)cr, (Uint8)cg, (Uint8)cb);
    SDL_SetTextureAlphaMod(tex, (Uint8)alpha);
    SDL_RenderCopy(g_renderer, tex, nullptr, &dstR);
    SDL_SetTextureColorMod(tex, 255, 255, 255);
    SDL_SetTextureAlphaMod(tex, 255);
}

static void DrawControlsContents( int *item, int shade )
{
	MBoolean    highlight;
	MPoint      dPoint;
	int         index;
	const char* controlName;
	int         r, g, b;
	const char  label[8][20] = { "1P Left",   "2P Left", 
	                             "1P Right",  "2P Right", 
	                             "1P Drop",   "2P Drop",
	                             "1P Rotate", "2P Rotate" };
	                           
	                         
    // Phase 1: capsule buttons (direct SDL render, target = drawSurface->texture)
    {
        MPoint mouse;
        SDLU_GetMouse(&mouse);

        SDL_Texture* prevTarget = SDL_GetRenderTarget(g_renderer);
        SDL_SetRenderTarget(g_renderer, drawSurface->texture);

        for (int i = 0; i < 2; ++i)
        {
            int target = MPointInMRect(mouse, &kCtrlBtnRects[i]) ? 255 : 0;
            if      (s_ctrlHighlight[i] < target) { s_ctrlHighlight[i] += 15; if (s_ctrlHighlight[i] > target) s_ctrlHighlight[i] = target; }
            else if (s_ctrlHighlight[i] > target) { s_ctrlHighlight[i] -= 15; if (s_ctrlHighlight[i] < target) s_ctrlHighlight[i] = target; }
            DrawCapsuleButton(s_ctrlCapsuleTex, kCtrlBtnRects[i], 130, s_ctrlHighlight[i]);
        }

        SDL_SetRenderTarget(g_renderer, prevTarget);
    }

	// Phase 2: text rendering
	Gfx_AcquireSurface( drawSurface );

	for( index=0; index<8; index++ )
	{
		highlight = (index == (*item - k1PLeft));

		dPoint.v = kCtrlRowTop + ((index & ~1) * kCtrlRowSpacing);
		dPoint.h = (index & 1)? kCtrlCol1LabelX: kCtrlCol0LabelX;

		dPoint.h = _HDW(dPoint.h);
		dPoint.v = _HD(dPoint.v);
		DrawRainbowText( smallFont, label[index], dPoint, (0.25 * index) + (0.075 * shade), highlight? kTextBrightRainbow: kTextRainbow );

		dPoint.v = kCtrlRowTop + kCtrlDashOffsetY + ((index & ~1) * kCtrlRowSpacing);
		dPoint.h = (index & 1)? kCtrlCol1DashX: kCtrlCol0DashX;
		dPoint.h = _HDW(dPoint.h);
		dPoint.v = _HD(dPoint.v);

        r = highlight? 255: 0;
		g = b = (int)(highlight? 255.0 - (88.0 * (sin(shade * 0.2) + 1.0)): 0.0);

        SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 );
		SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 );
		SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 );
		SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 );
        SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 );
        SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 );

		controlName = SDL_GetKeyName( playerKeys[index & 1][index >> 1] );
		if( controlName == NULL ) controlName = "???";

		dPoint.v = kCtrlRowTop + kCtrlKeyOffsetY + ((index & ~1) * kCtrlRowSpacing);
        dPoint.v = _HD(dPoint.v);

        dPoint.h = (index & 1)? _HDW(kCtrlCol1KeyX): _HDW(kCtrlCol0KeyX);
		dPoint.h -= (int)(GetTextWidth( smallFont, controlName ) * 0.75f) / 2;

		DrawRainbowText( smallFont, controlName, dPoint, (0.1 * shade), (controlToReplace == index)? kTextBlueGlow: kTextWhite, 0.75f );
	}

    // Capsule button labels
    {
        SkittlesFont* boldFont = GetFont(picSkiaBoldFont);
        static const char* kCtrlBtnLabels[2] = { "OK", "Reset" };
        for (int i = 0; i < 2; ++i)
        {
            int hv     = s_ctrlHighlight[i];
            int color  = 255 - (255 * hv / 255);
            int shadow = (hv >= 128) ? 0 : 1;
            DrawScaledCenteredText(boldFont, kCtrlBtnLabels[i], kCtrlBtnRects[i], 0.8f, color, color, color, shadow);
        }
    }

	Gfx_ReleaseSurface( drawSurface );
}

static void DrawPauseContents( int *item, int shade )
{
    (void) item;
    (void) shade;

    MPoint mouse;
    SDLU_GetMouse(&mouse);

    int numLargeButtons = Platform_IsTouchDevice() ? 2 : 3;  // Controls button hidden on touch devices

    // Phase 1 — Capsule rendering (direct SDL, target = drawSurface->texture)
    {
        SDL_Texture* prevTarget = SDL_GetRenderTarget(g_renderer);
        SDL_SetRenderTarget(g_renderer, drawSurface->texture);

        // Animate highlight values toward hover target
        for (int i = 0; i < 6; ++i)
        {
            int target = (MPointInMRect(mouse, &kPauseBtnRects[i]) && (i != 5 || g_cheatsEnabled)) ? 255 : 0;
            if      (s_pauseHighlight[i] < target) { s_pauseHighlight[i] += 15; if (s_pauseHighlight[i] > target) s_pauseHighlight[i] = target; }
            else if (s_pauseHighlight[i] > target) { s_pauseHighlight[i] -= 15; if (s_pauseHighlight[i] < target) s_pauseHighlight[i] = target; }
        }

        // Large buttons: always visible at base 130 alpha, brighten+grow to white on hover
        for (int i = 0; i < numLargeButtons; ++i)
            DrawCapsuleButton(s_pauseCapsuleTex, kPauseBtnRects[i], 130, s_pauseHighlight[i]);

        // Music On/Off: active = base 130, inactive = base 60; both grow to white on hover
        for (int i = 3; i <= 4; ++i)
        {
            DrawCapsuleButton(s_pauseSmallCapTex, kPauseBtnRects[i], 130, s_pauseHighlight[i]);
        }

        // Skip Level: only visible when cheats are enabled
        if (g_cheatsEnabled)
            DrawCapsuleButton(s_pauseSkipCapTex, kPauseBtnRects[5], 110, s_pauseHighlight[5]);

        // Music label pill — gray semi-transparent background
        DrawCapsuleFlat(s_musicLabelPillTex, kMusicLabelRect, 128, 128, 128, 130);

        SDL_SetRenderTarget(g_renderer, prevTarget);
    }

    // Phase 2 — Text rendering (inside Gfx_AcquireSurface / ReleaseSurface)
    Gfx_AcquireSurface(drawSurface);
    {
        SkittlesFont* boldFont  = GetFont(picSkiaBoldFont);
        
        // Large button labels
        static const char* kBigLabels[3]         = { "Resume Game",     "Quit to Main Menu", "Controls" };
        static const char* kBigLabelsTutorial[3] = { "Resume Tutorial", "Skip Tutorial",     "Controls" };
        const char* (&bigLabels)[3] = (level == kTutorialLevel) ? kBigLabelsTutorial : kBigLabels;
        for (int i = 0; i < numLargeButtons; ++i)
        {
            int hv     = s_pauseHighlight[i];
            int color  = 255 - (255 * hv / 255);
            int shadow = (hv >= 128) ? 0 : 1;
            DrawScaledCenteredText(boldFont, bigLabels[i], kPauseBtnRects[i], 0.8f, color, color, color, shadow);
        }

        // Music label
        DrawScaledCenteredText(boldFont, "Music:", kMusicLabelRect, 0.6f, 255, 255, 255, 1);

        // Music On / Off  (active = brighter; inactive = mid-gray)
        for (int i = 3; i <= 4; ++i)
        {
            bool active  = (i == 3) ? musicOn : !musicOn;
            int  hv      = s_pauseHighlight[i];
            int  base    = active ? 255 : 192;
            int  color   = base - (base * hv / 255);
            int  shadow  = (hv >= 128) ? 0 : 1;
            const char* label = musicOn ? ((i == 3) ? "\x01 On" :      "Off")
                                        : ((i == 3) ?      "On" : "\x01 Off");
            DrawScaledCenteredText(boldFont, label, kPauseBtnRects[i], 0.6f, color, color, color, shadow);
        }

        // Skip Level
        if (g_cheatsEnabled)
        {
            int hv     = s_pauseHighlight[5];
            int color  = 255 - (255 * hv / 255);
            int shadow = (hv >= 128) ? 0 : 1;
            DrawScaledCenteredText(boldFont, "Skip Level", kPauseBtnRects[5], 0.6f, color, color, color, shadow);
        }
    }
    Gfx_ReleaseSurface(drawSurface);
}


static MBoolean ContinueSelected( int *item, unsigned char inKey, SDL_Keycode inSDLKey )
{
	MRect yes = {_HD(280), _HDW(220), _HD(300), _HDW(260)}, 
	      no  = {_HD(280), _HDW(400), _HD(300), _HDW(440)};
	MPoint p;
	
	(void) inSDLKey; // is unused
	
	if( continueTimeOut )
	{
		*item = kEndGame;
		return true;
	}
	
	if( inKey == kEscapeKey )
	{
		*item = kContinue;
		return true;
	}
	
	SDLU_GetMouse( &p );

	     if( MPointInMRect( p, &yes ) ) *item = kContinue;	
	else if( MPointInMRect( p, &no  ) ) *item = kEndGame;
	else *item = kNothing;
	
	return( SDLU_Button( ) && (*item != kNothing) );
}

static MBoolean HiScoreSelected( int *item, unsigned char inKey, SDL_Keycode inSDLKey )
{
	int nameLength = int(strlen(highScoreName));

	// On touch devices the name was already entered via window.prompt(); accept a tap to confirm.
	if( Platform_IsTouchDevice() && nameLength > 0 )
	{
		static MBoolean s_lastDown = false;
		MBoolean down = SDLU_Button();
		if( down && !s_lastDown )
		{
			s_lastDown = false;
			*item = kResume;
			PlayMono( kSquishy );
			return true;
		}
		s_lastDown = down;
	}

	// return (SDL key)
	if( inSDLKey == SDLK_RETURN )
	{
		if( nameLength > 0 )
		{
			*item = kResume;
			PlayMono( kSquishy );
            return true;
		}
		else
		{
			PlayMono( kClick );
		}
	}
	
	// backspace (SDL key)
	if( inSDLKey == SDLK_BACKSPACE )
	{
		if( nameLength > 0 )
		{
			highScoreName[ nameLength-1 ] = '\0';
			PlayMono( kClick );
		}
	}
	
	// characters (ASCII key!)
	if( bigFont->width[inKey] != 0 )
	{
		highScoreName[ nameLength++ ] = inKey;
		highScoreName[ nameLength   ] = '\0';
		PlayMono( kPlace );
	}
	
	*item = kNothing;
	return false;
}


static MBoolean ControlsSelected( int *item, unsigned char inKey, SDL_Keycode inSDLKey )
{
	MPoint          p;
	MRect           dRect;
	int             index;
	static MBoolean lastDown = false;
	MBoolean        down;
	const MRect&    okRect    = kCtrlBtnRects[0];
	const MRect&    resetRect = kCtrlBtnRects[1];
	int             returnValue = 0;

	(void) inKey; // unused

	*item = kNothing;

	down = SDLU_Button();
	SDLU_GetMouse( &p );

	if( MPointInMRect( p, &okRect ) )
	{
		*item = kControlsOK;
		if( down )
		{
			PlayMono( kClick );
			returnValue = 1;
			controlToReplace = -1;
		}
	}
	else if( MPointInMRect( p, &resetRect ) )
	{
		*item = kControlsReset;
		if( down && !lastDown )
		{
			PlayMono( kClick );
			memcpy( playerKeys, defaultPlayerKeys, sizeof(playerKeys) );
		}
	}
	else
	{
		for( index=0; index<8; index++ )
		{
			dRect.top    = kCtrlRowTop + ((index & ~1) * kCtrlRowSpacing);
			dRect.left   = (index & 1)? kCtrlCol1LabelX: kCtrlCol0LabelX;
			dRect.bottom = dRect.top  + kCtrlHitH;
			dRect.right  = dRect.left + kCtrlHitW;

            dRect.top    = _HD(dRect.top);
            dRect.bottom = _HD(dRect.bottom);
            dRect.left   = _HDW(dRect.left);
            dRect.right  = _HDW(dRect.right);
			if( MPointInMRect( p, &dRect ) )
			{
				*item = k1PLeft + index;
				if( down && !lastDown && !SDLU_AnyKeyIsPressed() ) 
				{
					controlToReplace = (controlToReplace == index)? -1: index;
				}
				break;
			}
		}
	}
	
	if( inSDLKey != 0 && controlToReplace != -1 )
	{
		playerKeys[controlToReplace & 1][controlToReplace >> 1] = inSDLKey;
		controlToReplace = -1;
	}
	
	lastDown = down;
	
	return returnValue;
}


static MBoolean PauseSelected( int *item, unsigned char inKey, SDL_Keycode inSDLKey )
{
    // Secret warp: tiny off-screen hit area (kept from original)
    static const MRect kSecretRect = { _HD(120), _HDW(550), _HD(130), _HDW(560) };

    // Konami code: up up down down left right left right
    static const SDL_Keycode kKonamiSeq[] = {
        SDLK_UP, SDLK_UP, SDLK_DOWN, SDLK_DOWN,
        SDLK_LEFT, SDLK_RIGHT, SDLK_LEFT, SDLK_RIGHT
    };
    static const int kKonamiLen = 8;

    // Logo swipe hit area (generous rect covering the logo at the top of the pause panel)
    static const MRect kLogoSwipeRect = { 185, 620, 460, 1300 };

    auto advanceKonami = [&](SDL_Keycode dir)
    {
        if (dir == kKonamiSeq[s_konamiProgress])
            ++s_konamiProgress;
        else
            s_konamiProgress = (dir == kKonamiSeq[0]) ? 1 : 0;

        if (s_konamiProgress >= kKonamiLen)
        {
            s_konamiProgress = 0;
            g_cheatsEnabled = true;
            PlayMono(kWhomp);
        }
    };

    // Arrow key input
    if (inSDLKey == SDLK_UP || inSDLKey == SDLK_DOWN ||
        inSDLKey == SDLK_LEFT || inSDLKey == SDLK_RIGHT)
    {
        advanceKonami(inSDLKey);
    }

    static MBoolean lastDown = false;
    MPoint p;
    SDLU_GetMouse(&p);

    // ESC resumes immediately
    if (inKey == kEscapeKey)
    {
        *item = kResume;
        PlayMono(kClick);
        return true;
    }

    MBoolean trigger = SDLU_Button();

    // Swipe detection on the logo area
    if (trigger)
    {
        if (!s_swipeActive && MPointInMRect(p, &kLogoSwipeRect))
        {
            s_swipeActive = true;
            s_swipeStart  = p;
        }
    }
    else if (s_swipeActive)
    {
        s_swipeActive = false;
        int dx = p.h - s_swipeStart.h;
        int dy = p.v - s_swipeStart.v;
        const int kThresh = 30;
        SDL_Keycode dir = SDLK_UNKNOWN;
        if (abs(dx) > abs(dy) && abs(dx) >= kThresh)
            dir = (dx < 0) ? SDLK_LEFT : SDLK_RIGHT;
        else if (abs(dy) >= kThresh)
            dir = (dy < 0) ? SDLK_UP : SDLK_DOWN;
        if (dir != SDLK_UNKNOWN)
            advanceKonami(dir);
    }

    if (trigger)
    {
        if (!lastDown)
        {
            lastDown = true;

            // Music On (index 3)
            if (MPointInMRect(p, &kPauseBtnRects[3]))
            {
                PlayMono(kClick);
                musicOn = true;
                EnableMusic(true);
                return false;
            }
            // Music Off (index 4)
            if (MPointInMRect(p, &kPauseBtnRects[4]))
            {
                PlayMono(kClick);
                musicOn = false;
                EnableMusic(false);
                return false;
            }
            // Skip Level (index 5) — only when cheats are enabled
            if (g_cheatsEnabled && MPointInMRect(p, &kPauseBtnRects[5]))
            {
                PlayMono(kClick);
                *item = kWarp;
                return true;
            }
            // Controls (index 2)
            if (!Platform_IsTouchDevice() && MPointInMRect(p, &kPauseBtnRects[2]))
            {
                PlayMono(kClick);
                *item = kControls;
                return true;
            }
            // Quit to Main Menu (index 1)
            if (MPointInMRect(p, &kPauseBtnRects[1]))
            {
                PlayMono(kClick);
                *item = kEndGame;
                return true;
            }
            // Resume Game (index 0)
            if (MPointInMRect(p, &kPauseBtnRects[0]))
            {
                PlayMono(kClick);
                *item = kResume;
                return true;
            }
            // Secret warp
            if (MPointInMRect(p, &kSecretRect))
            {
                *item = kWarp;
                level++;
                return true;
            }
        }
    }
    else
    {
        lastDown = false;
    }

    *item = kNothing;
    return false;
}


void UI_ShowDialog(UIDialogType type)
{	
	CC_Rect       fullSDLRect = { 0, 0, 1920, 1080 };
	int            skip = 1;
	char           inASCII;
	SDL_Keycode    inSDLKey;
	MRect          pauseRect;
	
	// Clear state 
	controlToReplace = -1;
	
	// Remember dialog info
	dialogType = type;
	dialogStage = kOpening;

	smallFont      = GetFont( picFont );
	tinyFont       = GetFont( picFont );
	bigFont        = GetFont( picSkiaFont );
	dashedLineFont = GetFont( picDashedLineFont );
	continueFont   = GetFont( picContinueFont );
		
	// Load logo from single RGBA PNG (PICT_401 = "Candy Crisis Logo.png").
	// LoadPICTAsRGBSurface premultiplies alpha into the pixel data and uploads a GPU texture.
	s_logoBakedSurface = LoadPICTAsRGBSurface(picLogo);
	if (s_logoBakedSurface)
	{
		SDL_SetTextureBlendMode(s_logoBakedSurface->texture, Gfx_PremultipliedBlendMode());
		logoRect = { 0, 0, (short)s_logoBakedSurface->h, (short)s_logoBakedSurface->w };
	}

    // Load the overlay image.
    CC_RGBSurface* pauseOverlay = LoadPICTAsRGBSurface(picPauseOverlay);

	// Get a copy of the current game window contents
	backSurface      = Gfx_InitRGBSurface(1920, 1080);
	
	Gfx_BlitSurface( g_frontSurface, &g_frontSurface->clip_rect,
	                  backSurface,  &backSurface->clip_rect );
		
	drawSurface      = Gfx_InitRGBSurface(1920, 1080);

	Gfx_BlitSurface( backSurface, &backSurface->clip_rect,
	                  drawSurface, &drawSurface->clip_rect  );

	//
		
	PlayMono( kWhomp );
	dialogTimer = MTickCount();
	dialogShade = 0;
	dialogStageComplete = false;
	dialogItem = kNothing;

    // Pause-specific capsule UI setup
    if (type == kPauseDialog)
    {
        memset(s_pauseHighlight, 0, sizeof(s_pauseHighlight));
        s_pauseCapsuleTex  = CreateCapsuleTexture(660, 72);  // large buttons
        s_pauseSmallCapTex = CreateCapsuleTexture(172, 60);  // music On/Off
        s_pauseSkipCapTex  = CreateCapsuleTexture(370, 60);  // skip level
        s_musicLabelPillTex = CreateCapsuleTexture(262, 60); // music label backdrop
        s_konamiProgress   = 0;
        s_swipeActive      = false;
    }

    // Controls-specific capsule UI setup
    if (type == kControlsDialog)
    {
        memset(s_ctrlHighlight, 0, sizeof(s_ctrlHighlight));
        s_ctrlCapsuleTex = CreateCapsuleTexture(260, 72);  // OK / Reset buttons
    }

    // Define the interaction area (pauseRect) roughly where the old dialog was.
    // The old dialog centered on (240, 320) in 640x480 units.
    // Height approx 240 units, Width approx 480 units.
    pauseRect = { _HD(120), _HDW(80), _HD(360), _HDW(560) };

	SDLU_StartWatchingTyping(type == kHiScoreDialog);
	
	DoFullRepaint = ItsTimeToRedraw;

    int fadeAlpha = 0;
    const int kFadeSpeed = 20;

	while( ((dialogStage != kClosing) || !dialogStageComplete) && !finished )
	{
		dialogTimer += skip;

        UpdateSound();
        
		// Check mouse and keyboard
        SDLU_CheckASCIITyping( &inASCII );
        SDLU_CheckSDLTyping( &inSDLKey );
		
		if( (dialogStage == kOpening) && dialogStageComplete )
		{
			MBoolean (*DialogSelected[kNumDialogs])( int *item, unsigned char inKey, SDL_Keycode inSDLKey ) =
			{
				PauseSelected,
				HiScoreSelected,
				ContinueSelected,
				ControlsSelected,
			};
			
			if( DialogSelected[dialogType]( &dialogItem, inASCII, inSDLKey ) )
			{
				dialogStage = kClosing; 
			}
		}

		// Do animation ...
        if (dialogStage == kOpening)
        {
            fadeAlpha += kFadeSpeed;
            if (fadeAlpha >= 255)
            {
                fadeAlpha = 255;
                dialogStageComplete = true;
            }
        }
        else
        {
            fadeAlpha -= kFadeSpeed;
            if (fadeAlpha <= 0)
            {
                fadeAlpha = 0;
                dialogStageComplete = true; // Finished closing
            }
        }

        // Draw
        // 1. Restore background
        Gfx_BlitSurface(backSurface, &backSurface->clip_rect, drawSurface, &drawSurface->clip_rect);
        
        // 2. Draw Overlay
        if (pauseOverlay && pauseOverlay->texture)
        {
            SDL_SetTextureAlphaMod(pauseOverlay->texture, fadeAlpha);
            SDL_SetTextureColorMod(pauseOverlay->texture, fadeAlpha, fadeAlpha, fadeAlpha);
            MRect srcRect, dstRect;
            Gfx_CCRectToMRect(&pauseOverlay->clip_rect, &srcRect);
            Gfx_CCRectToMRect(&drawSurface->clip_rect, &dstRect);
            SurfaceBlendOver(drawSurface, &dstRect, pauseOverlay, &srcRect);
        }

		if( (dialogStage == kOpening) && dialogStageComplete )
		{
			void (*DialogDraw[kNumDialogs])( int *item, int shade ) =
			{
				DrawPauseContents,
				DrawHiScoreContents,
				DrawContinueContents,
				DrawControlsContents,
			};

			// Refresh screen if necessary
			if( timeToRedraw )
			{
				Gfx_BlitFrontSurface( backSurface, &fullSDLRect, &fullSDLRect );

				timeToRedraw = false;
			}
			
			// ... and fade in the logo

			dialogShade += skip;

			{
				const MBoolean dialogHasCandyCrisisLogo[kNumDialogs] = { true, true, true, true };
				
				if( dialogHasCandyCrisisLogo[dialogType] )
					DrawDialogLogo( &pauseRect, dialogShade );
			}
			
			// ... and animation is complete so add content			
			DialogDraw[dialogType]( &dialogItem, dialogShade );
			
			// ... and cursor
			DrawDialogCursor();
		}

		// Draw new animation on screen
		Gfx_BlitFrontSurface( drawSurface, &fullSDLRect, &fullSDLRect );
        SDLU_Present();

		// Wait for next frame
		if( dialogTimer <= MTickCount( ) )
		{
			dialogTimer = MTickCount( );
			skip = 2;
		}
		else
		{
			skip = 1;
			while( dialogTimer > MTickCount( ) ) 
			{
				SDLU_Yield();  
			}
		}
	}
	
	DoFullRepaint = nullptr;

	SDLU_StopWatchingTyping();

	// Bring back previous screen
	Gfx_BlitFrontSurface( backSurface, &fullSDLRect, &fullSDLRect );
    SDLU_Present();

	// Clean up capsule textures
    if (s_pauseCapsuleTex)  { SDL_DestroyTexture(s_pauseCapsuleTex);  s_pauseCapsuleTex  = nullptr; }
    if (s_pauseSmallCapTex) { SDL_DestroyTexture(s_pauseSmallCapTex); s_pauseSmallCapTex = nullptr; }
    if (s_pauseSkipCapTex)  { SDL_DestroyTexture(s_pauseSkipCapTex);  s_pauseSkipCapTex  = nullptr; }
    if (s_musicLabelPillTex){ SDL_DestroyTexture(s_musicLabelPillTex);s_musicLabelPillTex= nullptr; }
    if (s_ctrlCapsuleTex)   { SDL_DestroyTexture(s_ctrlCapsuleTex);   s_ctrlCapsuleTex   = nullptr; }

	// Dispose the GWorlds and fonts we used
	Gfx_FreeSurface( backSurface );
	Gfx_FreeSurface( drawSurface );
    if (pauseOverlay) Gfx_FreeSurface(pauseOverlay);
	Gfx_FreeSurface( s_logoBakedSurface ); s_logoBakedSurface = nullptr;
			
	switch( dialogItem )
	{
		case kControls:
			UI_ShowDialog( kControlsDialog );
			UI_ShowDialog( kPauseDialog );
			break;
		
		case kEndGame:
			ChooseMusic( -1 );
			if( players == 1 )
			{
                UI_AddHighScoreScreen(score[0], e_AfterUIScreen_GameOver);
			}
			else
			{
                UI_AddHighScoreScreen(score[0], e_AfterUIScreen_Title);
			}
			break;
		
		case kContinue:
			displayedScore[0] = score[0] = roundStartScore[0];
			ShowScore( 0 );
			BeginRound( true );
			break;
			
		case kWarp:
		{
            if (control[0] == kPlayerControl &&
                control[1] == kAIControl        )
            {
                // For a match versus the CPU, dump a screenful of garbage on him.
                CalculateGrays(1, 77);
                LockGrays(1);
            }
            else if (control[0] == kPlayerControl &&
                     control[1] == kNobodyControl    )
            {
                // For solitaire, increase the player score.
                score[0] = ScoreToReachSolitaireLevel(level);
            }
		}
	}
}


void UI_DoTransitionAfterUIScreen(TransitionAfterUIScreen t)
{
    switch (t)
    {
        case e_AfterUIScreen_GameOver:
            UI_ShowGameOverScreen(); break;
        case e_AfterUIScreen_Victory_World1:
            UI_ShowWorld1VictoryScreen(e_AfterUIScreen_Title); break;
        case e_AfterUIScreen_Victory_World2:
            UI_ShowWorld2VictoryScreen(e_AfterUIScreen_Title); break;
        case e_AfterUIScreen_Victory_World3:
            UI_ShowWorld3VictoryScreen(e_AfterUIScreen_Title); break;
        case e_AfterUIScreen_Title:
        default:
            Platform_FadeOut(); showStartMenu = true; break;
    }
}


void UI_AddHighScoreScreen(int score, TransitionAfterUIScreen behavior)
{
    short count, item;
    char rank[50];
    char text[256];
    const char *playerName = "You";
    const char *twoPlayerNames[] = { "Player 1", "Player 2" };
    int highScoreLevel = -1;
    
    
    // Check for high score
    // (only do high scores if it's player vs CPU)
    
    if( players == 1 &&
       control[0] == kPlayerControl &&
       control[1] == kAIControl        )
    {
        for( count=0; count<=9; count++ )
        {
            if( score >= scores[count].score )
            {
                snprintf( rank, sizeof(rank), "%d points", score );
                highScoreLevel = count;
                break;
            }
        }
    }
    
    // Determine player's name for best combo
    
    if( players == 2 ) playerName = twoPlayerNames[evenBetter.player];
    
    
    // See if both best combo AND high score
    
    if( evenBetter.value > best.value && highScoreLevel != -1 )
    {
        snprintf( text, sizeof(text), "You got a high score and the best combo!" );
        
        highScoreText = text;
        highScoreRank = rank;
        
        Platform_PromptForHighScoreName(highScoreName, sizeof(highScoreName), highScoreText);
        UI_ShowDialog( kHiScoreDialog );

        if( !finished )
        {
            highScoreName[kNameLength] = '\0';

            memcpy( &best, &evenBetter, sizeof(Combo) );
            strcpy( best.name, highScoreName );

            for( item=8; item>=highScoreLevel; item-- )
            {
                memmove( &scores[item+1], &scores[item], sizeof( HighScore ) );
            }

            scores[highScoreLevel].score = score;
            strcpy( scores[highScoreLevel].name, highScoreName );
        }
    }

    // See if best combo has been won

    else if( evenBetter.value > best.value )
    {
        snprintf( text, sizeof(text), "Congratulations! %s got best combo!", playerName );

        highScoreText = text;
        highScoreRank = "";

        Platform_PromptForHighScoreName(highScoreName, sizeof(highScoreName), highScoreText);
        UI_ShowDialog( kHiScoreDialog );

        if( !finished )
        {
            highScoreName[kNameLength] = '\0';

            memcpy( &best, &evenBetter, sizeof(Combo) );
            strcpy( best.name, highScoreName );
        }
    }

    // See if high score has been won

    else if( highScoreLevel != -1 )
    {
        highScoreText = "Congratulations! You got a high score!";
        highScoreRank = rank;

        Platform_PromptForHighScoreName(highScoreName, sizeof(highScoreName), highScoreText);
        UI_ShowDialog( kHiScoreDialog );

        if( !finished )
        {
            highScoreName[kNameLength] = '\0';

            for( item=8; item>=highScoreLevel; item-- )
            {
                memmove( &scores[item+1], &scores[item], sizeof( HighScore ) );
            }

            scores[highScoreLevel].score = score;
            strcpy( scores[highScoreLevel].name, highScoreName );
        }
    }
    
    // Honor the "behavior afterwards" parameter.
    
    switch (behavior)
    {
        case e_AfterUIScreen_GameOver:
            UI_ShowGameOverScreen();
            break;

        case e_AfterUIScreen_Title:
            Platform_FadeOut();
            showStartMenu = true;
            break;

        case e_AfterUIScreen_Victory_World1:
            UI_ShowWorld1VictoryScreen(e_AfterUIScreen_Title); break;
        case e_AfterUIScreen_Victory_World2:
            UI_ShowWorld2VictoryScreen(e_AfterUIScreen_Title); break;
        case e_AfterUIScreen_Victory_World3:
            UI_ShowWorld3VictoryScreen(e_AfterUIScreen_Title); break;

        case e_AfterUIScreen_Game:
            break;

        default:
            break;
    }
}
