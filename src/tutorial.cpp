// tutorial.c

#include "stdafx.h"
#include <algorithm>

#include "main.h"
#include "tutorial.h"
#include "level.h"
#include "font.h"
#include "pause.h"
#include "gworld.h"
#include "graphics.h"
#include "control.h"
#include "blitter.h"
#include "gameticks.h"
#include "soundfx.h"
#include "opponent.h"
#include "Input.h"
#include "Platform.h"

#include <string.h>
#include <stdio.h>

extern AutoPattern  tutorialPattern[];
extern AutoPattern  touchTutorialPattern[];
MRect               balloonRect = {0, 0, _HD(170), _HDW(100)};
SkittlesFont*       balloonFont;
MPoint              balloonPt;
char*               balloonChar;
char                balloonMsg[256];
int                 balloonTime, tutorialTime;
CC_RGBSurface*      balloonSurface = NULL;

void InitTutorial( void )
{
	// Balloon font
	balloonFont = GetFont( picFont );
	
	// Balloon backbuffer
	if( balloonSurface == NULL )
	{
		balloonSurface = Gfx_InitRGBSurface(backdropSurface->w, backdropSurface->h);
	}
	
	// Set up auto pattern
	autoPattern = Platform_IsTouchDevice() ? touchTutorialPattern : tutorialPattern;
	tutorialTime = 0;
}

void EndTutorial( void )
{
	Platform_FadeOut();
	
	showStartMenu = true;
}

static int CalculateBalloonWidth( char *message )
{
	int maxWidth = _HD(40);
	int currentWidth = 0;
	
	for( ;; )
	{
		char in = *message++;
		
		switch(in)
		{
			case '\0':
                return std::max<>(currentWidth, maxWidth);
				
			case '\n':
                maxWidth = std::max<>(currentWidth, maxWidth);
				currentWidth = 0;
				break;
			
			default:
				currentWidth += balloonFont->width[in];
				break;
		}
	}
}

static int CalculateBalloonHeight( char *message )
{
	int lines = 2;
    for (char *scan = message; *scan; ++scan)
    {
        lines += (*scan == '\n');
    }
    
    lines *= 20;
    
    return _HD(lines);
}

void StopBalloon( void )
{
	balloonTime = 0x7FFFFFFF;
}

void StartBalloon( const char *message )
{
	MPoint      balloonTip, balloonFill;
	int         replace;
	const char* match[] = { "~~", "||", "``", "{{" };
	char*       search;
	CC_Rect     balloonSDLRect, balloonContentsSDLRect;
	MRect       balloonContentsRect;
	
	strlcpy( balloonMsg, message, arrsize(balloonMsg) );
	for( replace=0; replace<4; replace++ )
	{
		search = strstr( balloonMsg, match[replace] );
		if( search )
		{
			char temp[256];
			
			search[0] = '%';
			search[1] = 's';
			snprintf( temp, arrsize(temp), balloonMsg, Platform_GetKeyName( playerKeys[1][replace] ) );
			strlcpy( balloonMsg, temp, arrsize(balloonMsg) );
		}
	}
	
	// Erase previous balloons
	Gfx_MRectToCCRect( &balloonRect, &balloonSDLRect );
	Gfx_BlitFrontSurface( backdropSurface, &balloonSDLRect, &balloonSDLRect );

	// Draw empty balloon outline
	Gfx_AcquireSurface( balloonSurface );

	balloonRect.left = balloonRect.right - _HD(25) - CalculateBalloonWidth ( balloonMsg );
	balloonRect.top = balloonRect.bottom - _HD(25) - CalculateBalloonHeight( balloonMsg );

	Gfx_MRectToCCRect( &balloonRect, &balloonSDLRect );
	Gfx_BlitSurface( backdropSurface, &balloonSDLRect,
	                  balloonSurface,  &balloonSDLRect  );
	
	balloonContentsRect = balloonRect;
	balloonContentsRect.bottom -= _HD(25);
    
    SurfaceEdgeData edgeData;
    
	SurfaceGetEdges( balloonSurface, &balloonContentsRect, edgeData );
	Gfx_FillRect( balloonSurface,
			 	   Gfx_MRectToCCRect( &balloonContentsRect, &balloonContentsSDLRect ),
                   { 0xFF, 0xFF, 0xFF } );
	SurfaceCurveEdges( balloonSurface, &balloonContentsRect, edgeData );

	balloonTip.v = balloonContentsRect.bottom - _HD(1);
	balloonTip.h = balloonContentsRect.right - _HD(40);
	balloonFill = balloonTip;

	SurfaceBlitCharacter( balloonFont, '\x06', &balloonFill,  0,   0,   0,  0 );
	SurfaceBlitCharacter( balloonFont, '\x07', &balloonTip, 255, 255, 255,  0 );

	Gfx_ReleaseSurface( balloonSurface );

	// Blit empty balloon to screen
	Gfx_MRectToCCRect( &balloonRect, &balloonSDLRect );
	Gfx_BlitFrontSurface( balloonSurface, &balloonSDLRect, &balloonSDLRect );
	
    balloonPt.h = balloonRect.left + _HD(10);
	balloonPt.v = balloonRect.top + _HD(10);
	balloonChar = balloonMsg;
	balloonTime = GameTickCount( );
	
	OpponentChatter( true );
}

void UpdateBalloon( void )
{
	CC_Rect balloonSDLRect;
	
	if( control[0] != kAutoControl ) return;
	if( GameTickCount() < balloonTime ) return;
	
	if( balloonChar )
	{
		char in = *balloonChar++;
				
		switch( in )
		{
			case 0:
				OpponentChatter( false );
				balloonChar = NULL;
				balloonTime += 120;
				break;
				
			case '\n':
				balloonPt.h = balloonRect.left + _HD(10);
				balloonPt.v += _HD(20);
				break;
				
			default:
				if( balloonFont->width[in] > 0 )
				{
					Gfx_AcquireSurface( balloonSurface );
					SurfaceBlitCharacter( balloonFont, in, &balloonPt, 0, 0, 0, 0 );
					Gfx_ReleaseSurface( balloonSurface );
					
					Gfx_MRectToCCRect( &balloonRect, &balloonSDLRect );
					Gfx_BlitFrontSurface( balloonSurface, &balloonSDLRect, &balloonSDLRect );

					balloonTime += 2;
				}
				break;			
		}	
	}
	else
	{
		Gfx_MRectToCCRect( &balloonRect, &balloonSDLRect );
		Gfx_BlitFrontSurface( backdropSurface, &balloonSDLRect, &balloonSDLRect );
		                    
		StopBalloon();
	}
}
