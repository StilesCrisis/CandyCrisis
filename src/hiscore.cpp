// hiscore.c

#include "stdafx.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#include "main.h"
#include "gworld.h"
#include "graphics.h"
#include "score.h"
#include "hiscore.h"
#include "Input.h"
#include "font.h"
#include "blitter.h"
#include "random.h"
#include "pause.h"
#include "level.h"
#include "tutorial.h"
#include "graymonitor.h"
#include "players.h"
#include "gameticks.h"
#include "music.h"
#include "soundfx.h"

Combo defaultBest = 
{
	/*bestGrid[kGridAcross][kGridDown] = */ 
	{ { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty,                          1, 1, 1, 2, 2 },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty,           1, 1 },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty } },

	/*bestA      = */ 2,
	/*bestB      = */ 2,
	/*bestM      = */ false, 
	/*bestG      = */ false, 
	/*bestLv     = */ kTutorialLevel,
	/*bestX      = */ 1,
	/*bestR      = */ upRotate,
	/*bestPlayer = */ 0,
	/*bestValue  = */ (40*1) + (50*9),
	/*bestName   = */ "Tutorial"
};

Combo best = 
{
	/*bestGrid[kGridAcross][kGridDown] = */ 
	{ { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty,                          1, 1, 1, 2, 2 },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty,           1, 1 },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	  { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty } },

	/*bestA      = */ 2,
	/*bestB      = */ 2,
	/*bestM      = */ false, 
	/*bestG      = */ false, 
	/*bestLv     = */ kTutorialLevel,
	/*bestX      = */ 1,
	/*bestR      = */ upRotate,
	/*bestPlayer = */ 0,
	/*bestValue  = */ (40*1) + (50*9),
	/*bestName   = */ "Tutorial"
};

Combo evenBetter = {};
Combo potentialCombo[2];

char g_recentHighScoreNames[3][kNameLength] = { "Darby", "Porkchop", "Mei" };

AutoPattern hiScorePattern[] =
{
	{ e_AutoCommand_Wait,               60, 0,   NULL },
	{ e_AutoCommand_kBlockUntilDrop,    0,  0,   NULL },
	{ e_AutoCommand_BlockUntilComplete, 0,  0,   NULL },	
	{ e_AutoCommand_Wait,               60, 0,   NULL },
	{ e_AutoCommand_ShowStartMenu,      0,  0,   NULL }
};

HighScore scores[10] = 
{	
	{"Leviathan", 40000},
	{"Yuki", 36000},
	{"Tan-chan's Big Sister", 32000},
	{"Fluffy", 28000},
	{"Tan-chan", 24000},
	{"Calamari Squad", 20000},
	{"Mei", 16000},
	{"Porkchop", 12000},
	{"Darby", 8000},
	{"Mister Pugglesworth", 4000}
};

HighScore defaultScores[10] = 
{	
    {"Leviathan", 40000},
    {"Dr. Crisis", 36000},
    {"Tan-chan's Big Sister", 32000},
    {"Fluffy", 28000},
    {"Tan-chan", 24000},
    {"Calamari Squad", 20000},
    {"Mei", 16000},
    {"Porkchop", 12000},
    {"Darby", 8000},
    {"Mister Pugglesworth", 4000}
};



void InitPotentialCombos( void )
{
    potentialCombo[0] = Combo{};
    potentialCombo[1] = Combo{};
	potentialCombo[0].player = 0;
	potentialCombo[1].player = 1;
}

void SubmitCombo( Combo *in )
{
	if( in->value > best.value && in->value > evenBetter.value )
	{
		PlayMono( kContinueSnd );
        evenBetter = *in;
	}	
}

// Please note: This function may well be the biggest kludge in Candy Crisis.
// It relies on tons of insider knowledge. But it works.
void ShowBestCombo( void )
{
    char bestCombo[500];
    snprintf( bestCombo, arrsize(bestCombo), "BEST COMBO - %s (%d points)", best.name, best.value );

	int levelCap;
	
	SkittlesFont*   boldFont = GetFont( picSkiaBoldFont );
	
	StopBalloon( );
	InitGame( kAutoControl, kNobodyControl, best.lv );
	scoreWindowVisible[0] = false;
	grayMonitorVisible[0] = false;

	levelCap = kLevels;
	if( (level < 1 || level > levelCap) && level != kTutorialLevel ) 
	{
        best = defaultBest;
        showStartMenu = true;
		return;
	}

	BeginRound( false );
	character[0].dropSpeed = 12;
	
	Gfx_AcquireSurface( backdropSurface );

    CC_RGBSurface* blackStripe = LoadPICTAsRGBSurface(picBlackStripe);
    if (blackStripe)
    {
        MRect backdropRect = { 0, 0, short(backdropSurface->h), short(backdropSurface->w) };
        SurfaceBlendOver(backdropSurface, &backdropRect, blackStripe, &backdropRect);
        Gfx_FreeSurface(blackStripe);
    }
    
	MRect comboRect = { (short)_HD(9), 0, (short)(_HD(9) + boldFont->h), 1920 };
	DrawCenteredText( boldFont, bestCombo, comboRect, 255, 255, 255, 4 );

    Gfx_ReleaseSurface( backdropSurface );
	
	memcpy( grid[0], best.grid, kGridAcross * kGridDown );
	ResolveSuction( 0 );
	RedrawBoardContents( 0 );
	RefreshAll( );

	nextA[0] = best.a;
	nextB[0] = best.b;
	nextG[0] = best.g;
	nextM[0] = best.m;
	RetrieveBlobs( 0 );

	EraseSpriteBlobs( 0 );
	blobX[0] = best.x;
	blobR[0] = best.r;
	DrawSpriteBlobs( 0, kNoSuction );
	
	Platform_FadeIn( );
	blobTime[0] = animTime[0] = GameTickCount( );

	autoPattern = hiScorePattern;	
	tutorialTime = 0;
    showStartMenu = false;
}
