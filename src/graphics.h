// graphics.h

#pragma once
#include "MTypes.h"

void DrawSpriteBlobs( int player, int type );
void EraseSpriteBlobs( int player );
void CleanSpriteArea( int player, const MRect *myRect );
void CalcBlobRect( int x, int y, MRect *myRect );
void DrawBackdrop( void );
void ShowTitle( void );
void InitBackdrop( void );
void CenterRectOnScreen( MRect *rect, double locationX, double locationY );


struct CC_RGBSurface;
extern CC_RGBSurface* backdropSurface;


enum 
{
	blobBlinkAnimation = 0,
	blobJiggleAnimation,
	blobCryAnimation
};
