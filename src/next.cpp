// next.c

#include "stdafx.h"

#include "main.h"
#include "next.h"
#include "graphics.h"
#include "gworld.h"
#include "gameticks.h"
#include "random.h"
#include "blitter.h"
#include "level.h"

#define kJiggleFrames 8
#define kPulling 10
#define kPullingFrames 18

CC_RGBSurface* nextSurface[2];
CC_RGBSurface* nextDrawSurface;

MRect nextWindowZRect, nextWindowRect[2];
MBoolean nextWindowVisible[2] = {true, true};
int nextTime[2][2], nextStage[2][2], pullA[2], pullB[2];

void InitNext( void )
{
    nextWindowZRect = { 0, 0, 256, 96 };
    
    nextWindowRect[0] = nextWindowRect[1] = nextWindowZRect;
    CenterRectOnScreen( &nextWindowRect[0], 0.38, kNextWindowVertCenter2P );
    CenterRectOnScreen( &nextWindowRect[1], 0.62, kNextWindowVertCenter2P );
    
    nextSurface[0]  = Gfx_InitRGBSurface(nextWindowZRect.right, nextWindowZRect.bottom);
    nextSurface[1]  = Gfx_InitRGBSurface(nextWindowZRect.right, nextWindowZRect.bottom);
    nextDrawSurface = Gfx_InitRGBSurface(nextWindowZRect.right, nextWindowZRect.bottom);
}

void RefreshNext( int player )
{
	nextStage[player][0] = 0;
    nextStage[player][1] = 0;

	nextTime[player][0] = GameTickCount( ) + RandomBefore( 60 );
	nextTime[player][1] = GameTickCount( ) + RandomBefore( 60 );

	ShowNext( player );
}

void PullNext( int player )
{
	pullA[player] = nextA[player];
	pullB[player] = nextB[player];
	nextStage[player][0] = kPulling;
	nextTime[player][0] = GameTickCount( );
}

void ShowPull( int player )
{
	MRect       srcRect;
    int         frame = nextStage[player][0] - kPulling;

    const int   drawA[2] = { pullA[player], nextA[player] };
    const int   drawB[2] = { pullB[player], nextB[player] };
    const float offset[2] =
    {
        (-5.78571f * frame + 2.57143f) * frame + 94.2858f,
        ((-0.75f * frame + 14.1071f) * frame - 91.6428f) * frame + 309.429f,
    };
    
	CC_Rect sourceSDLRect, destSDLRect;
	
	if( !nextWindowVisible[player] ) return;
	
	Gfx_AcquireSurface( nextDrawSurface );
	
	Gfx_BlitSurface( nextSurface[player],   &nextSurface[player]->clip_rect,
				     nextDrawSurface,       &nextDrawSurface->clip_rect );
	
    for (int count=0; count<2; count++)
	{
        MRect blobRect = { 0, 3*(4), kBlobVertSize, 3*(4) + kBlobHorizSize };
        MRect shadowRect = { 3*(4), 3*(8), 3*(4) + kBlobVertSize, 3*(8) + kBlobHorizSize };

        OffsetMRect( &blobRect, 0, offset[count] );
        OffsetMRect( &shadowRect, 0, offset[count] );
                
        SurfaceDrawShadow( &shadowRect, drawB[count], kNoSuction );
        
        CalcBlobRect( kNoSuction, drawB[count]-1, &srcRect );
        SurfaceBlitBlob( &srcRect, &blobRect );	  
                  
        OffsetMRect( &blobRect, 0, kBlobVertSize );
        OffsetMRect( &shadowRect, 0, kBlobVertSize );
        
        SurfaceDrawShadow( &shadowRect, drawA[count], nextM[player]? kFlashDarkBlob: kNoSuction );

        CalcBlobRect( nextM[player]? kFlashDarkBlob: kNoSuction, drawA[count]-1, &srcRect );
        SurfaceBlitBlob( &srcRect, &blobRect );	  
	}
	
	Gfx_ReleaseSurface( nextDrawSurface );
	
	Gfx_BlitFrontSurface( nextDrawSurface, 
	                      Gfx_MRectToCCRect( &nextWindowZRect, &sourceSDLRect ),
	                      Gfx_MRectToCCRect( &nextWindowRect[player], &destSDLRect ) );
}

void UpdateNext( int player )
{
	MBoolean changed = false;
	int blob;
	
	if( nextStage[player][0] >= kPulling )
	{
		if( GameTickCount() > nextTime[player][0] )
		{
			if( ++nextStage[player][0] >= kPullingFrames )
			{
				RefreshNext( player );
			}
			else
			{
				ShowPull( player );
				nextTime[player][0]++;
			}
		}
	} 
	else
	{
		for( blob=0; blob<2; blob++ )
		{	
			if( GameTickCount() > nextTime[player][blob] )
			{
				if( ++nextStage[player][blob] >= kJiggleFrames )
				{
					nextStage[player][blob] = 0;
					nextTime[player][blob] += 40 + RandomBefore( 80 );
				}
				else
				{
					nextTime[player][blob] += 2;
				}
					
				changed = true;
			}
		}
		
		if( changed ) ShowNext( player );
	}
}

void ShowNext( int player )
{
	int      jiggle[kJiggleFrames] = { kNoSuction,  kSquish,  kNoSuction,  kSquash,    
	                                   kNoSuction,  kSquish,  kNoSuction,  kSquash   };
	int      nextFrame = kNoSuction;
	MRect    blobRect = { 3*(32), 3*(4), 3*(32) + kBlobVertSize, 3*(4) + kBlobHorizSize };
	MRect    shadowRect = { 3*(36), 3*(8), 3*(36) + kBlobVertSize, 3*(8) + kBlobHorizSize };
	MRect    srcRect;
	CC_Rect sourceSDLRect, destSDLRect;
	
	if( !nextWindowVisible[player] ) return;

	if( control[player] == kNobodyControl )
	{
	}
	else
	{
		Gfx_AcquireSurface( nextDrawSurface );

		Gfx_BlitSurface( nextSurface[player], &nextSurface[player]->clip_rect,
						 nextDrawSurface,     &nextDrawSurface->clip_rect );
				
		nextFrame = nextG[player]? kNoSuction: jiggle[nextStage[player][0]];
		
		SurfaceDrawShadow( &shadowRect, nextB[player], nextFrame );
		
		CalcBlobRect( nextFrame, nextB[player]-1, &srcRect );
		SurfaceBlitBlob( &srcRect, &blobRect );	  
				  
		OffsetMRect( &blobRect, 0, kBlobVertSize );
		OffsetMRect( &shadowRect, 0, kBlobVertSize );

		nextFrame = nextG[player]? kNoSuction: 
						(nextM[player]? kFlashDarkBlob: jiggle[nextStage[player][1]]);
		
		SurfaceDrawShadow( &shadowRect, nextA[player], nextFrame );

		CalcBlobRect( nextFrame, nextA[player]-1, &srcRect );
		SurfaceBlitBlob( &srcRect, &blobRect );	  
		
		Gfx_ReleaseSurface( nextDrawSurface );

		Gfx_BlitFrontSurface( nextDrawSurface, 
		                       Gfx_MRectToCCRect( &nextWindowZRect, &sourceSDLRect ),
		                       Gfx_MRectToCCRect( &nextWindowRect[player], &destSDLRect ) );
	}
}
