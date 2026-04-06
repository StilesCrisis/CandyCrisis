// font.c

#include "stdafx.h"

#include "main.h"
#include "font.h"
#include "gworld.h"
#include "SDLU.h"


constexpr int kNumFonts = (picLastFont - picFont);

static SkittlesFont s_font[kNumFonts] = {};


static SkittlesFont* LoadFont( SkittlesFont* font, int pictID, unsigned char* letterMap )
{
    int fw, fh;
    uint8_t* fontPixels = LoadPICTAlphaPixels(pictID, "", "", &fw, &fh);
    if (!fontPixels)
    {
        Platform_Error( "LoadFont: files are missing or corrupt" );
        return NULL;
    }

    int      pitch    = fw;
    uint8_t* lastLine = fontPixels + pitch * (fh - 1);

    font->h = fh;

    // Step 1: parse the marker row.
    // Bright pixels (>= 0xC0) mark glyph regions; dark pixels (< 0xC0) are separators.
    // The initial bright run defines the per-glyph margin ("skip").
    int across = 0;
    while (across < fw && lastLine[across] >= 0xC0) across++;
    int skip = across;

    // Step 2: collect per-glyph info and find tight ink bounds within each drawn region.
    struct GlyphInfo {
        unsigned char letter;
        int srcX;      // x of tight ink region in original image
        int srcW;      // tight ink width (0 for glyphs with no ink, e.g. space)
        int bearingX;  // pixels from advance-box left edge to first ink column
        int advanceW;  // original advance width (preserves character spacing)
        int dstX;      // x in repacked texture (computed in step 3)
    };

    GlyphInfo glyphs[256];
    int       numGlyphs = 0;

    unsigned char* lm = letterMap;
    while (*lm && across < fw)
    {
        while (across < fw && lastLine[across] < 0xC0) across++;  // skip separator
        if (across >= fw) break;

        int regionStart = across;
        while (across < fw && lastLine[across] >= 0xC0) across++;  // measure glyph region
        int regionEnd = across;

        int drawnX0  = regionStart + (skip / 2);
        int advanceW = regionEnd - regionStart - skip;
        int drawnX1  = drawnX0 + advanceW;

        // Find tight ink bounds within the drawn region [drawnX0, drawnX1).
        // Font pixels: 0 = opaque ink, 255 = transparent background.
        int tightLeft  = drawnX1;
        int tightRight = drawnX0 - 1;
        for (int x = drawnX0; x < drawnX1; x++)
        {
            for (int y = 0; y < fh - 1; y++)
            {
                if (fontPixels[y * pitch + x] < 0xF0)  // any meaningful ink
                {
                    if (x < tightLeft)  tightLeft  = x;
                    if (x > tightRight) tightRight = x;
                    break;
                }
            }
        }

        GlyphInfo& g = glyphs[numGlyphs++];
        g.letter   = *lm;
        g.advanceW = advanceW;
        if (tightLeft <= tightRight)
        {
            g.srcX     = tightLeft;
            g.srcW     = tightRight - tightLeft + 1;
            g.bearingX = tightLeft - drawnX0;
        }
        else
        {
            // No ink (e.g. space character): zero-width draw, full advance.
            g.srcX     = drawnX0;
            g.srcW     = 0;
            g.bearingX = 0;
        }
        g.dstX = 0;
        lm++;
    }

    // Step 3: compute packed positions with 1-pixel separators.
    int packedWidth = 1;  // 1px left margin
    for (int i = 0; i < numGlyphs; i++)
    {
        glyphs[i].dstX = packedWidth;
        packedWidth += glyphs[i].srcW + 1;
    }

    // Step 4: build repacked pixel buffer.
    // Initialize to 255 (no ink = transparent in the final texture).
    uint8_t* packed = (uint8_t*)malloc(packedWidth * fh);
    memset(packed, 255, packedWidth * fh);
    for (int i = 0; i < numGlyphs; i++)
    {
        const GlyphInfo& g = glyphs[i];
        if (g.srcW == 0) continue;
        for (int y = 0; y < fh; y++)
            memcpy(packed + y * packedWidth + g.dstX,
                   fontPixels + y * pitch + g.srcX,
                   g.srcW);
    }

    // Step 5: build GPU texture from repacked pixels.
    if (packedWidth > 4096)
        Platform_Error( "LoadFont: repacked texture exceeds 4096px — add more font image rows" );
    // Pixel format: white (0xFFFFFF) with alpha = 255 - coverage.
    if (g_renderer)
    {
        SDL_Surface* texData = SDL_CreateRGBSurface(0, packedWidth, fh, 32,
                                                     RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK);
        for (int y = 0; y < fh; y++)
        {
            const uint8_t* src = packed + y * packedWidth;
            uint32_t*      dst = (uint32_t*)((uint8_t*)texData->pixels + y * texData->pitch);
            for (int x = 0; x < packedWidth; x++)
            {
                uint8_t alpha = 255 - src[x];
                dst[x] = ((uint32_t)alpha << BITS_PER_3CHANNELS) | 0x00FFFFFF;
            }
        }
#if 0 && !defined(__EMSCRIPTEN__)
        {
            char path[64];
            snprintf(path, sizeof(path), "/tmp/font_packed_%d.png", pictID);
            IMG_SavePNG(texData, path);
        }
#endif
        font->texture = SDL_CreateTextureFromSurface(g_renderer, texData);
        SDL_FreeSurface(texData);
        SDL_SetTextureBlendMode(font->texture, SDL_BLENDMODE_BLEND);
    }

    // Step 6: store font metrics.
    for (int i = 0; i < numGlyphs; i++)
    {
        const GlyphInfo& g = glyphs[i];
        font->across  [g.letter] = g.dstX;
        font->width   [g.letter] = g.advanceW;
        font->srcWidth [g.letter] = g.srcW;
        font->bearingX [g.letter] = g.bearingX;
    }

    free(packed);
    free(fontPixels);
    return font;
}


void InitFont( void ) 
{
	LoadFont( &s_font[0],  picFont, (unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!:,.()*?0123456789'|-\x01\x02\x03\x04\x05\x06\x07 " );
	LoadFont( &s_font[1],  picSkiaFont, (unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*().,/-=_+<>?|'\":; " );
	LoadFont( &s_font[2],  picContinueFont, (unsigned char*) "0123456789" );
//	LoadFont( &s_font[3],  picBalloonFont, (unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+;:,./<>? \x06\x07'\"" );
	LoadFont( &s_font[4],  picZapFont, (unsigned char*) "0123456789*PS" );
	LoadFont( &s_font[5],  picZapOutlineFont, (unsigned char*) "0123456789*" );
	LoadFont( &s_font[6],  picVictoryFont, (unsigned char*) "AB" );
	LoadFont( &s_font[7],  picDashedLineFont, (unsigned char*) "." );
	LoadFont( &s_font[8],  picSkiaBoldFont, (unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*().,/-=_+<>?|'\":; \x01\x02\x03" );
}


SkittlesFont*   GetFont( int pictID )
{
	int fontID = pictID - picFont;
	
	if( (fontID < 0) || (fontID >= kNumFonts) )
		Platform_Error( "GetFont: fontID" );
			
	return &s_font[fontID];
} 


int GetTextWidth( SkittlesFont*   font, const char *text )
{
	int width = 0;
	while( *text )
	{
		width += font->width[*text++];
	}
	
	return width;
}

