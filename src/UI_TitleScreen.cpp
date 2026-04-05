///
///  UI_TitleScreen.cpp
///

#include "stdafx.h"
#include "UI_TitleScreen.h"
#include "MTypes.h"
#include "blitter.h"
#include "font.h"
#include "main.h"
#include "music.h"
#include "random.h"
#include "soundfx.h"
#include "SDLU.h"
#include "gameticks.h"
#include "zap.h"
#include "level.h"
#include "tutorial.h"
#include "hiscore.h"
#include "pause.h"
#include "Platform.h"
#include "UI_HighScoreScreen.h"
#include "UI_WorldSelectScreen.h"
#include "UI_PuppetShow.h"
#include <algorithm>


#define kNumSplats 16
#define kIdleSplat -2
#define kFallingSplat -1
#define kTitleItems 5
#define kIncrementPerFrame 2
#define kSplatType 4

using std::min;
using std::max;


const int startSkip = 1;
static MBoolean shouldFullRepaint = false;

const int kCursorWidth  = 72;
const int kCursorHeight = 72;

static void InsertCursor( MPoint mouseHere, CC_RGBSurface* scratch, CC_RGBSurface* surface )
{
    CC_Rect cursorBackSDLRect  = { 0, 0, kCursorWidth, kCursorHeight };
    CC_Rect cursorFrontSDLRect = { 0, 0, kCursorWidth, kCursorHeight };

    cursorFrontSDLRect.x = mouseHere.h;
    cursorFrontSDLRect.y = mouseHere.v;

    Gfx_BlitSurface( surface, &cursorFrontSDLRect,
                    scratch, &cursorBackSDLRect   );

    SurfaceBlitCursor( mouseHere, surface );
}

static void RemoveCursor( MPoint mouseHere, CC_RGBSurface* scratch, CC_RGBSurface* surface )
{
    CC_Rect      cursorBackSDLRect = { 0, 0, kCursorWidth, kCursorHeight };
    CC_Rect      cursorFrontSDLRect = { 0, 0, kCursorWidth, kCursorHeight };
    
    cursorFrontSDLRect.x = mouseHere.h;
    cursorFrontSDLRect.y = mouseHere.v;
    
    Gfx_BlitSurface( scratch, &cursorBackSDLRect,
                    surface, &cursorFrontSDLRect );
}

static void UI_TitleScreenRepaint()
{
    shouldFullRepaint = true;
}

void UI_ShowTitleScreen( void )
{
    Platform_SetPauseButtonVisible(false);
    g_iPadScreenLeftEdge = 204;
    Platform_NotifyScreenLeftEdge(g_iPadScreenLeftEdge);

    int startMenuTime = 0;
    int splatState[kNumSplats], splatColor[kNumSplats], splatSide[kNumSplats];
    MRect splatBlob[kNumSplats];
    static const MRect titleRect[kTitleItems] =
    {
        // measured from PICT_302.png @ 1920x1080
        { 370, 652, 456, 1268 },  // tutorial
        { 510, 652, 596, 1268 },  // 1p
        { 650, 652, 736, 1268 },  // 2p
        { 790, 652, 876, 1268 },  // solitaire
        { 930, 652, 1016, 1268 }, // high scores
    };
    // NOTE: be wary of initializing variables here! This function can run top-to-bottom
    // multiple times in a row, thanks to "redo". Put initializations after redo.
    CC_RGBSurface*  gameStartSurface;
    CC_RGBSurface*  gameStartDrawSurface;
    CC_RGBSurface*  cursorBackSurface;
    const CC_Rect   kBackdropSDLRect = { 0, 0, 1920, 1080 };
    CC_Rect         cursorBackSDLRect = { 0, 0, kCursorWidth, kCursorHeight };
    CC_Rect         destSDLRect;
    MRect           drawRect[4], chunkRect, tempRect;
    int             blob, count, oldGlow, splat, chunkType, selected;
    int             skip;
    MPoint          mouse;
    MPoint          dPoint;
    int             combo[2], comboBright[2], missBright[2];
    SkittlesFont*   smallFont = GetFont( picFont );
    const float     kComboScale = 0.75f;
    CC_Rect         meterRect[2] = { { 65, _HD(360), 250, _HD(20) }, { 1605, _HD(360), 250, _HD(20) } };
    int             titleGlow[kTitleItems];
    int             shouldAddBlob;
    
    const int       kLeftSide = 0, kRightSide = 1, kGlow = 2, kCursor = 3;
    
redo:
    
    combo[0] = combo[1] = 0;
    comboBright[0] = comboBright[1] = 0;
    missBright[0] = missBright[1] = 0;
    
    skip = 1;
    selected = -1;
    mouse.h = mouse.v = 0;
    
    if( finished ) return;
    
    if( musicSelection != 13 ) ChooseMusic( 13 );
    
    for( count=0; count<kTitleItems; count++ )
    {
        titleGlow[count] = 0;
    }
    
    for( count=0; count<kNumSplats; count++ )
    {
        splatState[count] = kIdleSplat;
    }
    
    // make background surface
    gameStartSurface     = LoadPICTAsRGBSurface( picGameStart );
    
    // make cursor backing store
    cursorBackSurface    = Gfx_InitRGBSurface( cursorBackSDLRect.w, cursorBackSDLRect.h );
    Gfx_FillRect( cursorBackSurface, &cursorBackSDLRect, {0, 0, 0} );
    
    // make drawing surface
    gameStartDrawSurface = Gfx_InitRGBSurface( kBackdropSDLRect.w, kBackdropSDLRect.h );
    Gfx_BlitSurface( gameStartSurface,     &gameStartSurface->clip_rect,
                    gameStartDrawSurface, &gameStartDrawSurface->clip_rect );
    
    Gfx_BlitFrontSurface( gameStartDrawSurface, &kBackdropSDLRect, &kBackdropSDLRect );
    
    SDLU_WaitForRelease();
    
    Platform_FadeIn( );
    
    DoFullRepaint = UI_TitleScreenRepaint;
    
    shouldAddBlob = 5;
    startMenuTime = MTickCount( );
    while( ( selected == -1 || !SDLU_Button() ) && !finished )
    {
        startMenuTime += skip;
        
        UpdateSound();
        
        // Add a new falling blob
        --shouldAddBlob;
        if (shouldAddBlob <= 0)
        {
            for( blob=0; blob<kNumSplats; blob++ )
            {
                if( splatState[blob] == kIdleSplat )
                {
                    splatSide[blob] = RandomBefore(2);
                    splatBlob[blob].top = -24 - RandomBefore(15);
                    splatBlob[blob].left = (splatSide[blob] == 0)? RandomBefore( 248 ): 1920 - kBlobHorizSize - RandomBefore( 248 );
                    splatBlob[blob].bottom = splatBlob[blob].top + kBlobVertSize;
                    splatBlob[blob].right = splatBlob[blob].left + kBlobHorizSize;
                    splatColor[blob] = ((startMenuTime >> 2) % kBlobTypes) + 1;
                    splatState[blob] = kFallingSplat;
                    
                    break;
                }
            }
            shouldAddBlob = 5;
        }
        
        // Erase and redraw falling blobs and chunks
        
        Gfx_AcquireSurface( gameStartDrawSurface );
        
        // Take the cursor out of the scene
        RemoveCursor( mouse, cursorBackSurface, gameStartDrawSurface );
        drawRect[kCursor].top    = mouse.v;
        drawRect[kCursor].left   = mouse.h;
        drawRect[kCursor].bottom = mouse.v + kCursorHeight;
        drawRect[kCursor].right  = mouse.h + kCursorWidth;
        
        // Inverted rectangles mean "nothing to do."
        drawRect[kLeftSide].top    = drawRect[kRightSide].top    = drawRect[kGlow].top    =
        drawRect[kLeftSide].left   = drawRect[kRightSide].left   = drawRect[kGlow].left   = 9999;
        drawRect[kLeftSide].bottom = drawRect[kRightSide].bottom = drawRect[kGlow].bottom =
        drawRect[kLeftSide].right  = drawRect[kRightSide].right  = drawRect[kGlow].right  = -9999;
        
        // Get cursor position
        SDLU_GetMouse( &mouse );
        if( mouse.v > 1035 ) mouse.v = 1035;
        
        // Erase falling blobs
        for( blob=0; blob<kNumSplats; blob++ )
        {
            if( splatState[blob] == kFallingSplat )
            {
                Gfx_FillRect( gameStartDrawSurface, Gfx_MRectToCCRect( &splatBlob[blob], &destSDLRect ), {0, 0, 0} );
                UnionMRect( &drawRect[splatSide[blob]], &splatBlob[blob], &drawRect[splatSide[blob]] );
                
                OffsetMRect( &splatBlob[blob], 0, startSkip * (6 + (splatBlob[blob].bottom / 20)) );
            }
            else if( splatState[blob] >= kIncrementPerFrame )
            {
                for( splat=-3; splat<=3; splat++ )
                {
                    if( splat )
                    {
                        chunkRect = splatBlob[blob];
                        GetZapStyle( 0, &chunkRect, &splatColor[blob], &chunkType, splat, splatState[blob]-kIncrementPerFrame, kSplatType );
                        Gfx_FillRect( gameStartDrawSurface, Gfx_MRectToCCRect( &chunkRect, &destSDLRect ), {0, 0, 0} );
                        UnionMRect( &drawRect[splatSide[blob]], &chunkRect, &drawRect[splatSide[blob]] );
                    }
                }
                
                Gfx_FillRect( gameStartDrawSurface, Gfx_MRectToCCRect( &splatBlob[blob], &destSDLRect ), {0, 0, 0} );
                UnionMRect( &drawRect[splatSide[blob]], &splatBlob[blob], &drawRect[splatSide[blob]] );
            }
        }
        
        // Draw combo meters
        
        for( count=0; count<2; count++ )
        {
            int bright = comboBright[count];
            int mBright = missBright[count];
            if( bright || mBright )
            {
                Gfx_FillRect( gameStartDrawSurface, &meterRect[count], {0, 0, 0} );
                UnionMRect( &drawRect[count], Gfx_CCRectToMRect( &meterRect[count], &tempRect ), &drawRect[count] );
                
                if( mBright > 1 )
                {
                    dPoint.v = meterRect[count].y;
                    dPoint.h = meterRect[count].x + 10;
                    SurfaceBlitCharacter( smallFont, 'M', &dPoint, mBright, mBright >> 2, mBright >> 2, 1 );
                    SurfaceBlitCharacter( smallFont, 'I', &dPoint, mBright, mBright >> 2, mBright >> 2, 1 );
                    SurfaceBlitCharacter( smallFont, 'S', &dPoint, mBright, mBright >> 2, mBright >> 2, 1 );
                    SurfaceBlitCharacter( smallFont, 'S', &dPoint, mBright, mBright >> 2, mBright >> 2, 1 );
                    missBright[count] -= 8;
                }
                else if( (combo[count] >= 10) && (bright > 1) )
                {
                    char  number[16] = { 0 };
                    char* scan;
                    snprintf( number, sizeof(number), "%d", combo[count] );
                    
                    float fdx = (float)meterRect[count].x;
                    float fdy = (float)meterRect[count].y + 3;
                    BlitString( smallFont, "COMBO ", &fdx, (int)fdy, bright, bright, bright, bright, kComboScale );
                    dPoint.h = (short)fdx;
                    dPoint.v = (short)(fdy - 3);
                    
                    for( scan = number; *scan; scan++ )
                    {
                        SurfaceBlitCharacter( smallFont, *scan, &dPoint, bright>>2, bright>>2, bright, 1 );
                    }
                    
                    comboBright[count] -= 16;
                }
                else
                {
                    comboBright[count] = 0;
                }
            }
        }
        
        // Redraw falling blobs
        for( blob=0; blob<kNumSplats; blob++ )
        {
            if( splatState[blob] == kFallingSplat )
            {
                if( splatBlob[blob].bottom >= 1080 )
                {
                    splatBlob[blob].top = 1080 - kBlobVertSize;
                    splatBlob[blob].bottom = 1080;
                    splatState[blob] = 1;
                    
                    // Process combos
                    if( mouse.v > 945 &&
                       mouse.h >= (splatBlob[blob].left - 68) &&
                       mouse.h <= (splatBlob[blob].right + 22)    )
                    {
                        combo[splatSide[blob]]++;
                        comboBright[splatSide[blob]] = 255;
                    }
                    else
                    {
                        if( combo[splatSide[blob]] >= 10 ) missBright[splatSide[blob]] = 255;
                        combo[splatSide[blob]] = 0;
                        comboBright[splatSide[blob]] = 0;
                    }
                }
                else
                {
                    SurfaceDrawSprite( &splatBlob[blob], splatColor[blob], kNoSuction );
                    UnionMRect( &drawRect[splatSide[blob]], &splatBlob[blob], &drawRect[splatSide[blob]] );
                }
            }
            
            if( splatState[blob] >= 0 && splatState[blob] <= kZapFrames )
            {
                if( splatState[blob] <= (kZapFrames - kIncrementPerFrame) )
                {
                    chunkType = 0;
                    
                    for( splat=-3; splat<=3; splat++ )
                    {
                        if( splat )
                        {
                            chunkRect = splatBlob[blob];
                            GetZapStyle( 0, &chunkRect, &splatColor[blob], &chunkType, splat, splatState[blob], kSplatType );
                            SurfaceDrawSprite( &chunkRect, splatColor[blob], chunkType );
                            UnionMRect( &drawRect[splatSide[blob]], &chunkRect, &drawRect[splatSide[blob]] );
                        }
                    }
                    
                    SurfaceDrawSprite( &splatBlob[blob], splatColor[blob], chunkType );
                    UnionMRect( &drawRect[splatSide[blob]], &splatBlob[blob], &drawRect[splatSide[blob]] );
                }
                
                splatState[blob] += kIncrementPerFrame;
                if( splatState[blob] > kZapFrames ) splatState[blob] = kIdleSplat;
            }
        }
        
        Gfx_ReleaseSurface( gameStartDrawSurface );
        
        // Find mouse coords
        
        selected = -1;
        for( count=0; count<kTitleItems; count++ )
        {
            if( MPointInMRect( mouse, &titleRect[count] ) )
            {
                selected = count;
                break;
            }
        }
        
        // update glows
        for (int glowUpdate=0; glowUpdate < kTitleItems; ++glowUpdate)
        {
            oldGlow = titleGlow[glowUpdate];
            
            if( selected == glowUpdate )
            {
                titleGlow[glowUpdate] += (40 * startSkip);
                if( titleGlow[glowUpdate] > 255 ) titleGlow[glowUpdate] = 255;
            }
            else
            {
                titleGlow[glowUpdate] -= (40 * startSkip);
                if( titleGlow[glowUpdate] < 0 ) titleGlow[glowUpdate] = 0;
            }
            
            if( titleGlow[glowUpdate] != oldGlow )
            {
                MRect clearRect = titleRect[glowUpdate];
                InflateMRect(&clearRect, 25, 15);
                Gfx_BlitSurface(gameStartSurface, Gfx_MRectToCCRect(&clearRect, &destSDLRect), gameStartDrawSurface, &destSDLRect);

                if (titleGlow[glowUpdate] > 0)
                    SurfaceDrawHighlightedButton(gameStartSurface, gameStartDrawSurface, titleRect[glowUpdate], titleGlow[glowUpdate], 195);
                
                UnionMRect(&drawRect[kGlow], &clearRect, &drawRect[kGlow]);
            }
        }
        
        // Reinsert the cursor into the scene
        InsertCursor( mouse, cursorBackSurface, gameStartDrawSurface );
        drawRect[kCursor].top    = min<short>( drawRect[kCursor].top,    mouse.v );
        drawRect[kCursor].left   = min<short>( drawRect[kCursor].left,   mouse.h );
        drawRect[kCursor].bottom = max<short>( drawRect[kCursor].bottom, mouse.v + kCursorHeight );
        drawRect[kCursor].right  = max<short>( drawRect[kCursor].right,  mouse.h + kCursorWidth );
        
        // Copy down everything		
        if( shouldFullRepaint )
        {
            Gfx_BlitFrontSurface( gameStartDrawSurface, &gameStartDrawSurface->clip_rect, &gameStartDrawSurface->clip_rect );
            shouldFullRepaint = false;
        }
        else
        {
            for( count=0; count<4; count++ )
            {		
                if( drawRect[count].left < drawRect[count].right )
                {
                    Gfx_MRectToCCRect( &drawRect[count], &destSDLRect );			
                    Gfx_BlitFrontSurface( gameStartDrawSurface, &destSDLRect, &destSDLRect );
                }
            }
        }
        
        SDLU_Present();
        
        // Skip frames? Or delay?
        if( startMenuTime <= MTickCount( ) )
        {
            startMenuTime = MTickCount( );
            skip = 2;
        }
        else
        {
            skip = 1;
            while( startMenuTime > MTickCount( ) )
            {
                SDLU_Yield();
            }
        }

        // On touch screens the SDL cursor jumps to the touch position and the
        // mouse button goes down in the same event-pump, but `selected` was set
        // by the hit-test earlier in this frame.  If the button is not held at
        // the end of this frame, clear `selected` so the while condition above
        // re-evaluates it against the *new* cursor position next frame instead
        // of the stale one — preventing false button clicks.
        if( !SDLU_Button() ) selected = -1;
    }
    
    DoFullRepaint = nullptr;
    
    if( finished ) return;

    switch( selected )
    {
        case 0:
        case 2:
        case 3:
            PlayMono( kChime );
            break;
        case 1:
            PlayStereoFrequency( 2, kChime, -0.666666 );
            break;
    }
    
    Gfx_FreeSurface( gameStartSurface );
    Gfx_FreeSurface( gameStartDrawSurface );
    Gfx_FreeSurface( cursorBackSurface );
    
    Platform_FadeOut( );
    
    switch( selected )
    {
        case 0: 
            InitGame( kAutoControl, kNobodyControl, kTutorialLevel );
            level = kTutorialLevel;
            BeginRound( true );
            InitTutorial( );
            Platform_FadeIn( );
            break;				
            
        case 1:
            if (UI_ShowWorldSelectScreen())
            {
                // Fade-in is handled inside UI_ShowPuppetShowForLevel's transition.
                break;  // game started; exit title screen, let main loop run
            }
            goto redo;

        case 2:
        {
            InitGame( kPlayerControl, kPlayerControl, 1 );
            BeginRound( true );
            Platform_FadeIn( );
            break;
        }

        case 3:
        {
            // Solitaire: start at level 1 with a puppet show intro.
            // Fade-in is handled inside UI_ShowPuppetShowForLevel's transition.
            InitGame( kPlayerControl, kNobodyControl, 1 );
            UI_ShowPuppetShowForLevel( level, e_AfterPuppetShow_StartGame );
            break;
        }
            
        case 4: 
            UI_ShowHighScoreScreen();
            ShowBestCombo();
            break;
            
    }
}
