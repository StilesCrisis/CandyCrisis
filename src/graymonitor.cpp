// graymonitor.c

#include "stdafx.h"

#include "main.h"
#include "graymonitor.h"
#include "blitter.h"
#include "grays.h"
#include "score.h"
#include "gworld.h"
#include "graphics.h"
#include "level.h"
#include "Globals.h"


MRect                   grayMonitorZRect, grayMonitorRect[2];
MBoolean                grayMonitorVisible[2] = {true, true};
CC_RGBSurface*          grayMonitorSurface[2];
static CC_RGBSurface*   s_grayMonitorDrawSurface;

void InitGrayMonitors()
{
    grayMonitorZRect = { 0, 0, kBlobVertSize + _HD(4), kBlobHorizSize * kGridAcross };
	
	grayMonitorRect[0] = grayMonitorRect[1] = grayMonitorZRect;
	CenterRectOnScreen( &grayMonitorRect[0], kLeftPlayerWindowCenter,  kGrayMonitorVertCenter );
	CenterRectOnScreen( &grayMonitorRect[1], kRightPlayerWindowCenter, kGrayMonitorVertCenter );

    grayMonitorSurface[0]    = Gfx_InitRGBSurface(grayMonitorZRect.right, grayMonitorZRect.bottom);
    grayMonitorSurface[1]    = Gfx_InitRGBSurface(grayMonitorZRect.right, grayMonitorZRect.bottom);
    s_grayMonitorDrawSurface = Gfx_InitRGBSurface(grayMonitorZRect.right, grayMonitorZRect.bottom);
}

void ShowGrayMonitor( short player )
{
	CC_Rect    sourceSDLRect, destSDLRect;
	short      monitor;
	MRect      myRect = { _HD(2), _HD(4), kBlobVertSize + _HD(2), _HD(4) };
	MRect      srcRect;
	static const int smallGrayList[] = { 0, kSmallGray1, kSmallGray2, kSmallGray3, kSmallGray4, kSmallGray5 };
	
	if( !grayMonitorVisible[player] ) return;
	
	if( control[player] != kNobodyControl )
	{
		Gfx_AcquireSurface( s_grayMonitorDrawSurface );
		
        CC_Rect  surfaceRect{0, 0, s_grayMonitorDrawSurface->w, s_grayMonitorDrawSurface->h};

        Gfx_BlitSurface( grayMonitorSurface[player], &surfaceRect,
						 s_grayMonitorDrawSurface,   &surfaceRect  );
	 				
		monitor = unallocatedGrays[player];
		
		CalcBlobRect( kSobBlob, 3, &srcRect );
		while( monitor >= (6*4) )
		{
			myRect.right += kBlobHorizSize;
			SurfaceDrawSprite( &myRect, kBlob4, kSobBlob );
			myRect.left = myRect.right;
			
			monitor -= (6*4);
		}
		
		CalcBlobRect( kNoSuction, kGray-1, &srcRect );
		while( monitor >= 6 )
		{
			myRect.right += kBlobHorizSize;
			SurfaceDrawSprite( &myRect, kGray, kGrayNoBlink );
			myRect.left = myRect.right;
			
			monitor -= 6;
		}
		
		if( monitor > 0 )
		{
			myRect.right += kBlobHorizSize;
			SurfaceDrawSprite( &myRect, kGray, smallGrayList[monitor] );
			myRect.left = myRect.right;
			myRect.right += kBlobHorizSize;
			SurfaceDrawSprite( &myRect, kGray, smallGrayList[monitor]+1 );
		}
		
		Gfx_ReleaseSurface( s_grayMonitorDrawSurface );

		Gfx_BlitFrontSurface( s_grayMonitorDrawSurface,
		                       Gfx_MRectToCCRect( &grayMonitorZRect, &sourceSDLRect ),
		                       Gfx_MRectToCCRect( &grayMonitorRect[player], &destSDLRect ) );
	}
}
