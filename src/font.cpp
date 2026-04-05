// font.c

#include "stdafx.h"

#include "main.h"
#include "font.h"
#include "gworld.h"
#include "SDLU.h"


constexpr int kNumFonts = (picDashedLineFont - picFont + 1);

static SkittlesFont s_font[kNumFonts] = {};


static SkittlesFont*   LoadFont( SkittlesFont*   font, int pictID, unsigned char *letterMap )
{
	unsigned char*     lastLine;
	MBoolean           success = false;
	int                start, across, skip;

	int fw, fh;
	uint8_t* fontPixels = LoadPICTAlphaPixels(pictID, "", "", &fw, &fh);

	if (fontPixels == NULL)
    {
        Platform_Error( "LoadFont: files are missing or corrupt" );
        return NULL;
    }

    int pitch = fw;  // 1 byte per pixel, no padding

    // Build a GPU texture from the alpha pixels: white pixels, alpha = 255 - coverage.
    // coverage 0 = fully opaque character pixel; coverage 255 = fully transparent.
    if (g_renderer)
    {
        SDL_Surface* texData = SDL_CreateRGBSurface(0, fw, fh, 32, RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK);
        for (int y = 0; y < fh; y++)
        {
            const uint8_t* src = fontPixels + y * pitch;
            uint32_t*      dst = (uint32_t*)((uint8_t*)texData->pixels + y * texData->pitch);
            for (int x = 0; x < fw; x++)
            {
                uint8_t alpha = 255 - src[x];   // invert: 0 coverage → fully opaque
                dst[x] = ((uint32_t)alpha << BITS_PER_3CHANNELS) | 0x00FFFFFF;
            }
        }
        font->texture = SDL_CreateTextureFromSurface(g_renderer, texData);
        SDL_FreeSurface(texData);
        SDL_SetTextureBlendMode(font->texture, SDL_BLENDMODE_BLEND);
    }

    font->h  = fh;
    lastLine = fontPixels + pitch * (fh - 1);
    across   = 0;

    // Measure empty space between character breaks
    while( lastLine[across] >= 0xC0 ) across++;
    skip = across;

    success = true;

    // Measure character starts and widths
    while( *letterMap )
    {
        while( lastLine[across] < 0xC0 ) across++;
        if( across > pitch )
        {
            success = false;
            break;
        }

        start = across;
        font->across[*letterMap] = across + (skip/2);

        while( lastLine[across] >= 0xC0 ) across++;
        font->width [*letterMap] = across - start - skip;

        letterMap++;
    }

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
	LoadFont( &s_font[7],  picBubbleFont, (unsigned char*) "*" );
	LoadFont( &s_font[8],  picSkiaBoldFont, (unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*().,/-=_+<>?|'\":; \x01\x02\0x03" );
	LoadFont( &s_font[9],  picDashedLineFont, (unsigned char*) "." );
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

