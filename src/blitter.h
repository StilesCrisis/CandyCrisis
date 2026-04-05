// blitter.h

#pragma once

#include "font.h"
#include "gworld.h"
#include "main.h"


#define _HD(x)         (((x) << 1) + ((x) >> 2))         // 2.25x
#define _HDW(x)        (240 + (((x) << 1) + ((x) >> 2))) // 2.25x, centered from 1440w to 1920w

#define _5TO8(x)      (((x) << 3) | ((x) >> 2)) // 8.25x

#define _15TO8_8_8(x) _5TO8((x >> 10) & 0x1F), \
                      _5TO8((x >> 5 ) & 0x1F), \
                      _5TO8(x & 0x1F)


void SurfaceApplyChromaKey(CC_RGBSurface* surface, const MRect* surfaceRect, CC_RGBSurface* background, const MRect* backgroundRect);

void SurfaceBlendUnder(CC_RGBSurface* surface, const MRect* surfaceRect, CC_RGBSurface* background, const MRect* backgroundRect);
void SurfaceBlendOver(CC_RGBSurface* surface, const MRect* surfaceRect, CC_RGBSurface* foreground, const MRect* foregroundRect);

void SurfaceBlitBlob( const MRect* blobRect, const MRect* destRect );

void SurfaceBlitColor( CC_MaskSurface* mask,  CC_RGBSurface* dest,
                       const MRect* maskRect, const MRect* destRect, 
                       int r, int g, int b, int weight );



void SurfaceBlitWeightedCharacter( SkittlesFont* font, unsigned char text, float* dx, int dy, int r, int g, int b, int alpha, float scale = 1.0f );
// Draw an entire string using SurfaceBlitWeightedCharacter, left-to-right from (*x, y); advances *x.
void BlitString(SkittlesFont* font, const char* text, float* x, int y, int r, int g, int b, int alpha, float scale = 1.f);

void SurfaceBlitCharacter( SkittlesFont*   font, unsigned char text, MPoint *dPoint, int r, int g, int b, int dropShadow );
void SurfaceBlitScaledCharacter( SkittlesFont* font, unsigned char text, float* dx, float dy, float scale, int r, int g, int b, int dropShadow, int alpha = 255 );

void SurfaceBlitCursor( MPoint mouseHere, CC_RGBSurface* surface );

void SurfaceBlitBlendOver(CC_RGBSurface* source,     CC_RGBSurface* dest,
                           const MRect* sourceRect, const MRect* destRect, 
                           int r1, int g1, int b1, 
                           int r2, int g2, int b2, 
                           int r3, int g3, int b3, 
                           int r4, int g4, int b4, 
                           int weight );
                          
void SurfaceBlitColorOver(CC_RGBSurface* source,     CC_RGBSurface* dest,
                           const MRect* sourceRect, const MRect* destRect,
                           int r, int g, int b, int weight );

void SurfaceDrawHighlightedButton(CC_RGBSurface* src, CC_RGBSurface* dst, MRect srcRect, int glow, int baseBrightness);

const int kEdgeSize = 8;
typedef COLOR_T SurfaceEdgeData[4][kEdgeSize][kEdgeSize];

void SurfaceGetEdges( CC_RGBSurface* edgeSurface, const MRect *rect, SurfaceEdgeData& edgeData );
void SurfaceCurveEdges( CC_RGBSurface* edgeSurface, const MRect *rect, SurfaceEdgeData& edgeData );


void SetUpdateRect( int player, const MRect *where );
void UpdatePlayerWindow( int player );
void InitBlitter( void );

extern MBoolean update[2][kGridAcross][kGridDown];
extern MBoolean refresh[2];
extern MPoint topLeft[2];

// Shared UI helpers (used by World Select, Pause, etc.)
SDL_Texture* CreateCapsuleTexture(int w, int h);
void DrawCenteredText(SkittlesFont* font, const char* text, const MRect& rect, int r, int g, int b, int shadow);
void DrawScaledCenteredText(SkittlesFont* font, const char* text, const MRect& rect, float scale, int r, int g, int b, int shadow, int alpha = 255);
