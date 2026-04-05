///
///  UI_HighScoreScreen.cpp
///  

#include "stdafx.h"
#include "UI_HighScoreScreen.h"
#include "SDLU.h"
#include "gameticks.h"
#include "gworld.h"
#include "blitter.h"
#include "MTypes.h"
#include "hiscore.h"
#include "random.h"
#include "level.h"
#include <algorithm>
#include <string.h>

using std::min;

static void FadeScreen( CC_RGBSurface* hiScoreSurface, CC_RGBSurface* fadeSurface, int start, int end )
{
    int       skip, timer, frame, fade, color, direction, fadeStart, fadeEnd;
    CC_Rect   destSDLRect;
    CC_Rect   fullSDLRect = { 0, 0, 1920, 1080 };
    
    if( start < end )
    {
        direction = 1;
        fadeStart = 0;
        fadeEnd = 32;
    }
    else
    {
        direction = -1;
        fadeStart = 32;
        fadeEnd = 0;
    }
    
    skip = 1;
    timer = MTickCount( ) + 1;
    while( timer > MTickCount() ) { SDLU_Yield(); }
    
    for( frame = start; (direction>0)? (frame <= end): (frame >= end); frame += direction )
    {
        MRect drawRect = {0, 0, _HD(16), 1920};
        timer += skip;
        
        for( fade = fadeStart; fade != fadeEnd; fade += direction )
        {
            color = frame + fade;
            if( color <  0 ) color = 0;
            if( color > 31 ) color = 31;
            
            Gfx_MRectToCCRect( &drawRect, &destSDLRect );
            
            switch( color )
            {
                case 0:
                    Gfx_BlitSurface( hiScoreSurface, &destSDLRect,
                                    fadeSurface,    &destSDLRect  );
                    break;
                    
                case 31:
                    Gfx_FillRect( fadeSurface, &destSDLRect, {0, 0, 0} );
                    break;
                    
                default:
                    SurfaceBlitColorOver( hiScoreSurface,  fadeSurface,
                                         &drawRect,       &drawRect,
                                         0, 0, 0, _5TO8(color) );
                    break;
            }
            
            OffsetMRect( &drawRect, 0, _HD(16) );
        }
        
        Gfx_BlitFrontSurface( fadeSurface, &fullSDLRect, &fullSDLRect );
        SDLU_Present();
        
        if( timer <= MTickCount( ) )
        {
            skip = 2;
        }
        else
        {
            skip = 1;
            while( timer > MTickCount( ) )
            {
                SDLU_Yield();
            }
        }
    }
}

void UI_ShowHighScoreScreen()
{
    short            count, length;
    char             myString[256];
    int              stringLength;
    CC_RGBSurface*   hiScoreSurface;
    CC_RGBSurface*   fadeSurface;
    CC_Rect          fullSDLRect = { 0, 0, 1920, 1080 };
    SkittlesFont*    font;
    SkittlesFont*    boldFont;
    CC_RGBPixel      anyColor;
    MPoint           dPoint;
    constexpr const char*  kHighScores = "HIGH SCORES";
    int              r, g, b;
    
    if (SDLU_DeleteKeyIsPressed())
    {
        // If the user holds delete while opening the high scores,
        // clear the high score table.
        
        memcpy( &scores, &defaultScores, sizeof( scores ) );
        memcpy( &best,   &defaultBest,   sizeof( best   ) );
    }
    
    fadeSurface    = Gfx_InitRGBSurface( fullSDLRect.w, fullSDLRect.h );
    hiScoreSurface = Gfx_InitRGBSurface( fullSDLRect.w, fullSDLRect.h );
    
    DrawPICTInSurface(hiScoreSurface, 5000 + RandomBefore(kLevels), "/Backdrop");
    
    CC_RGBSurface* blackStripe = LoadPICTAsRGBSurface(picBlackStripe);
    if (blackStripe)
    {
        MRect backdropRect = { 0, 0, short(hiScoreSurface->h), short(hiScoreSurface->w) };
        SurfaceBlendOver(hiScoreSurface, &backdropRect, blackStripe, &backdropRect);
        Gfx_FreeSurface(blackStripe);
    }
    
    font = GetFont( picSkiaFont );
    boldFont = GetFont( picSkiaBoldFont );
    
    Gfx_AcquireSurface( hiScoreSurface );
    
    Gfx_GetPixel( hiScoreSurface, RandomBefore( fullSDLRect.w ), RandomBefore( fullSDLRect.h ), &anyColor );
    
    anyColor.r = min( 255, anyColor.r + 112 );
    anyColor.g = min( 255, anyColor.g + 112 );
    anyColor.b = min( 255, anyColor.b + 112 );
    
    dPoint.v = _HD(9);
	dPoint.h = _HDW(320) - (GetTextWidth( boldFont, kHighScores ) / 2);
    for( count=0; kHighScores[count]; count++ )
    {
        SurfaceBlitCharacter( boldFont, kHighScores[count], &dPoint, 255, 255, 255, 4 );
    }
    
    for( count=0; count<=9; count++ )
    {
        r = ((255 * (10-count)) + (anyColor.r * count)) / 10;
        g = ((255 * (10-count)) + (anyColor.g * count)) / 10;
        b = ((255 * (10-count)) + (anyColor.b * count)) / 10;
        
        dPoint.v = _HD(70) + (count * _HD(38));
        dPoint.h = _HDW(85);
        
        if( count<9 )
        {
            SurfaceBlitCharacter( font, count + '1', &dPoint, r, g, b, 4 );
        }
        else
        {
            SurfaceBlitCharacter( font, '1', &dPoint, r, g, b, 4 );
            SurfaceBlitCharacter( font, '0', &dPoint, r, g, b, 4 );
        }
        
        SurfaceBlitCharacter( font, '.', &dPoint, r, g, b, 4 );
        SurfaceBlitCharacter( font, ' ', &dPoint, r, g, b, 4 );
        
        length = 0;
        while( scores[count].name[length] && dPoint.h < _HDW(430) )
        {
            SurfaceBlitCharacter( font, scores[count].name[length++], &dPoint, r, g, b, 4 );
        }
        
        while( dPoint.h < _HDW(450) )
        {
            SurfaceBlitCharacter( font, '.', &dPoint, r, g, b, 4 );
        }
        
        dPoint.h = _HDW(470);
        
        stringLength = snprintf( myString, sizeof(myString), "%d", scores[count].score );
        for( length=0; length < stringLength; length++ )
        {
            SurfaceBlitCharacter( font, myString[length], &dPoint, r, g, b, 4 );
        }
    }
    
    Gfx_ReleaseSurface( hiScoreSurface );
    
    Gfx_FillRect( g_frontSurface, &g_frontSurface->clip_rect, {0, 0, 0});
    SDLU_Present();
    
    FadeScreen( hiScoreSurface, fadeSurface, 31, -32 );
    do
    {
        SDLU_Yield();
    }
    while( !SDLU_AnyKeyIsPressed( ) && !SDLU_Button() );
    FadeScreen( hiScoreSurface, fadeSurface, -31, 32 );

    Gfx_FreeSurface( hiScoreSurface );
    Gfx_FreeSurface( fadeSurface );
}
