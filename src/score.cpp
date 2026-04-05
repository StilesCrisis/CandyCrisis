// score.c

#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <initializer_list>

#include "main.h"
#include "score.h"
#include "gworld.h"
#include "graphics.h"
#include "blitter.h"
#include "SDLU.h"
#include "hiscore.h"
#include "gameticks.h"
#include "level.h"


const int kNumberWidth       = 49;
const int kNumberKernedWidth = 37;
const int kNumberHeight      = 72;


static CC_RGBSurface*   s_scoreDrawSurface;
static CC_RGBSurface*   s_numberOuterSurface;
static CC_RGBSurface*   s_numberInnerSurface;

CC_RGBSurface*          scoreSurface[2];
MRect           scoreWindowZRect, scoreWindowRect[2];
MBoolean        scoreWindowVisible[2] = {true, true};
int             roundStartScore[2], score[2], displayedScore[2], scoreTime[2];

void InitScore( void )
{
    scoreWindowZRect = { 0, 0, kNumberHeight, kBlobHorizSize * kGridAcross };
	
	scoreWindowRect[0] = scoreWindowRect[1] = scoreWindowZRect;
	CenterRectOnScreen( &scoreWindowRect[0], kLeftPlayerWindowCenter,  kScoreWindowVertCenter );
	CenterRectOnScreen( &scoreWindowRect[1], kRightPlayerWindowCenter, kScoreWindowVertCenter );
	
    s_scoreDrawSurface = Gfx_InitRGBSurface(scoreWindowZRect.right, scoreWindowZRect.bottom);
    scoreSurface[0]    = Gfx_InitRGBSurface(scoreWindowZRect.right, scoreWindowZRect.bottom);
    scoreSurface[1]    = Gfx_InitRGBSurface(scoreWindowZRect.right, scoreWindowZRect.bottom);
	
	displayedScore[0] = displayedScore[1] = 0;
	score[0]          = score[1]          = 0;
	scoreTime[0]      = scoreTime[1]      = 0;
}

// Bake src masked by mask into a new BLEND surface using GPU compositing.
static CC_RGBSurface* BakeMaskedSurface(CC_RGBSurface* src, CC_MaskSurface* mask)
{
    CC_RGBSurface* result = Gfx_InitRGBSurface(src->w, src->h);
    Gfx_AcquireSurface(result);
    // Copy src into result (BLEND_NONE overwrites every pixel regardless of alpha)
    Gfx_RenderCopyBlend(src->texture, SDL_BLENDMODE_NONE, nullptr, nullptr);
    // Multiply result alpha by mask alpha: visible where mask=255, transparent where mask=0
    Gfx_RenderCopyBlend(mask->texture, Gfx_AlphaMultiplyBlendMode(), nullptr, nullptr);
    Gfx_ReleaseSurface(result);
    return result;
}

void PrepareStageGraphics_Score()
{
    if (s_numberOuterSurface) { Gfx_FreeSurface(s_numberOuterSurface); s_numberOuterSurface = nullptr; }
    if (s_numberInnerSurface) { Gfx_FreeSurface(s_numberInnerSurface); s_numberInnerSurface = nullptr; }

    CC_RGBSurface*  numberSurface = LoadPICTAsRGBSurface(backgroundID, "/Score");
    CC_MaskSurface* outerMask     = LoadPICTAsMaskSurface(backgroundID, "/ScoreOuterMask");
    CC_MaskSurface* innerMask     = LoadPICTAsMaskSurface(backgroundID, "/ScoreInnerMask");

    s_numberOuterSurface = BakeMaskedSurface(numberSurface, outerMask);
    s_numberInnerSurface = BakeMaskedSurface(numberSurface, innerMask);

    Gfx_FreeSurface(numberSurface);
    Gfx_FreeSurface(outerMask);
    Gfx_FreeSurface(innerMask);

    // Load the score surfaces and alpha-blend them.
    DrawPICTInSurface(scoreSurface[0], backgroundID, "/BoardScore");
    Gfx_BlitSurface(scoreSurface[0], &scoreSurface[0]->clip_rect, scoreSurface[1], &scoreSurface[1]->clip_rect);
    DrawPICTInSurface(scoreSurface[1], backgroundID, "/BoardScore-R");
    
    SurfaceBlendUnder(scoreSurface[0], &scoreWindowZRect, backdropSurface, &scoreWindowRect[0]);
    SurfaceBlendUnder(scoreSurface[1], &scoreWindowZRect, backdropSurface, &scoreWindowRect[1]);
}

void UpdateScore( int player )
{
	if( GameTickCount( ) >= scoreTime[player] )
	{		
		scoreTime[player] = GameTickCount() + 1;
		
		if( displayedScore[player] < score[player] )
		{
			if( (score[player] - displayedScore[player]) > 5000 )
			{
				displayedScore[player] += 1525;
			}
			else if( (score[player] - displayedScore[player]) > 1000 )
			{
				displayedScore[player] += 175;
			}
			else
			{
				displayedScore[player] += 25;
			}
			
			if( displayedScore[player] > score[player] )
				displayedScore[player] = score[player];
			
			ShowScore( player );
		}
	}
}


static void DrawCharacter(CC_RGBSurface* bakedSurface, char which, const MRect *myRect)
{
    static const char characterLookup[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C' };

    for (int count = 0; count < arrsize(characterLookup); count++)
    {
        if (characterLookup[count] == which)
        {
            SDL_Rect srcR = { count * kNumberWidth, 0, kNumberWidth, kNumberHeight };
            SDL_Rect dstR = { myRect->left, myRect->top,
                              myRect->right - myRect->left, myRect->bottom - myRect->top };
            SDL_RenderCopy(g_renderer, bakedSurface->texture, &srcR, &dstR);
            break;
        }
    }
}

void ShowScore( int player )
{
	CC_Rect    sourceSDLRect, destSDLRect;
	MRect      myRect;
	char       myString[256];
	int        count;
	
	if( !scoreWindowVisible[player] ) return;
	
	if( control[player] != kNobodyControl )
	{
		Gfx_AcquireSurface( s_scoreDrawSurface );
		
		Gfx_BlitSurface( scoreSurface[player], &s_scoreDrawSurface->clip_rect,
				 		 s_scoreDrawSurface,   &s_scoreDrawSurface->clip_rect   );
		
        myRect = { 0, 5, kNumberHeight, kNumberWidth + 5 };
		DrawCharacter(s_numberOuterSurface, 'A',   &myRect);
        OffsetMRect(&myRect, kNumberWidth, 0);
        DrawCharacter(s_numberOuterSurface, 'B',   &myRect);
        OffsetMRect(&myRect, kNumberWidth, 0);
        DrawCharacter(s_numberOuterSurface, 'C',   &myRect);

        snprintf(myString, arrsize(myString), "%d", displayedScore[player]);

        for (CC_RGBSurface* surf: {s_numberOuterSurface, s_numberInnerSurface})
        {
            myRect = scoreWindowZRect;
            myRect.right -= kNumberKernedWidth / 8;
            myRect.left = myRect.right - kNumberWidth;

            for( count = int(strlen(myString)) - 1; count >= 0; count-- )
            {
                DrawCharacter(surf, myString[count], &myRect);
                OffsetMRect(&myRect, -kNumberKernedWidth, 0);
            }
        }
        
		Gfx_ReleaseSurface( s_scoreDrawSurface );
		
		Gfx_BlitFrontSurface( s_scoreDrawSurface, 
		                       Gfx_MRectToCCRect( &scoreWindowZRect, &sourceSDLRect ),
		                       Gfx_MRectToCCRect( &scoreWindowRect[player], &destSDLRect ) );
	}
}
