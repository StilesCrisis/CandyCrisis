// gworld.c

#include "stdafx.h"

#include "main.h"
#include "gworld.h"
#include "blitter.h"
#include "graphics.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#if __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#endif
#include "SDLU.h"
#include "Platform.h"

static CC_RGBPixel* LoadPICTRGBPixels(int pictID, const char* suffixA, const char* suffixB, int* outW, int* outH);

CC_RGBSurface*   g_frontSurface;
CC_RGBSurface*   blobSurface;
CC_MaskSurface*  maskSurface;
CC_MaskSurface*  charMaskSurface;
CC_RGBSurface*   boardSurface[2];
CC_RGBSurface*   blastSurface;
CC_RGBSurface*   blastMaskSurface;

// Strip textures for GPU charring: kBombTop-1 (opaque source) and kBombBottom-1 (white+blue-as-alpha).
static SDL_Texture* s_charSrcTex   = nullptr;
static SDL_Texture* s_charAlphaTex = nullptr;

// Custom blend modes for charring passes (alpha-only operations).
// s_alphaReplaceMode:  dst.rgb unchanged; dst.a = src.a
// s_alphaMultiplyMode: dst.rgb unchanged; dst.a = src.a * dst.a / 255
static SDL_BlendMode s_alphaReplaceMode  = SDL_BLENDMODE_NONE;
static SDL_BlendMode s_alphaMultiplyMode = SDL_BLENDMODE_NONE;
CC_RGBSurface*   playerSurface[2];
CC_RGBSurface*   playerSpriteSurface[2];


// Premultiplied-alpha blend mode: dstRGB = srcRGB + dstRGB*(1-srcA), dstA = srcA + dstA*(1-srcA).
// SDL2 doesn't have a named constant for this; compose it once on first use.
SDL_BlendMode Gfx_PremultipliedBlendMode()
{
    static SDL_BlendMode s_mode = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ONE,               SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD,
        SDL_BLENDFACTOR_ONE,               SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
    return s_mode;
}

// Blend-under mode: renders src UNDER existing dst content.
// dstRGB = srcRGB*(1-dstA) + dstRGB*dstA   (background fills transparent areas of dst)
// dstA   = srcA*(1-dstA)   + dstA           (dst alpha grows toward opaque)
SDL_BlendMode Gfx_BlendUnderMode()
{
    static SDL_BlendMode s_mode = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA, SDL_BLENDFACTOR_DST_ALPHA,           SDL_BLENDOPERATION_ADD,
        SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA, SDL_BLENDFACTOR_ONE,                 SDL_BLENDOPERATION_ADD);
    return s_mode;
}

// Alpha-multiply mode: dst.RGB unchanged; dst.a = src.a * dst.a / 255.
// Used to clip a surface's alpha by a mask texture.
SDL_BlendMode Gfx_AlphaMultiplyBlendMode()
{
    static SDL_BlendMode s_mode = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ZERO,      SDL_BLENDFACTOR_ONE,  SDL_BLENDOPERATION_ADD,   // RGB: dst unchanged
        SDL_BLENDFACTOR_DST_ALPHA, SDL_BLENDFACTOR_ZERO, SDL_BLENDOPERATION_ADD);  // A:   dst.a = src.a * dst.a / 255
    return s_mode;
}


// SDL2 reads texture blend mode at flush time, not at submission time.
// This wrapper guarantees the blend mode is active when the batch executes:
// set → queue → flush → restore. Callers never need to manage the ordering.
// Fast path: if the texture already has the requested blend mode and alpha=255,
// just queue the copy — no flush needed because there is nothing to lock in.
void Gfx_RenderCopyBlend(SDL_Texture* src, SDL_BlendMode blend,
                          const SDL_Rect* srcR, const SDL_Rect* dstR, Uint8 alpha)
{
    SDL_BlendMode prev;
    SDL_GetTextureBlendMode(src, &prev);
    if (prev == blend && alpha == 255)
    {
        SDL_RenderCopy(g_renderer, src, srcR, dstR);
        return;
    }
    SDL_SetTextureBlendMode(src, blend);
    SDL_SetTextureAlphaMod(src, alpha);
    SDL_RenderCopy(g_renderer, src, srcR, dstR);
    SDL_RenderFlush(g_renderer);
    SDL_SetTextureAlphaMod(src, 255);
    SDL_SetTextureBlendMode(src, prev);
}


// for acquiresurface
const int             k_acquireMax = 10;
static int            s_acquireHead = -1;
static CC_RGBSurface* s_acquireList[k_acquireMax];


void InitFrontSurface()
{
    g_frontSurface = Gfx_InitRGBSurface(1920, 1080);

    // Gfx_InitRGBSurface created a fresh TARGET texture, but SDLU already created g_frontTexture
    // as the designated game-frame render target. Replace the duplicate so that
    // Gfx_AcquireSurface(g_frontSurface) sets the render target to exactly g_frontTexture.
    if (g_frontTexture)
    {
        SDL_DestroyTexture(g_frontSurface->texture);
        g_frontSurface->texture = g_frontTexture;
    }

}

void GetBlobGraphics()
{
	// Allocate surfaces for the board
	
	const int width  = kBlobHorizSize * kGridAcross;
	const int height = kBlobVertSize * (kGridDown-1);
	
    boardSurface[0] = Gfx_InitRGBSurface(width, height);
    boardSurface[1] = Gfx_InitRGBSurface(width, height);

	// Get blob surfaces

    // Load blob pixels locally for baking
    int blobW, blobH;
    CC_RGBPixel* blobPixels = LoadPICTRGBPixels(picBlob, "", "", &blobW, &blobH);
    blobSurface = Gfx_InitRGBSurface(blobW, blobH);

    maskSurface     = LoadPICTAsMaskSurface( picBlobMask );
    charMaskSurface = LoadPICTAsMaskSurface( picCharMask );

    if (g_renderer && blobPixels && maskSurface)
    {
        const int pitchPix = blobW;

        for (int y = 0; y < blobH; y++)
        {
            CC_RGBPixel*    pixRow = blobPixels + y * pitchPix;
            const uint32_t* mskRow = (const uint32_t*)(maskSurface->pixels + y * maskSurface->pitch);
            for (int x = 0; x < blobW; x++)
            {
                uint32_t bit = 0x80000000u >> (x & 31);
                pixRow[x].X  = (mskRow[x >> 5] & bit) ? 255 : 0;
            }
        }

        {
            const int grayTop  = (kGray  - 1) * kBlobVertSize;
            const int lightTop = (kLight - 1) * kBlobVertSize;
            for (int py = 0; py < kBlobVertSize; py++)
            {
                CC_RGBPixel*       grayRow  = blobPixels + (grayTop  + py) * pitchPix;
                const CC_RGBPixel* lightRow = blobPixels + (lightTop + py) * pitchPix;
                for (int px = 0; px < blobW; px++)
                    grayRow[px].X = 255 - lightRow[px].b;
            }
        }

        SDL_UpdateTexture(blobSurface->texture, NULL, blobPixels, blobW * (int)sizeof(CC_RGBPixel));
        SDL_SetTextureBlendMode(blobSurface->texture, SDL_BLENDMODE_BLEND);
    }

	// Get blast surfaces

    // Load blast pixels locally for baking
    int blastW, blastH, blastMskW, blastMskH;
    CC_RGBPixel* blastPixels    = LoadPICTRGBPixels(picBlast,     "", "", &blastW,    &blastH);
    CC_RGBPixel* blastMskPixels = LoadPICTRGBPixels(picBlastMask, "", "", &blastMskW, &blastMskH);
    blastSurface     = Gfx_InitRGBSurface(blastW,    blastH);
    blastMaskSurface = Gfx_InitRGBSurface(blastMskW, blastMskH);

    if (g_renderer && blastPixels && blastMskPixels)
    {
        const int pitchPix = blastW;
        const int mskPitch = blastMskW;
        for (int y = 0; y < blastH; y++)
        {
            CC_RGBPixel*       dstRow = blastPixels    + y * pitchPix;
            const CC_RGBPixel* mskRow = blastMskPixels + y * mskPitch;
            for (int x = 0; x < blastW; x++)
                dstRow[x].X = 255 - mskRow[x].b;
        }
        SDL_UpdateTexture(blastSurface->texture, NULL, blastPixels, blastW * (int)sizeof(CC_RGBPixel));
        SDL_SetTextureBlendMode(blastSurface->texture, SDL_BLENDMODE_BLEND);
    }
    free(blastPixels);
    free(blastMskPixels);

    // Custom blend modes for charring passes — alpha-only, RGB left unchanged.
    s_alphaReplaceMode = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE,      SDL_BLENDOPERATION_ADD,   // RGB: dst unchanged
        SDL_BLENDFACTOR_ONE,  SDL_BLENDFACTOR_ZERO,     SDL_BLENDOPERATION_ADD);  // A:   dst.a = src.a
    s_alphaMultiplyMode = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ZERO,      SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD,   // RGB: dst unchanged
        SDL_BLENDFACTOR_DST_ALPHA, SDL_BLENDFACTOR_ZERO, SDL_BLENDOPERATION_ADD); // A:   dst.a = src.a * dst.a / 255

    // Build strip textures for GPU-based charring.
    // s_charSrcTex:   kBombTop-1 row, RGB from blobPixels, alpha=255 (seeds scratch with BLEND_NONE).
    // s_charAlphaTex: kBombBottom-1 row, white pixels, alpha = blue channel (per-pixel opacity source).
    if (g_renderer && blobPixels)
    {
        const int stripW     = blobW;
        const int pitchPix   = blobW;
        const int stripPitch = stripW * (int)sizeof(CC_RGBPixel);
        CC_RGBPixel* tmp = (CC_RGBPixel*)malloc((size_t)stripPitch * kBlobVertSize);

        // charSrcTex: source visual, alpha=255 everywhere so MUL can modulate it cleanly.
        {
            const int rowTop = (kBombTop - 1) * kBlobVertSize;
            for (int y = 0; y < kBlobVertSize; y++)
            {
                const CC_RGBPixel* src = blobPixels + (rowTop + y) * pitchPix;
                CC_RGBPixel*       dst = tmp + y * stripW;
                for (int x = 0; x < stripW; x++) { dst[x] = src[x]; dst[x].X = 255; }
            }
            s_charSrcTex = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STATIC, stripW, kBlobVertSize);
            SDL_UpdateTexture(s_charSrcTex, NULL, tmp, stripPitch);
        }

        // charAlphaTex: white pixels, alpha = blue channel of kBombBottom-1 strip.
        {
            const int rowBot = (kBombBottom - 1) * kBlobVertSize;
            for (int y = 0; y < kBlobVertSize; y++)
            {
                const CC_RGBPixel* src = blobPixels + (rowBot + y) * pitchPix;
                CC_RGBPixel*       dst = tmp + y * stripW;
                for (int x = 0; x < stripW; x++)
                {
                    dst[x].r = dst[x].g = dst[x].b = 255;
                    dst[x].X = src[x].b;
                }
            }
            s_charAlphaTex = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888,
                                               SDL_TEXTUREACCESS_STATIC, stripW, kBlobVertSize);
            SDL_UpdateTexture(s_charAlphaTex, NULL, tmp, stripPitch);
        }

        free(tmp);
    }
    free(blobPixels);
}


void InitPlayerWorlds()
{
	for (int count=0; count<2; ++count)
	{
        playerSurface[count]       = Gfx_InitRGBSurface(kGridAcross * kBlobHorizSize, kGridDown * kBlobVertSize);
		playerSpriteSurface[count] = Gfx_InitRGBSurface(kGridAcross * kBlobHorizSize, kGridDown * kBlobVertSize);
	}
}


void SurfaceDrawBoard( int player, const MRect *myRect )
{
	MRect   srcRect, offsetRect;
	CC_Rect srcSDLRect, offsetSDLRect;
	
	srcRect = *myRect;
	if( srcRect.bottom <= kBlobVertSize )
        return;

    if( srcRect.top < kBlobVertSize )
        srcRect.top = kBlobVertSize;

	offsetRect = srcRect;
	OffsetMRect( &offsetRect, 0, -kBlobVertSize );

	Gfx_BlitSurface( boardSurface[player],    Gfx_MRectToCCRect( &offsetRect, &offsetSDLRect ),
	                 Gfx_GetCurrentSurface(), Gfx_MRectToCCRect( &srcRect, &srcSDLRect )         );
}


void SurfaceDrawBlob( int player, const MRect *myRect, int blob, int state, int charred )
{
	SurfaceDrawBoard( player, myRect );
	SurfaceDrawSprite( myRect, blob, state );

	if( charred & 0x0F )
	{
		MRect blobRect, charRect;
		CalcBlobRect( (charred & 0x0F), kBombTop-1, &charRect );
		CalcBlobRect( state, blob-1, &blobRect );

		const int w = kBlobHorizSize, h = kBlobVertSize;
		SDL_Rect scratchR = { 0, 0, w, h };
		SDL_Rect charSrcR = { charRect.left, 0, w, h };
		SDL_Rect maskSrcR = { blobRect.left, blobRect.top, w, h };
		SDL_Rect destR    = { myRect->left, myRect->top, w, h };
		CC_RGBSurface* dest = Gfx_GetCurrentSurface();

		SDL_Texture* prevTarget = SDL_GetRenderTarget(g_renderer);

		// Step 1: Seed scratch with char source RGB, alpha=255 everywhere (BLEND_NONE overwrites).
		SDL_SetRenderTarget(g_renderer, g_scratchTexture);
		SDL_SetTextureBlendMode(s_charSrcTex, SDL_BLENDMODE_NONE);
		SDL_RenderCopy(g_renderer, s_charSrcTex, &charSrcR, &scratchR);

		// Step 2: Replace scratch.a with charAlpha.a (= blue channel). RGB untouched.
		SDL_SetTextureBlendMode(s_charAlphaTex, s_alphaReplaceMode);
		SDL_RenderCopy(g_renderer, s_charAlphaTex, &charSrcR, &scratchR);

		// Step 3: Multiply scratch.a by charMask.a (zero outside blob shape). RGB untouched.
		Gfx_RenderCopyBlend(charMaskSurface->texture, s_alphaMultiplyMode, &maskSrcR, &scratchR);
		SDL_SetRenderTarget(g_renderer, dest->texture);

		// Step 4: Blend scratch onto dest, alphaMod = inWeight scales the final opacity.
		SDL_SetTextureBlendMode(g_scratchTexture, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod(g_scratchTexture, (uint8_t)(charred & 0xF0));
		SDL_RenderCopy(g_renderer, g_scratchTexture, &scratchR, &destR);
		// Restore target (flushes batch) before restoring alpha mod.
		SDL_SetRenderTarget(g_renderer, prevTarget);
		SDL_SetTextureAlphaMod(g_scratchTexture, 255);
	}
}

void SurfaceDrawShadow( const MRect *myRect, int blob, int state )
{
	int x;
	const MPoint offset[4] = { {-4, 0}, {0, -4}, {4, 0}, {0, 4} }; // 3-scale
	
	if( blob > kEmpty )
	{
		MRect blobRect, destRect;

		for( x=0; x<4; x++ )
		{
			destRect = *myRect;
			OffsetMRect( &destRect, offset[x].h, offset[x].v );
			
			CalcBlobRect( state, blob-1, &blobRect );
			SurfaceBlitColor( maskSurface,  Gfx_GetCurrentSurface(),
			                  &blobRect,    &destRect, 
			                   0, 0, 0, _5TO8(3) );
		}
	}
}

void SurfaceDrawColor( const MRect *myRect, int blob, int state, int r, int g, int b, int w )
{
	MRect blobRect;
	if( blob > kEmpty )
	{
		CalcBlobRect( state, blob-1, &blobRect );
		SurfaceBlitColor( charMaskSurface,  Gfx_GetCurrentSurface(),
						  &blobRect,         myRect, 
						   r, g, b, w );
	}
}


void SurfaceDrawSprite( const MRect *myRect, int blob, int state )
{
	MRect blobRect;
	if( blob > kEmpty )
	{
		CalcBlobRect( state, blob-1, &blobRect );
		SurfaceBlitBlob( &blobRect, myRect );
	}
}


const char* QuickResourceName(const char* prefix, int id, const char* suffixA, const char* suffixB, const char* suffixC)
{
    static char name[1024];
    if (id)
    {
        snprintf(name, arrsize(name), "%s%s_%d%s%s%s", candyCrisisResources, prefix, id, suffixA, suffixB, suffixC);
    }
    else
    {
        snprintf(name, arrsize(name), "%s%s%s%s%s", candyCrisisResources, prefix, suffixA, suffixB, suffixC);
    }
    
    return name;
}


MBoolean FileExists( const char* name )
{
    FILE* f = fopen( name, "rb" );
    if( f == NULL )
    {
        return false;
    }
    
    fclose( f );
    return true;
}


MBoolean PICTExists( int pictID )
{
    if (FileExists(QuickResourceName("PICT", pictID, ".png")))
        return true;

	if (FileExists(QuickResourceName("PICT", pictID, ".jpg")))
		return true;

	return false;
}


bool DrawPICTInSurface( CC_RGBSurface* surface, int pictID, const char* suffixA, const char* suffixB)
{
    CC_RGBSurface* image = LoadPICTAsRGBSurface(pictID, suffixA, suffixB);
    if( image == nullptr )
        return false;
    
    Gfx_BlitSurface( image,    &image->clip_rect,
                     surface,  &surface->clip_rect );
    
    Gfx_FreeSurface( image );
    
    return true;
}


// Internal: load a PICT into a premultiplied-alpha CC_RGBPixel buffer. Caller frees.
static CC_RGBPixel* LoadPICTRGBPixels(int pictID, const char* suffixA, const char* suffixB, int* outW, int* outH)
{
#if __APPLE__
    static CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGImageRef img = NULL;
    const char* filename = QuickResourceName("PICT", pictID, suffixA, suffixB, ".jpg");
    CGDataProviderRef dataProvider = CGDataProviderCreateWithFilename(filename);
    if (dataProvider != NULL) {
        img = CGImageCreateWithJPEGDataProvider(dataProvider, NULL, true, kCGRenderingIntentPerceptual);
        CGDataProviderRelease(dataProvider);
    } else {
        filename = QuickResourceName("PICT", pictID, suffixA, suffixB, ".png");
        dataProvider = CGDataProviderCreateWithFilename(filename);
        if (dataProvider != NULL) {
            img = CGImageCreateWithPNGDataProvider(dataProvider, NULL, true, kCGRenderingIntentPerceptual);
            CGDataProviderRelease(dataProvider);
        }
    }
    if (img == NULL) return NULL;
    int width  = (int)CGImageGetWidth(img);
    int height = (int)CGImageGetHeight(img);
    int pitch  = width * sizeof(CC_RGBPixel);
    CC_RGBPixel* pixels = (CC_RGBPixel*)calloc(pitch, height);
    CGContextRef context = CGBitmapContextCreate(pixels, width, height, 8, pitch, colorSpace,
        kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), img);
    CGContextRelease(context);
    CGImageRelease(img);
    *outW = width; *outH = height;
    return pixels;
#else
    const char* filename;
    SDL_Surface* surfaceNative = nullptr;
    filename = QuickResourceName("PICT", pictID, suffixA, suffixB, ".png");
    if (FileExists(filename))
        surfaceNative = IMG_Load(filename);
    else {
        filename = QuickResourceName("PICT", pictID, suffixA, suffixB, ".jpg");
        if (FileExists(filename))
            surfaceNative = IMG_Load(filename);
        else
            return NULL;
    }
    SDL_Surface* surfaceRGB = SDL_CreateRGBSurface(SDL_SWSURFACE, surfaceNative->w, surfaceNative->h, 32,
                                                   RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK);
    SDL_BlitSurface(surfaceNative, &surfaceNative->clip_rect, surfaceRGB, &surfaceRGB->clip_rect);
    SDL_FreeSurface(surfaceNative);
    int w = surfaceRGB->clip_rect.w, h = surfaceRGB->clip_rect.h;
    int pitch = w * sizeof(CC_RGBPixel);
    CC_RGBPixel* pixels = (CC_RGBPixel*)calloc(pitch, h);
    for (int y = 0; y < h; ++y) {
        COLOR_T* srcPixels = (COLOR_T*)((uint8_t*)surfaceRGB->pixels + (y * surfaceRGB->pitch));
        COLOR_T* dstPixels = (COLOR_T*)((uint8_t*)pixels             + (y * pitch));
        for (int x = 0; x < w; ++x) {
            COLOR_T color = *srcPixels;
            int workB = (color & BLUE_MASK);
            int workG = (color & GREEN_MASK) >> BITS_PER_1CHANNEL;
            int workR = (color & RED_MASK)   >> BITS_PER_2CHANNELS;
            int workA = (color & ALPHA_MASK) >> BITS_PER_3CHANNELS;
            *dstPixels = ((workB * workA) / 255) |
                        (((workG * workA) / 255) << BITS_PER_1CHANNEL) |
                        (((workR * workA) / 255) << BITS_PER_2CHANNELS) |
                          (workA << BITS_PER_3CHANNELS);
            ++srcPixels; ++dstPixels;
        }
    }
    SDL_FreeSurface(surfaceRGB);
    *outW = w; *outH = h;
    return pixels;
#endif
}


CC_RGBSurface*   LoadPICTAsRGBSurface(int pictID, const char* suffixA, const char* suffixB)
{
#ifdef __EMSCRIPTEN__
    if (pictID >= 5000 && pictID < 5100) SDLU_LoadLevelPackage(pictID);
#endif
    int w, h;
    CC_RGBPixel* pixels = LoadPICTRGBPixels(pictID, suffixA, suffixB, &w, &h);
    if (!pixels) return nullptr;

    CC_RGBSurface* surface = Gfx_InitRGBSurface(w, h);
    if (g_renderer)
    {
        SDL_UpdateTexture(surface->texture, NULL, pixels, w * sizeof(CC_RGBPixel));
        SDL_SetTextureBlendMode(surface->texture, Gfx_PremultipliedBlendMode());
    }
    free(pixels);
    return surface;
}


uint8_t* LoadPICTAlphaPixels(int pictID, const char* suffixA, const char* suffixB, int* outW, int* outH)
{
#if __APPLE__
    static CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
    CGImageRef img = NULL;
    const char* filename = QuickResourceName("PICT", pictID, suffixA, suffixB, ".png");
    CGDataProviderRef dataProvider = CGDataProviderCreateWithFilename(filename);
    if (dataProvider != NULL) {
        img = CGImageCreateWithPNGDataProvider(dataProvider, NULL, true, kCGRenderingIntentPerceptual);
        CGDataProviderRelease(dataProvider);
    }
    if (img == NULL) return NULL;
    int width  = (int)CGImageGetWidth(img);
    int height = (int)CGImageGetHeight(img);
    int pitch  = width;
    uint8_t* pixels = (uint8_t*)calloc(pitch, height);
    CGContextRef context = CGBitmapContextCreate(pixels, width, height, 8, pitch, colorSpace, kCGImageAlphaNone);
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), img);
    CGContextRelease(context);
    CGImageRelease(img);
    *outW = width; *outH = height;
    return pixels;
#else
    const char*  filename;
    SDL_Surface* surfaceNative = nullptr;
    filename = QuickResourceName("PICT", pictID, suffixA, suffixB, ".png");
    if (FileExists(filename))
        surfaceNative = IMG_Load(filename);
    else
        return NULL;

    extern SDL_Palette* g_grayscalePalette;
    int w = surfaceNative->w, h = surfaceNative->h;
    SDL_Surface* surfaceGray = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
    SDL_SetSurfacePalette(surfaceGray, g_grayscalePalette);
    SDL_BlitSurface(surfaceNative, &surfaceNative->clip_rect, surfaceGray, &surfaceGray->clip_rect);
    SDL_FreeSurface(surfaceNative);

    uint8_t* pixels = (uint8_t*)calloc(w, h);
    for (int y = 0; y < h; ++y) {
        uint8_t* srcRow = (uint8_t*)surfaceGray->pixels + (y * surfaceGray->pitch);
        uint8_t* dstRow = pixels + (y * w);
        memcpy(dstRow, srcRow, w);
    }
    SDL_FreeSurface(surfaceGray);
    *outW = w; *outH = h;
    return pixels;
#endif
}


CC_MaskSurface*  LoadPICTAsMaskSurface(int pictID, const char* suffixA, const char* suffixB)
{
#ifdef __EMSCRIPTEN__
    if (pictID >= 5000 && pictID < 5100) SDLU_LoadLevelPackage(pictID);
#endif
    int aw, ah;
    uint8_t* alpha = LoadPICTAlphaPixels(pictID, suffixA, suffixB, &aw, &ah);
    if (!alpha) return NULL;

    // Build 1-bit mask
    CC_MaskSurface* maskSurface = Gfx_InitMaskSurface(aw, ah);
    for (int y = 0; y < ah; y++)
    {
        uint8_t*  src     = alpha + (y * aw);
        uint32_t* dst     = (uint32_t*)(maskSurface->pixels + (y * maskSurface->pitch));
        uint32_t  bitmask = 0x80000000;
        for (int x = 0; x < aw; ++x)
        {
            if (*src <= 0xF0)
                *dst |= bitmask;
            ++src;
            bitmask >>= 1;
            if (!bitmask) { bitmask = 0x80000000; ++dst; }
        }
    }

    // Build GPU alpha-mask texture
    if (g_renderer)
    {
        SDL_Surface* texPixels = SDL_CreateRGBSurface(0, aw, ah, 32,
                                                      RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK);
        for (int y = 0; y < ah; y++)
        {
            const uint8_t* src = alpha + (y * aw);
            uint32_t*      dst = (uint32_t*)((uint8_t*)texPixels->pixels + (y * texPixels->pitch));
            for (int x = 0; x < aw; x++)
            {
                uint8_t a = (src[x] <= 0xF0) ? 255 : 0;
                dst[x]    = ((uint32_t)a << BITS_PER_3CHANNELS) | 0x00FFFFFF;
            }
        }
        maskSurface->texture = SDL_CreateTextureFromSurface(g_renderer, texPixels);
        SDL_FreeSurface(texPixels);
        SDL_SetTextureBlendMode(maskSurface->texture, SDL_BLENDMODE_BLEND);
    }

    free(alpha);
    return maskSurface;
}



CC_Rect* Gfx_MRectToCCRect( const MRect* in, CC_Rect* out )
{
    int t = in->top, l = in->left, b = in->bottom, r = in->right;
    
    out->x = l;
    out->y = t;
    out->w = r - l;
    out->h = b - t;
    
    return out;
}


MRect* Gfx_CCRectToMRect( const CC_Rect* in, MRect* out )
{
    int x = in->x, y = in->y, w = in->w, h = in->h;
    
    out->top    = y;
    out->left   = x;
    out->bottom = y + h;
    out->right  = x + w;
    
    return out;
}


// Helper: convert CC_Rect to SDL_Rect.
static SDL_Rect Gfx_CCRectToSDLRect(const CC_Rect* r)
{
    SDL_Rect sr;
    sr.x = r->x; sr.y = r->y; sr.w = r->w; sr.h = r->h;
    return sr;
}


void Gfx_FillRect( CC_RGBSurface* surface, CC_Rect* ccRect, CC_RGBPixel color)
{
    MRect rect;
    Gfx_CCRectToMRect(ccRect, &rect);
    
    // Clip rect against surface.
    if (rect.top < 0)
    {
        int clipTop = 0 - rect.top;
        rect.top += clipTop;
    }
    
    if (rect.left < 0)
    {
        int clipLeft = 0 - rect.left;
        rect.left += clipLeft;
    }
    
    if (rect.bottom >= surface->h)
    {
        int clipBottom = rect.bottom - surface->h;
        rect.bottom -= clipBottom;
    }
    
    if (rect.right >= surface->w)
    {
        int clipRight = rect.right - surface->w;
        rect.right -= clipRight;
    }
    
    // If we have clipped ourselves out of a job, bail now.
    if (rect.left >= rect.right || rect.top >= rect.bottom)
        return;

    // GPU path: fill the texture region with the same color.
    if (surface->texture)
    {
        SDL_Texture* prevTarget = SDL_GetRenderTarget(g_renderer);
        SDL_SetRenderTarget(g_renderer, surface->texture);
        SDL_BlendMode prevMode;
        SDL_GetRenderDrawBlendMode(g_renderer, &prevMode);
        SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(g_renderer, color.r, color.g, color.b, 255);
        SDL_Rect sdlRect = Gfx_CCRectToSDLRect(ccRect);
        SDL_RenderFillRect(g_renderer, &sdlRect);
        SDL_SetRenderDrawBlendMode(g_renderer, prevMode);
        SDL_SetRenderTarget(g_renderer, prevTarget);
    }
}

void Gfx_BlitSurface( CC_RGBSurface* srcSurface, const CC_Rect* srcCCRect,
                      CC_RGBSurface* dstSurface, const CC_Rect* dstCCRect  )
{
    MRect srcRect, dstRect;
    Gfx_CCRectToMRect(srcCCRect, &srcRect);
    Gfx_CCRectToMRect(dstCCRect, &dstRect);
    
    // Clip source rect against source surface.
    if (srcRect.top < 0)
    {
        int clipTop = 0 - srcRect.top;
        dstRect.top += clipTop;
        srcRect.top += clipTop;
    }
    
    if (srcRect.left < 0)
    {
        int clipLeft = 0 - srcRect.left;
        dstRect.left += clipLeft;
        srcRect.left += clipLeft;
    }
    
    if (srcRect.bottom >= srcSurface->h)
    {
        int clipBottom = srcRect.bottom - srcSurface->h;
        dstRect.bottom -= clipBottom;
        srcRect.bottom -= clipBottom;
    }
    
    if (srcRect.right >= srcSurface->w)
    {
        int clipRight = srcRect.right - srcSurface->w;
        dstRect.right -= clipRight;
        srcRect.right -= clipRight;
    }
    
    // Clip destination rect against destination surface.
    if (dstRect.top < 0)
    {
        int clipTop = 0 - dstRect.top;
        dstRect.top += clipTop;
        srcRect.top += clipTop;
    }
    
    if (dstRect.left < 0)
    {
        int clipLeft = 0 - dstRect.left;
        dstRect.left += clipLeft;
        srcRect.left += clipLeft;
    }
    
    if (dstRect.bottom >= dstSurface->h)
    {
        int clipBottom = dstRect.bottom - dstSurface->h;
        dstRect.bottom -= clipBottom;
        srcRect.bottom -= clipBottom;
    }
    
    if (dstRect.right >= dstSurface->w)
    {
        int clipRight = dstRect.right - dstSurface->w;
        dstRect.right -= clipRight;
        srcRect.right -= clipRight;
    }
    
    // Clamp source and dest rectangles if one is larger than the other.
    int srcWidth = srcRect.right - srcRect.left;
    int dstWidth = dstRect.right - dstRect.left;
    
    if (srcWidth < dstWidth)
    {
        dstRect.right = dstRect.left + srcWidth;
    }
    else if (dstWidth < srcWidth)
    {
        srcRect.right = srcRect.left + dstWidth;
    }
    
    int srcHeight = srcRect.bottom - srcRect.top;
    int dstHeight = dstRect.bottom - dstRect.top;
    
    if (srcHeight < dstHeight)
    {
        dstRect.bottom = dstRect.top + srcHeight;
    }
    else if (dstHeight < srcHeight)
    {
        srcRect.bottom = srcRect.top + dstHeight;
    }
    
    // If we have clipped ourselves out of a job, bail now.
    if (srcRect.left >= srcRect.right || srcRect.top >= srcRect.bottom)
        return;

    // GPU path: blit source texture to dest texture region.
    // Use BLEND_NONE (straight copy, matching the CPU memcpy semantics).
    if (srcSurface->texture && dstSurface->texture)
    {
        // Use the fully-clipped MRects computed above, not the original CC_Rect inputs.
        // This matches the CPU path's clipping (negative coords, out-of-bounds edges, etc.).
        SDL_Rect sr = { srcRect.left, srcRect.top, srcRect.right - srcRect.left, srcRect.bottom - srcRect.top };
        SDL_Rect dr = { dstRect.left, dstRect.top, dstRect.right - dstRect.left, dstRect.bottom - dstRect.top };

        SDL_Texture* prevTarget = SDL_GetRenderTarget(g_renderer);
        SDL_SetRenderTarget(g_renderer, dstSurface->texture);
        Gfx_RenderCopyBlend(srcSurface->texture, SDL_BLENDMODE_NONE, &sr, &dr);
        SDL_SetRenderTarget(g_renderer, prevTarget);
    }
}


void Gfx_GetPixel(	CC_RGBSurface* surface, int x, int y, CC_RGBPixel* pixel )
{
    SDL_Texture* prev = SDL_GetRenderTarget(g_renderer);
    SDL_SetRenderTarget(g_renderer, surface->texture);
    SDL_Rect r = { x, y, 1, 1 };
    uint32_t px = 0;
    SDL_RenderReadPixels(g_renderer, &r, SDL_PIXELFORMAT_ARGB8888, &px, (int)sizeof(px));
    SDL_SetRenderTarget(g_renderer, prev);
    pixel->b = (uint8_t)( px        & 0xFF);
    pixel->g = (uint8_t)((px >>  8) & 0xFF);
    pixel->r = (uint8_t)((px >> 16) & 0xFF);
    pixel->X = (uint8_t)((px >> 24) & 0xFF);
}


void             Gfx_FreeSurface(CC_RGBSurface* s)
{
    if (s->texture) SDL_DestroyTexture(s->texture);
    delete s;
}


void             Gfx_FreeSurface(CC_MaskSurface* s)
{
    if (s->texture) SDL_DestroyTexture(s->texture);
    free(s->pixels);
    delete s;
}


CC_RGBSurface* Gfx_InitRGBSurface(int width, int height)
{
    CC_RGBSurface*  surface = new CC_RGBSurface;
    surface->w = width;
    surface->h = height;
    surface->clip_rect = {0, 0, width, height};
    surface->pitch = width * sizeof(CC_RGBPixel);

    if (g_renderer)
    {
        surface->texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_TARGET, width, height);
        SDL_SetTextureBlendMode(surface->texture, SDL_BLENDMODE_NONE);
    }

    return surface;
}


CC_MaskSurface* Gfx_InitMaskSurface(int width, int height)
{
    CC_MaskSurface*  surface = new CC_MaskSurface;
    surface->w = width;
    surface->h = height;
    surface->clip_rect = {0, 0, width, height};
    surface->pitch = ((width + 31) / 32) * 4;
    surface->pixels = static_cast<uint8_t*>(calloc(surface->pitch, surface->h));
    
    return surface;
}


void Gfx_BlitFrontSurface( CC_RGBSurface* source, const CC_Rect* sourceSDLRect, const CC_Rect* destSDLRect )
{
    Gfx_BlitSurface( source,         sourceSDLRect,
                     g_frontSurface, destSDLRect );
    Platform_CompositeOverlays();
}



void Gfx_AcquireSurface( CC_RGBSurface* surface )
{
    if (s_acquireHead >= arrsize(s_acquireList) - 1)
        Platform_Error("Gfx_AcquireSurface: overflow");

    s_acquireList[++s_acquireHead] = surface;
    SDL_SetRenderTarget(g_renderer, surface->texture);
}


CC_RGBSurface* Gfx_GetCurrentSurface()
{
    return s_acquireList[s_acquireHead];
}


void Gfx_ReleaseSurface( CC_RGBSurface* surface )
{
    if (s_acquireHead < 0)
        Platform_Error( "Gfx_ReleaseSurface: underflow" );

    if( s_acquireList[s_acquireHead] != surface )
        Platform_Error( "Gfx_ReleaseSurface: out of order" );

    s_acquireHead--;

    // Restore the previous render target: the surface below on the stack,
    // or g_frontTexture if we just popped the last entry.
    SDL_Texture* prevTarget = (s_acquireHead >= 0)
                                  ? s_acquireList[s_acquireHead]->texture
                                  : g_frontTexture;
    SDL_SetRenderTarget(g_renderer, prevTarget);
}


void MaskRect( MRect *r )
{
    CC_Rect sdlRect;
    Gfx_MRectToCCRect( r, &sdlRect );
    Gfx_BlitFrontSurface( backdropSurface, &sdlRect, &sdlRect );
}

