// blitter.c

#include "stdafx.h"

#include "main.h"
#include "gworld.h"
#include "blitter.h"
#include "SDLU.h"
#include "font.h"
#include "level.h"
#include "graphics.h"
#include "Platform.h"
#include <assert.h>
#include <vector>
#include <cmath>

// Helper: convert MRect to SDL_Rect.
static SDL_Rect BL_MRectToSDL(const MRect* r)
{
    SDL_Rect sr = { r->left, r->top, r->right - r->left, r->bottom - r->top };
    return sr;
}

MBoolean update[2][kGridAcross][kGridDown];
MBoolean refresh[2];

void InitBlitter( void )
{
	int player, x, y;

	for( player=0; player<=1; player++ )
	{
		refresh[player] = false;
		
		for( x=0; x<kGridAcross; x++ )
		{
			for( y=0; y<kGridDown; y++ )
			{
				update[player][x][y] = false;
			}
		}
	}
}

void UpdatePlayerWindow( int player )
{
	CC_Rect fullSDLRect, offsetSDLRect;
	int     x, y;
	
	if( control[player] == kNobodyControl ) return;
	
	if( playerWindowVisible[player] && refresh[player] )
	{
		MRect updateRect = {0, 0, 0, 0}, fullRect, offsetRect;
		MBoolean first = true;
		
		for( x=0; x<kGridAcross; x++ )
		{
			for( y=1; y<kGridDown; y++ )
			{
				if( update[player][x][y] )
				{
					updateRect.top  = y * kBlobVertSize;
					updateRect.left = x * kBlobHorizSize;
					updateRect.bottom = updateRect.top + kBlobVertSize;
					updateRect.right = updateRect.left + kBlobHorizSize;
					if( first )
					{
						fullRect = updateRect;
						first = false;
					}
					else
					{
						UnionMRect( &fullRect, &updateRect, &fullRect );
					}

					update[player][x][y] = false;
				}
			}
		}

		if( !first )
		{
			offsetRect = fullRect;
			OffsetMRect( &offsetRect, playerWindowRect[player].left, playerWindowRect[player].top - kBlobVertSize );
			
			Gfx_BlitFrontSurface( playerSpriteSurface[player], 
								   Gfx_MRectToCCRect( &fullRect, &fullSDLRect ),
								   Gfx_MRectToCCRect( &offsetRect, &offsetSDLRect ) );
		}
	}
}

void SetUpdateRect( int player, const MRect *where )
{
	int x,y;
	int xMin, xMax, yMin, yMax;
	
	xMin = where->left / kBlobHorizSize;
	xMax = ( where->right + kBlobHorizSize - 1 ) / kBlobHorizSize;
	
	if( xMin < 0 ) xMin = 0;
	if( xMin > (kGridAcross-1) ) xMin = kGridAcross-1;
	if( xMax < 0 ) xMax = 0;
	if( xMax > kGridAcross ) xMax = kGridAcross;
	
	yMin = where->top / kBlobVertSize;
	yMax = ( where->bottom + kBlobVertSize - 1 ) / kBlobVertSize;

	if( yMin < 0 ) yMin = 0;
	if( yMin > (kGridDown-1) ) yMin = kGridDown-1;
	if( yMax < 0 ) yMax = 0;
	if( yMax > kGridDown ) yMax = kGridDown;
	
	for( x=xMin; x<xMax; x++ )
	{
		for( y=yMin; y<yMax; y++ )
		{
			update[player][x][y] = true;
		}
	}
	
	refresh[player] = true;
}


void SurfaceBlendOver(CC_RGBSurface* surface, const MRect* surfaceRect, CC_RGBSurface* foreground, const MRect* foregroundRect)
{
    // GPU path: render foreground onto surface using the texture's existing blend mode.
    // PICT-loaded textures use premultiplied blend; TARGET textures use SDL_BLENDMODE_BLEND.
    if (foreground->texture && surface->texture)
    {
        SDL_Texture* prevTarget = SDL_GetRenderTarget(g_renderer);
        SDL_SetRenderTarget(g_renderer, surface->texture);
        SDL_Rect srcR = BL_MRectToSDL(foregroundRect);
        SDL_Rect dstR = BL_MRectToSDL(surfaceRect);
        SDL_RenderCopy(g_renderer, foreground->texture, &srcR, &dstR);
        SDL_SetRenderTarget(g_renderer, prevTarget);
    }
}


void SurfaceBlendUnder(CC_RGBSurface* surface, const MRect* surfaceRect, CC_RGBSurface* background, const MRect* backgroundRect)
{
    // GPU path: render background UNDER surface's existing content using the blend-under mode.
    // Where surface->texture is transparent (alpha=0), background fills in; opaque areas are kept.
    if (background->texture && surface->texture)
    {
        SDL_Texture* prevTarget = SDL_GetRenderTarget(g_renderer);
        SDL_SetRenderTarget(g_renderer, surface->texture);
        SDL_Rect srcR = BL_MRectToSDL(backgroundRect);
        SDL_Rect dstR = BL_MRectToSDL(surfaceRect);
        Gfx_RenderCopyBlend(background->texture, Gfx_BlendUnderMode(), &srcR, &dstR);
        SDL_SetRenderTarget(g_renderer, prevTarget);
    }
}


void SurfaceBlitBlob( const MRect* blobRect, const MRect* destRect )
{
    // GPU path: blobSurface->texture has mask alpha baked in (X=255 where mask=1, X=0 where mask=0).
    // The render target is already set to the current surface by Gfx_AcquireSurface.
    CC_RGBSurface* dest = Gfx_GetCurrentSurface();
    if (blobSurface->texture && dest && dest->texture)
    {
        SDL_Rect srcR = BL_MRectToSDL(blobRect);
        SDL_Rect dstR = BL_MRectToSDL(destRect);
        // Explicit clip to destination surface bounds, matching what SurfaceBlitMask does.
        if (dstR.y < 0)                         { int d = -dstR.y;                    srcR.y += d; srcR.h -= d; dstR.h -= d; dstR.y = 0; }
        if (dstR.x < 0)                         { int d = -dstR.x;                    srcR.x += d; srcR.w -= d; dstR.w -= d; dstR.x = 0; }
        if (dstR.y + dstR.h > dest->h)          { int d = dstR.y + dstR.h - dest->h;  srcR.h -= d; dstR.h -= d; }
        if (dstR.x + dstR.w > dest->w)          { int d = dstR.x + dstR.w - dest->w;  srcR.w -= d; dstR.w -= d; }
        if (dstR.w > 0 && dstR.h > 0)
            SDL_RenderCopy(g_renderer, blobSurface->texture, &srcR, &dstR);
    }
}


void SurfaceBlitColor( CC_MaskSurface* mask,  CC_RGBSurface* dest,
                       const MRect* maskRect, const MRect* destRect,
                       int r, int g, int b, int weight )
{
    // GPU path: mask->texture is white with alpha=255 where mask=1, alpha=0 elsewhere.
    // Apply color and alpha mods then blit with BLEND so only masked pixels are tinted.
    if (mask->texture && dest && dest->texture)
    {
        SDL_Texture* prevTarget = SDL_GetRenderTarget(g_renderer);
        SDL_SetRenderTarget(g_renderer, dest->texture);
        SDL_Rect srcR = BL_MRectToSDL(maskRect);
        SDL_Rect dstR = BL_MRectToSDL(destRect);
        SDL_SetTextureColorMod(mask->texture, (uint8_t)r, (uint8_t)g, (uint8_t)b);
        SDL_SetTextureAlphaMod(mask->texture, (uint8_t)weight);
        SDL_RenderCopy(g_renderer, mask->texture, &srcR, &dstR);
        SDL_SetTextureColorMod(mask->texture, 255, 255, 255);
        SDL_SetTextureAlphaMod(mask->texture, 255);
        SDL_SetRenderTarget(g_renderer, prevTarget);
    }
}


void SurfaceBlitWeightedCharacter( SkittlesFont* font, unsigned char text, float* dx, int dy, int r, int g, int b, int alpha, float scale )
{
    if (alpha == 0) return;

    int   width  = font->width[text];
    int   height = font->h;
    float sw     = width * scale;
    float sh     = height * scale;

    SDL_Rect  srcR = { font->across[text], 0, width, height };
    SDL_FRect dstR = { *dx, (float)dy, sw, sh };
    SDL_SetTextureColorMod(font->texture, (uint8_t)r, (uint8_t)g, (uint8_t)b);
    SDL_SetTextureAlphaMod(font->texture, (uint8_t)(alpha > 255 ? 255 : alpha));
    SDL_RenderCopyF(g_renderer, font->texture, &srcR, &dstR);
    SDL_SetTextureColorMod(font->texture, 255, 255, 255);
    SDL_SetTextureAlphaMod(font->texture, 255);
    *dx += sw;
}

void BlitString(SkittlesFont* font, const char* text, float* x, int y, int r, int g, int b, int alpha, float scale)
{
    for (const char* c = text; *c; ++c)
        SurfaceBlitWeightedCharacter(font, (unsigned char)*c, x, y, r, g, b, alpha, scale);
}

void SurfaceBlitCharacter( SkittlesFont*   font, unsigned char text, MPoint *dPoint, int r, int g, int b, int dropShadow )
{
    int width  = font->width[text];
    int height = font->h;
    SDL_Rect srcR = { font->across[text], 0, width, height };
    SDL_Rect dstR = { dPoint->h, dPoint->v, width, height };
    if (dropShadow > 0)
    {
        // Shadow pass: black character, shifted diagonally by dropShadow pixels.
        SDL_Rect shadowR = { dPoint->h + dropShadow, dPoint->v + dropShadow, width, height };
        SDL_SetTextureColorMod(font->texture, 0, 0, 0);
        SDL_RenderCopy(g_renderer, font->texture, &srcR, &shadowR);
    }
    SDL_SetTextureColorMod(font->texture, (uint8_t)r, (uint8_t)g, (uint8_t)b);
    SDL_RenderCopy(g_renderer, font->texture, &srcR, &dstR);
    SDL_SetTextureColorMod(font->texture, 255, 255, 255);
    dPoint->h += width;
}

void SurfaceBlitScaledCharacter( SkittlesFont* font, unsigned char text, float* dx, float dy, float scale, int r, int g, int b, int dropShadow, int alpha )
{
    int   width   = font->width[text];
    int   height  = font->h;
    float sw      = width * scale;
    float sh      = height * scale;

    SDL_Rect  srcR = { font->across[text], 0, width, height };
    SDL_FRect dstR = { *dx, dy, sw, sh };
    if (dropShadow > 0)
    {
        float sd = dropShadow * scale;
        SDL_FRect shadowR = { *dx + sd, dy + sd, sw, sh };
        SDL_SetTextureColorMod(font->texture, 0, 0, 0);
        SDL_SetTextureAlphaMod(font->texture, (uint8_t)alpha);
        SDL_RenderCopyF(g_renderer, font->texture, &srcR, &shadowR);
    }
    SDL_SetTextureColorMod(font->texture, (uint8_t)r, (uint8_t)g, (uint8_t)b);
    SDL_SetTextureAlphaMod(font->texture, (uint8_t)alpha);
    SDL_RenderCopyF(g_renderer, font->texture, &srcR, &dstR);
    SDL_SetTextureColorMod(font->texture, 255, 255, 255);
    SDL_SetTextureAlphaMod(font->texture, 255);
    *dx += sw;
}

void SurfaceBlitCursor( MPoint mouseHere, CC_RGBSurface* surface )
{
    if (Platform_IsTouchDevice()) return;

    SkittlesFont* cursorFont = GetFont( picFont );
    MPoint mouseHereToo = mouseHere;

    Gfx_AcquireSurface( surface );
    SurfaceBlitCharacter( cursorFont, '\x05', &mouseHere,    0,   0,   0,   0 );
    SurfaceBlitCharacter( cursorFont, '\x04', &mouseHereToo, 255, 255, 255,  0 );
    Gfx_ReleaseSurface( surface );
}

void SurfaceBlitColorOver( CC_RGBSurface* source,   CC_RGBSurface* dest,
                           const MRect* sourceRect, const MRect* destRect,
                           int r, int g, int b, int weight )
{
    // GPU path: two passes.
    // Pass 1: fill dest rect with solid (r,g,b).
    // Pass 2: composite source over it with alpha = (255 - weight), so the result is:
    //   dest' = source * (1 - weight/255) + color * (weight/255)
    if (source->texture && dest && dest->texture)
    {
        SDL_Texture* prevTarget = SDL_GetRenderTarget(g_renderer);
        SDL_SetRenderTarget(g_renderer, dest->texture);
        SDL_Rect dstR = BL_MRectToSDL(destRect);
        SDL_Rect srcR = BL_MRectToSDL(sourceRect);

        // Pass 1: fill with color.
        SDL_BlendMode prevMode;
        SDL_GetRenderDrawBlendMode(g_renderer, &prevMode);
        SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(g_renderer, (uint8_t)r, (uint8_t)g, (uint8_t)b, 255);
        SDL_RenderFillRect(g_renderer, &dstR);
        SDL_SetRenderDrawBlendMode(g_renderer, prevMode);

        // Pass 2: source blended over the color fill.
        Gfx_RenderCopyBlend(source->texture, SDL_BLENDMODE_BLEND, &srcR, &dstR, (uint8_t)(255 - weight));
        SDL_SetRenderTarget(g_renderer, prevTarget);
    }
}

void SurfaceBlitBlendOver( CC_RGBSurface* source,   CC_RGBSurface* dest,
                           const MRect* sourceRect, const MRect* destRect,
                           int r1, int g1, int b1,
                           int r2, int g2, int b2,
                           int r3, int g3, int b3,
                           int r4, int g4, int b4,
                           int weight )
{
	if( destRect->left   > dest->w ||			// completely clipped?
		destRect->right  < 0       ||
		destRect->top    > dest->h ||
		destRect->bottom < 0 )
	{
		return;
	}

    if (source->texture && dest && dest->texture)
    {
        SDL_Texture* prevTarget = SDL_GetRenderTarget(g_renderer);
        SDL_SetRenderTarget(g_renderer, dest->texture);

        // Step 1: copy source background into dest region (straight copy).
        {
            SDL_Rect srcR = BL_MRectToSDL(sourceRect);
            SDL_Rect dstR = BL_MRectToSDL(destRect);
            Gfx_RenderCopyBlend(source->texture, SDL_BLENDMODE_NONE, &srcR, &dstR);
        }

        // Step 2: overlay the bilinear gradient using SDL_RenderGeometry (SDL 2.0.18+).
        // Each vertex carries one corner colour; SDL interpolates smoothly across triangles.
        // Vertex alpha = weight → BLEND mode mixes gradient over the background.
        {
            SDL_BlendMode prevMode;
            SDL_GetRenderDrawBlendMode(g_renderer, &prevMode);
            SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
            const uint8_t a = (uint8_t)weight;
            SDL_Vertex verts[4] = {
                {{(float)destRect->left,  (float)destRect->top},    {(uint8_t)r1,(uint8_t)g1,(uint8_t)b1,a}, {0,0}}, // TL
                {{(float)destRect->right, (float)destRect->top},    {(uint8_t)r2,(uint8_t)g2,(uint8_t)b2,a}, {0,0}}, // TR
                {{(float)destRect->left,  (float)destRect->bottom}, {(uint8_t)r3,(uint8_t)g3,(uint8_t)b3,a}, {0,0}}, // BL
                {{(float)destRect->right, (float)destRect->bottom}, {(uint8_t)r4,(uint8_t)g4,(uint8_t)b4,a}, {0,0}}, // BR
            };
            const int indices[6] = {0, 1, 2,  1, 3, 2};
            SDL_RenderGeometry(g_renderer, nullptr, verts, 4, indices, 6);
            SDL_SetRenderDrawBlendMode(g_renderer, prevMode);
        }

        SDL_SetRenderTarget(g_renderer, prevTarget);
    }
}

void SurfaceGetEdges( CC_RGBSurface* edgeSurface, const MRect *rect, SurfaceEdgeData& edge )
{
    if (edgeSurface->texture)
    {
        // GPU path: read only the 4 corner blocks directly into edge[][].
        // Uses targeted SDL_RenderReadPixels instead of a full-surface readback.
        // If edgeSurface is already the active render target (called from inside
        // Gfx_AcquireSurface) we skip the redundant SetRenderTarget; SDL_RenderReadPixels
        // always flushes pending commands before reading, so the backdrop blit is safe.
        SDL_Texture* prev = SDL_GetRenderTarget(g_renderer);
        if (prev != edgeSurface->texture)
            SDL_SetRenderTarget(g_renderer, edgeSurface->texture);

        const int cx[4] = { rect->left, rect->right - kEdgeSize, rect->left,              rect->right - kEdgeSize };
        const int cy[4] = { rect->top,  rect->top,               rect->bottom - kEdgeSize, rect->bottom - kEdgeSize };

        for (int count = 0; count < 4; count++)
        {
            SDL_Rect cr = { cx[count], cy[count], kEdgeSize, kEdgeSize };
            SDL_RenderReadPixels(g_renderer, &cr, SDL_PIXELFORMAT_ARGB8888,
                                 edge[count], kEdgeSize * BYTES_PER_PIXEL);
        }

        if (prev != edgeSurface->texture)
            SDL_SetRenderTarget(g_renderer, prev);
    }
}


void SurfaceCurveEdges( CC_RGBSurface* edgeSurface, const MRect *rect, SurfaceEdgeData& edge )
{
    if (edgeSurface->texture)
    {
        // GPU path: express all border and corner effects as GPU draw calls.
        // This avoids SDL_UpdateTexture entirely, which would conflict with D3D11's
        // render-target state and corrupt GPU-rendered content (e.g. the white fill).
        //
        // When called from inside Gfx_AcquireSurface(edgeSurface) the texture is already
        // the active render target — the SetRenderTarget below is a no-op in that case,
        // so there is no rebind and no risk of D3D11 discarding prior renders.
        SDL_Texture* prev = SDL_GetRenderTarget(g_renderer);
        if (prev != edgeSurface->texture)
            SDL_SetRenderTarget(g_renderer, edgeSurface->texture);

        SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_NONE);

        const int W      = rect->right  - rect->left;
        const int H      = rect->bottom - rect->top;
        const int innerW = W - 2 * kEdgeSize;
        const int innerH = H - 2 * kEdgeSize;

        // Half-bright white: HALFBRIGHT_MASK of 0xFFFFFFFF = 0x007F7F7F
        // SDL draw color (R,G,B,A) = (127,127,127,0)
        const Uint8 hbR = 127, hbG = 127, hbB = 127, hbA = 0;

        auto fillRect = [&](int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
        {
            if (w <= 0 || h <= 0) return;
            SDL_SetRenderDrawColor(g_renderer, r, g, b, a);
            SDL_Rect sr = { x, y, w, h };
            SDL_RenderFillRect(g_renderer, &sr);
        };

        // Top border strip (between the corner blocks)
        fillRect(rect->left + kEdgeSize, rect->top,     innerW, 1, 0,   0,   0,   0);    // outer: transparent
        fillRect(rect->left + kEdgeSize, rect->top + 1, innerW, 1, hbR, hbG, hbB, hbA); // inner: half-bright white
        // Bottom border strip
        fillRect(rect->left + kEdgeSize, rect->bottom - 1, innerW, 1, 0,   0,   0,   0);
        fillRect(rect->left + kEdgeSize, rect->bottom - 2, innerW, 1, hbR, hbG, hbB, hbA);
        // Left border strip
        fillRect(rect->left,     rect->top + kEdgeSize, 1, innerH, 0,   0,   0,   0);
        fillRect(rect->left + 1, rect->top + kEdgeSize, 1, innerH, hbR, hbG, hbB, hbA);
        // Right border strip
        fillRect(rect->right - 1, rect->top + kEdgeSize, 1, innerH, 0,   0,   0,   0);
        fillRect(rect->right - 2, rect->top + kEdgeSize, 1, innerH, hbR, hbG, hbB, hbA);

        // Corner pixel draws.
        // [0]=top-left, [1]=top-right, [2]=bottom-left, [3]=bottom-right
        const int cx[4] = { rect->left, rect->right - kEdgeSize, rect->left,              rect->right - kEdgeSize };
        const int cy[4] = { rect->top,  rect->top,               rect->bottom - kEdgeSize, rect->bottom - kEdgeSize };

        static const char edgeMap[4][kEdgeSize][kEdgeSize+1] = {
            "      --",
            "    -...",
            "   -.xxX",
            "  -.xXXX",
            " -.xXXXX",
            " .xXXXXX",
            "-.xXXXXX",
            "-.XXXXXX",
            "--      ",
            "...-    ",
            "Xxx.-   ",
            "XXXx.-  ",
            "XXXXx.- ",
            "XXXXXx. ",
            "XXXXXx.-",
            "XXXXXX.-",
            "-.XXXXXX",
            "-.xXXXXX",
            " .xXXXXX",
            " -.xXXXX",
            "  -.xXXX",
            "   -.xxX",
            "    -...",
            "      --",
            "XXXXXX.-",
            "XXXXXx.-",
            "XXXXXx. ",
            "XXXXx.- ",
            "XXXx.-  ",
            "Xxx.-   ",
            "...-    ",
            "--      " };

        for (int count = 0; count < 4; count++)
        {
            for (int h = 0; h < kEdgeSize; h++)
            {
                for (int w = 0; w < kEdgeSize; w++)
                {
                    COLOR_T bgPixel = edge[count][h][w];
                    Uint8 bR = (bgPixel >> 16) & 0xFF;
                    Uint8 bG = (bgPixel >>  8) & 0xFF;
                    Uint8 bB =  bgPixel        & 0xFF;
                    Uint8 bA = (bgPixel >> 24) & 0xFF;
                    Uint8 dr, dg, db, da;
                    switch (edgeMap[count][h][w])
                    {
                        case 'X': continue;                           // keep white fill — no draw
                        case 'x': dr=hbR; dg=hbG; db=hbB; da=hbA; break; // half-bright white
                        case '.': dr=0;    dg=0;    db=0;    da=0;   break; // transparent
                        case '-': dr=bR/2; dg=bG/2; db=bB/2; da=0;  break; // half-bright backdrop
                        case ' ': dr=bR;   dg=bG;   db=bB;   da=bA; break; // full backdrop
                        default:  continue;
                    }
                    SDL_SetRenderDrawColor(g_renderer, dr, dg, db, da);
                    SDL_RenderDrawPoint(g_renderer, cx[count] + w, cy[count] + h);
                }
            }
        }

        if (prev != edgeSurface->texture)
            SDL_SetRenderTarget(g_renderer, prev);
    }
}

void SurfaceDrawHighlightedButton(CC_RGBSurface* src, CC_RGBSurface* dst, MRect srcRect, int glow, int baseBrightness)
{
    if (!src->texture || !dst->texture) return;

    // Calculate scale (0.0 to 0.05 increase)
    float scale = 1.0f + 0.05f * (glow / 255.0f);
    
    int w = srcRect.right - srcRect.left;
    int h = srcRect.bottom - srcRect.top;
    int dw = (int)(w * scale);
    int dh = (int)(h * scale);
    int dx = srcRect.left - (dw - w) / 2;
    int dy = srcRect.top  - (dh - h) / 2;
    
    SDL_Rect srcR = { srcRect.left, srcRect.top, w, h };
    SDL_Rect dstR = { dx, dy, dw, dh };

    // Save state
    SDL_Texture* prevTarget = SDL_GetRenderTarget(g_renderer);
    SDL_BlendMode prevSrcBlend;
    SDL_GetTextureBlendMode(src->texture, &prevSrcBlend);
    SDL_BlendMode prevDrawBlend;
    SDL_GetRenderDrawBlendMode(g_renderer, &prevDrawBlend);
    
    SDL_SetRenderTarget(g_renderer, dst->texture);

    // 1. Erase the original button area with black.
    // Since the background is black, this effectively removes the old button.
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(g_renderer, &srcR);

    // 2. Draw the scaled button additively.
    // The source image has a black background, so ADD works perfectly to composite
    // the button onto the black destination we just cleared.
    SDL_SetTextureBlendMode(src->texture, SDL_BLENDMODE_ADD);
    SDL_SetTextureColorMod(src->texture, (Uint8)baseBrightness, (Uint8)baseBrightness, (Uint8)baseBrightness);
    SDL_RenderCopy(g_renderer, src->texture, &srcR, &dstR);
    // Flush before restoring color mod: Step 3 uses the same texture with reset color mod,
    // so we must ensure Step 2's RenderCopy executes with the modulated color.
    SDL_RenderFlush(g_renderer);
    SDL_SetTextureColorMod(src->texture, 255, 255, 255);

    // 3. Add the glow (draw again with alpha modulation).
    if (glow > 0)
    {
        SDL_SetTextureAlphaMod(src->texture, (Uint8)glow);
        SDL_RenderCopy(g_renderer, src->texture, &srcR, &dstR);
    }

    // Restore state — target switch first to flush all pending commands,
    // then restore texture mods so they execute with the right values.
    SDL_SetRenderDrawBlendMode(g_renderer, prevDrawBlend);
    SDL_SetRenderTarget(g_renderer, prevTarget);  // flush before restoring mods
    SDL_SetTextureAlphaMod(src->texture, 255);
    SDL_SetTextureBlendMode(src->texture, prevSrcBlend);
}

// Creates a white capsule (stadium/pill shape) texture using an SDF for smooth AA edges.
// Blend mode is SDL_BLENDMODE_BLEND; animate with SDL_SetTextureColorMod/AlphaMod.
SDL_Texture* CreateCapsuleTexture(int w, int h)
{
    std::vector<Uint32> pixels(w * h);
    float rad     = h * 0.5f;
    float feather = 1.0f;

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            float px = x + 0.5f;
            float py = y + 0.5f;
            float cx = fmaxf(rad, fminf(w - rad, px));
            float cy = rad;
            float dx = px - cx, dy = py - cy;
            float sdf = sqrtf(dx*dx + dy*dy) - rad;
            float cov = fmaxf(0.0f, fminf(1.0f, (feather - sdf) / (2.0f * feather)));
            Uint8 a = (Uint8)(cov * 255.0f);
            pixels[y * w + x] = ((Uint32)a << 24) | 0x00FFFFFFu;  // premultiplied white
        }
    }

    SDL_Texture* tex = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STATIC, w, h);
    if (!tex) return nullptr;
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_UpdateTexture(tex, nullptr, pixels.data(), w * (int)sizeof(Uint32));
    return tex;
}

void DrawCenteredText(SkittlesFont* font, const char* text, const MRect& rect, int r, int g, int b, int shadow)
{
    if (!font || !text || !*text) return;
    MPoint pt;
    pt.h = (short)(rect.left + ((rect.right  - rect.left) - GetTextWidth(font, text)) / 2);
    pt.v = (short)(rect.top  + ((rect.bottom - rect.top)  - font->h) / 2);
    for (const char* c = text; *c; ++c)
        SurfaceBlitCharacter(font, (unsigned char)*c, &pt, r, g, b, shadow);
}

void DrawScaledCenteredText(SkittlesFont* font, const char* text, const MRect& rect, float scale, int r, int g, int b, int shadow, int alpha)
{
    if (!font || !text || !*text) return;
    float textW = GetTextWidth(font, text) * scale;
    float textH = font->h * scale;
    float dx = rect.left + ((rect.right - rect.left) - textW) * 0.5f;
    float dy = rect.top  + ((rect.bottom - rect.top) - textH) * 0.5f;
    for (const char* c = text; *c; ++c)
        SurfaceBlitScaledCharacter(font, (unsigned char)*c, &dx, dy, scale, r, g, b, shadow, alpha);
}
