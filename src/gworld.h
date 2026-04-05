// gworld.h

#pragma once
#include "MTypes.h"

#if _WIN32
#include "SDL.h"
#elif __APPLE__
#include "SDL.h"
#endif

#define kBlobHorizSize (3*(24))
#define kBlobVertSize (3*(24))
#define kBlobShadowDepth (3*(6))
#define kBlobShadowError (3*(2))

#define kBlastWidth (3*(72)) 
#define kBlastHeight (3*(72))
#define kBlastFrames 14

#define BLUE_MASK          0x000000FF
#define GREEN_MASK         0x0000FF00
#define RED_MASK           0x00FF0000
#define ALPHA_MASK         0xFF000000
#define CHANNEL_MASK       BLUE_MASK
#define FULL_WEIGHT        (1+CHANNEL_MASK)

#define COLOR_T            int

#define BITS_PER_1CHANNEL  8
#define BITS_PER_2CHANNELS 16
#define BITS_PER_3CHANNELS 24

#define BYTES_PER_PIXEL    4

void GetBlobGraphics();
void InitPlayerWorlds();
void MaskRect( MRect *r );

// Forward-declare SDL_Texture so the surface structs below can hold GPU texture pointers
// without requiring a full SDL header include here.
struct SDL_Texture;

struct CC_Rect
{
    int x, y;
    int w, h;
};

struct CC_RGBPixel
{
    uint8_t b, g, r, X;
};

struct CC_Surface
{
    int          w, h;
    CC_Rect      clip_rect;
    int          pitch;
};

struct CC_RGBSurface : public CC_Surface
{
    SDL_Texture *texture = nullptr;  // GPU-resident texture; null until explicitly created
};

struct CC_MaskSurface : public CC_Surface
{
    uint8_t     *pixels;
    SDL_Texture *texture = nullptr;  // 8-bit alpha mask texture; null until explicitly created
};

SDL_BlendMode    Gfx_PremultipliedBlendMode();   // ONE / ONE_MINUS_SRC_ALPHA, composed once
SDL_BlendMode    Gfx_BlendUnderMode();           // src fills transparent areas under existing dst
SDL_BlendMode    Gfx_AlphaMultiplyBlendMode();   // dst.a = src.a * dst.a / 255; RGB unchanged
void             Gfx_RenderCopyBlend(SDL_Texture* src, SDL_BlendMode blend, const SDL_Rect* srcR, const SDL_Rect* dstR, Uint8 alpha = 255);

void             InitFrontSurface();

CC_Rect*         Gfx_MRectToCCRect( const MRect* in, CC_Rect* out );
MRect*           Gfx_CCRectToMRect( const CC_Rect* in, MRect* out );
void             Gfx_BlitSurface( CC_RGBSurface* srcSurface, const CC_Rect* srcRect, CC_RGBSurface* dstSurface, const CC_Rect* dstRect);
void             Gfx_FillRect( CC_RGBSurface* surface, CC_Rect* rect, CC_RGBPixel color);
void             Gfx_GetPixel(	CC_RGBSurface* surface, int x, int y, CC_RGBPixel* pixel );
CC_RGBSurface*   Gfx_InitRGBSurface(int width, int height);
CC_MaskSurface*  Gfx_InitMaskSurface(int width, int height);
void             Gfx_FreeSurface(CC_RGBSurface* s);
void             Gfx_FreeSurface(CC_MaskSurface* s);
void             Gfx_BlitFrontSurface( CC_RGBSurface* source, const CC_Rect* sourceSDLRect, const CC_Rect* destSDLRect );
void             Gfx_AcquireSurface( CC_RGBSurface* surface );
CC_RGBSurface*   Gfx_GetCurrentSurface();
void             Gfx_ReleaseSurface( CC_RGBSurface* surface );

bool             DrawPICTInSurface(CC_RGBSurface* surface, int pictID, const char* suffixA = "", const char* suffixB = "");
CC_RGBSurface*   LoadPICTAsRGBSurface(int pictID, const char* suffixA = "", const char* suffixB = "");
CC_MaskSurface*  LoadPICTAsMaskSurface(int pictID, const char* suffixA = "", const char* suffixB = "");
uint8_t*         LoadPICTAlphaPixels(int pictID, const char* suffixA, const char* suffixB, int* outW, int* outH);
MBoolean         PICTExists( int pictID );
MBoolean         FileExists( const char* name );
const char*      QuickResourceName(const char* prefix, int id, const char* suffixA, const char* suffixB = "", const char* suffixC = "");

void SurfaceDrawBoard( int player, const MRect *myRect );
void SurfaceDrawShadow( const MRect *myRect, int blob, int state );
void SurfaceDrawColor( const MRect *myRect, int blob, int state, int r, int g, int b, int w );
void SurfaceDrawBlob( int player, const MRect *myRect, int blob, int state, int charred );
void SurfaceDrawSprite( const MRect *myRect, int blob, int state );

extern CC_RGBSurface*   g_frontSurface;
extern CC_RGBSurface*   blobSurface;
extern CC_MaskSurface*  maskSurface;
extern CC_MaskSurface*  charMaskSurface;
extern CC_RGBSurface*   boardSurface[2];
extern CC_RGBSurface*   blastSurface;
extern CC_RGBSurface*   blastMaskSurface;
extern CC_RGBSurface*   playerSurface[2];
extern CC_RGBSurface*   playerSpriteSurface[2];
