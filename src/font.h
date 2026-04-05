// font.h


#pragma once


struct SDL_Texture;

struct SkittlesFont
{
	SDL_Texture *texture;
	int h;
	int width[256];
	int across[256];
};


void          InitFont( void );
SkittlesFont* GetFont( int pictID );
int           GetTextWidth( SkittlesFont* font, const char *text );

