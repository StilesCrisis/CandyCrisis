// font.h


#pragma once


struct SDL_Texture;

struct SkittlesFont
{
	SDL_Texture* texture;
	int h;
	int width[256];    // advance width (cursor movement)
	int across[256];   // x position in repacked texture
	int srcWidth[256]; // tight ink draw width (may be less than width for sparse glyphs)
	int bearingX[256]; // pixel offset from advance-box left edge to first ink column
};


void          InitFont( void );
SkittlesFont* GetFont( int pictID );
int           GetTextWidth( SkittlesFont* font, const char *text );

