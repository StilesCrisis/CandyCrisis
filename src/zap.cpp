// zap.cpp

#include "stdafx.h"

#include <stdio.h>
#include <algorithm>

#include "main.h"
#include "players.h"
#include "zap.h"
#include "grays.h"
#include "soundfx.h"
#include "gworld.h"
#include "graphics.h"
#include "gameticks.h"
#include "level.h"
#include "random.h"
#include "tweak.h"
#include "blitter.h"
#include "font.h"
#include "score.h"
#include "hiscore.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

int          chain[2];
signed char death[2][kGridAcross][kGridDown];
int zapIteration[2];
int grenadeFrame[2] = {kBlastFrames + 1, kBlastFrames + 1}, zapScoreFrame[2];
MPoint zapScorePt[2];
MRect grenadeRect[2];
SkittlesFont* zapFont;
SkittlesFont* zapOutline;
char zapScore[2][20] = { "", "" };
int zapScoreWidth[2];
int zapScoreR[2], zapScoreG[2], zapScoreB[2];
int zapOffsetX[7][kZapFrames], zapOffsetY[7][kZapFrames];

using std::min;

void ZapScoreDisplay( int player, int amount, int multiplier, int x, int y, int c )
{
	if( amount     > 0 &&
	    multiplier > 0 && 
	    x >= 0 && x < kGridAcross &&
	    y >= 0 && y < kGridDown   &&
	    c >= kFirstBlob && c <= (kLastBlob+1) )
	{
		zapScorePt[player].v = y * kBlobVertSize  + 3*(6);
		zapScorePt[player].h = x * kBlobHorizSize + 3*(6);

		zapScoreR[player] = glowColors[c][0];
		zapScoreG[player] = glowColors[c][1];
		zapScoreB[player] = glowColors[c][2];

        if (multiplier == 1)
        {
            snprintf(zapScore[player], arrsize(zapScore[player]), "%d", amount);
        }
        else
        {
            snprintf(zapScore[player], arrsize(zapScore[player]), "%d*%d", amount, multiplier);
        }

		zapScoreWidth[player] = GetTextWidth( zapFont, zapScore[player] );
		
		if( (zapScorePt[player].h + zapScoreWidth[player] + 3*(8)) > (kGridAcross * kBlobHorizSize) )
		{
			zapScorePt[player].h = (kGridAcross * kBlobHorizSize) - zapScoreWidth[player] - 3*(8);
		}
	}
}

void ZapBlobs( int player )
{
	int x, y, cluster, clusterCount = 0, multiplier, amount = 0;
	int zapFocusX = -1, zapFocusY = -1, zapFocusC = 0;
	
	
	zapScorePt[player].v = 0;
	zapScoreFrame[player] = 0;
	
	switch( chain[player] )
	{
		case 1:  multiplier = 1;                  break;
		default: multiplier = 2 << chain[player]; break;
	}
    
	for( y=kGridDown-1; y>=0; y-- )
	{
		for( x=kGridAcross-1; x>=0; x-- )
		{
			if( grid[player][x][y] >= kFirstBlob &&
				grid[player][x][y] <= kLastBlob &&
				suction[player][x][y] != kInDeath )
			{
				cluster = SizeUp( grid[player], x, y, grid[player][x][y] );
				if( cluster >= kBlobClusterSize )
				{
					clusterCount++;
					zapFocusX = x;
					zapFocusY = y;
					zapFocusC = grid[player][x][y];
					
					amount += cluster * 10;
					
					multiplier += cluster - kBlobClusterSize;
					
					RemoveBlobs( player, x, y, grid[player][x][y], 0 );
				}
			}
		}
	}

	if( clusterCount > 0 )
	{
		switch( clusterCount )
		{
			case 1:                     break;
			case 2:   multiplier += 3;  break;
			case 3:   multiplier += 6;  break;
			case 4:   multiplier += 12; break;
			default:  multiplier += 24; break;
		}

		if( multiplier > 999 ) multiplier = 999;
		CalculateGrays( 1-player, amount * multiplier / difficulty[player] );
		potentialCombo[player].value += amount * multiplier;
		
		if( players == 1 ) amount *= ((level <= kLevels)? level: 1);
		score[player] += amount * multiplier;
		
		ZapScoreDisplay( player, amount, multiplier, zapFocusX, zapFocusY, zapFocusC );

        // If the player is human...
        if (control[player] == kPlayerControl)
        {
            // Keep track of the longest chain and award achievements.
            if (chain[player] > g_allTimeLongestChain)
            {
                // Award achievements.
                switch (chain[player])
                {
                    case 3: Platform_ReportAchievement("Combo3Chain"); break;
                    case 4: Platform_ReportAchievement("Combo4Chain"); break;
                    case 5: Platform_ReportAchievement("Combo5Chain"); break;
                    case 6: Platform_ReportAchievement("Combo6Chain"); break;
                    case 7: Platform_ReportAchievement("Combo7Chain"); break;
                }
                
                g_allTimeLongestChain = chain[player];
            }

            // Keep track of the max cluster amount and award achievements.
            if (clusterCount > g_allTimeMaxClusters)
            {
                // Award achievements.
                if (g_allTimeMaxClusters < 3 && clusterCount >= 3)
                {
                    Platform_ReportAchievement("Pop3Colors");
                }

                if (g_allTimeMaxClusters < 4 && clusterCount >= 4)
                {
                    Platform_ReportAchievement("Pop4Colors");
                }

                g_allTimeMaxClusters = clusterCount;
            }
        }
    }
	
	blobTime[player] = GameTickCount( );
	
	if( clusterCount > 0 )
	{
		chain[player]++;
		role[player] = kKillBlobs;
		PlayStereoFrequency( player, kSquishy, zapIteration[player]++ );
	}
	else
	{
		if( control[player] == kPlayerControl )
		{
			SubmitCombo( &potentialCombo[player] );
		}
		
		SetupGrays( player );
		role[player] = kDropGrays;
		
		if( BusyDroppingGrays( player ) )
		{
			PlayStereoFrequency( player, kWhistle, player );
		}
	}
}

void RemoveBlobs( int player, int x, int y, int color, int generation )
{
	if( (x<0) || (x>=kGridAcross) || (y<0) || (y>=kGridDown) )
		return;
	
	if( grid[player][x][y] == kGray )
	{
		suction[player][x][y] = kGrayBlink1;
		death[player][x][y] = -8 - generation;
		return;
	}
	
	if( grid[player][x][y] != color || suction[player][x][y] == kInDeath )
		return;
	
	suction[player][x][y] = kInDeath;
	death[player][x][y] = -12 - generation;
	
	RemoveBlobs( player, x-1, y,   color, generation+3 );
	RemoveBlobs( player, x+1, y,   color, generation+3 );
	RemoveBlobs( player, x,   y-1, color, generation+3 );
	RemoveBlobs( player, x,   y+1, color, generation+3 );
}

void KillBlobs( int player )
{
	int x,y;
    static const int   shading [] =
    {
        _5TO8(20),
        _5TO8(21),
        _5TO8(22),
        _5TO8(23),
        _5TO8(24),
        _5TO8(25),
        _5TO8(26),
        _5TO8(27),
        _5TO8(28),
        _5TO8(29),
        _5TO8(30),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(31),
        _5TO8(30),
        _5TO8(29),
        _5TO8(28),
        _5TO8(26),
        _5TO8(24),
        _5TO8(21),
        _5TO8(18),
        _5TO8(15),
        _5TO8(12),
        _5TO8(9),
        _5TO8(6),
        _5TO8(3),
        _5TO8(0)
    };
	static const int
                blobGraphic[kZapFrames] = { kDying,   kDying,   kDying,   kDying,   kSquish1,
								            kSquish1, kSquish1, kSquish1, kSquish2, kSquish2, 
			 					            kSquish2, kSquish2, kSquish3, kSquish3, kSquish3,
								            kSquish3, kSquish4, kSquish4, kSquish4, kSquish4 },
		  		grayGraphic[kZapFrames] = { kGrayBlink1, kGrayBlink1, kGrayBlink1,
		  						            kGrayBlink1, kGrayBlink1, kGrayBlink1, 
		  						            kGrayBlink2, kGrayBlink2, kGrayBlink2,
		  						            kGrayBlink2, kGrayBlink2, kGrayBlink2, 
		  						            kGrayBlink3, kGrayBlink3, kGrayBlink3,
		  					            	kGrayBlink3, kGrayBlink3, kGrayBlink3, 
		  						            kGrayBlink3, kGrayBlink3 };
	MRect myRect;
	MBoolean busy = false;
	
	char *scan;
	
	if( blobTime[player] > GameTickCount( ) )
		return;
	
	blobTime[player]++;

	Gfx_AcquireSurface( playerSurface[player] );
	
	// clear grenade sprite
	if( grenadeFrame[player] <= kBlastFrames )
	{
		CleanSpriteArea( player, &grenadeRect[player] );
		if( grenadeFrame[player] == kBlastFrames ) grenadeFrame[player]++;
	}
		
	for( x=0; x<kGridAcross; x++ )
	{
		for( y=0; y<kGridDown; y++ )
		{
			if( grid[player][x][y] >= kFirstBlob &&  // if a blob is dying
				grid[player][x][y] <= kLastBlob &&
				suction[player][x][y] == kInDeath )
			{
				death[player][x][y]++;
				busy = true;
				
				CalcBlobRect( x, y, &myRect );
				
				if( death[player][x][y] >= 0 && death[player][x][y] <= kZapFrames ) // draw its death
				{					
					if( death[player][x][y] == kZapFrames )
					{
						grid[player][x][y] = kEmpty;
						suction[player][x][y] = kNoSuction;
						charred[player][x][y] = kNoCharring;
						SurfaceDrawBlob( player, &myRect, kEmpty, kNoSuction, kNoCharring );
						CleanSpriteArea( player, &myRect );
					}
					else
					{
						SurfaceDrawBlob( player, &myRect,
								         grid[player][x][y],
								         blobGraphic[ death[player][x][y] ],
								         kNoCharring );
						CleanSpriteArea( player, &myRect );
					}
					
					CleanChunks( player, x, y, death[player][x][y], character[player].zapStyle );
				}
				else
				{
					SurfaceDrawBlob( player, &myRect, grid[player][x][y],
								(blobTime[player] & 2)? kFlashDarkBlob: kNoSuction, kNoCharring );
					CleanSpriteArea( player, &myRect );
				}
			}
			else
			{
				if( grid[player][x][y] == kGray &&					// gray dying
					suction[player][x][y] == kGrayBlink1 )
				{
					CalcBlobRect( x, y, &myRect );
					
					if( death[player][x][y] >= 0 && death[player][x][y] <= kZapFrames )
					{
						if( death[player][x][y] == kZapFrames )
						{
							grid[player][x][y] = kEmpty;
							suction[player][x][y] = kNoSuction;
							SurfaceDrawBlob( player, &myRect, kEmpty, kNoSuction, kNoCharring );
						}
						else
						{
							SurfaceDrawBoard( player, &myRect );
							SurfaceDrawSprite( &myRect, kGray, grayGraphic[ death[player][x][y] ] );
							busy = true;
						}
						CleanSpriteArea( player, &myRect );
					}
					
					death[player][x][y]++;
				}
			}
		}
	}
	
	// draw score info above blobs but below chunks and explosions
	
	if( zapScoreFrame[player] < arrsize(shading) )
	{
        float position = zapScoreFrame[player];
        float lastPos  = position - 1;
        
        position = position * (position * (position * (0.0476275 - 0.000348595 * position) - 2.39227) + 52.7719) - 17.1217; // 3-scale
        lastPos  = lastPos  * (lastPos  * (lastPos  * (0.0476275 - 0.000348595 * lastPos ) - 2.39227) + 52.7719) - 17.1217; // 3-scale
        
        myRect.top    = zapScorePt[player].v - position;
		myRect.left   = zapScorePt[player].h;
		myRect.bottom = zapScorePt[player].v - lastPos + zapFont->h;
		myRect.right  = myRect.left + zapScoreWidth[player];
		CleanSpriteArea( player, &myRect );

		if( zapScoreFrame[player] < arrsize(shading)-1 )
		{		
			Gfx_AcquireSurface( playerSpriteSurface[player] );	
			
			float dx = (float)myRect.left;
			float ox = (float)myRect.left;
			int   dy = myRect.top;
			scan = zapScore[player];
			while( *scan )
			{
				SurfaceBlitWeightedCharacter( zapFont,    *scan, &dx, dy, zapScoreR[player], zapScoreG[player], zapScoreB[player], shading[zapScoreFrame[player]] );
				SurfaceBlitWeightedCharacter( zapOutline, *scan, &ox, dy, 0,                 0,                 0,                 shading[zapScoreFrame[player]] );
				scan++;
			}
			
			Gfx_ReleaseSurface( playerSpriteSurface[player] );	

			zapScoreFrame[player]++;
			busy = true;
		}	
	}
		
	///////////////////////////////////////////////////////////////

	for( x=0; x<kGridAcross; x++ )
	{
		for( y=0; y<kGridDown; y++ )
		{
			if( grid[player][x][y] >= kFirstBlob &&  // if a blob is dying
				grid[player][x][y] <= kLastBlob &&
				suction[player][x][y] == kInDeath &&
				death[player][x][y] >= 0 && death[player][x][y] < kZapFrames ) // draw chunks (after all that stuff)
			{
				DrawChunks( player, x, y, death[player][x][y], character[player].zapStyle );
			}
		}
	}
	
	Gfx_ReleaseSurface( playerSurface[player] );
	
	if( grenadeFrame[player] < kBlastFrames )
	{
		busy = true;
		
		Gfx_AcquireSurface( playerSpriteSurface[player] );
		
		myRect.top = grenadeFrame[player] * kBlastHeight;
		myRect.left = 0;
		myRect.bottom = myRect.top + kBlastHeight;
		myRect.right = kBlastWidth;
		
		SurfaceBlendOver( playerSpriteSurface[player], &grenadeRect[player], blastSurface, &myRect );

		grenadeFrame[player]++;

		Gfx_ReleaseSurface( playerSpriteSurface[player] );
	}
	
	if( !busy && role[player] == kKillBlobs )
	{
		blobTime[player] = GameTickCount( );
		halfway[player] = false;
		role[player] = kDropBlobs;
	}
}

int SizeUp( signed char myGrid[kGridAcross][kGridDown], int x, int y, int color )
{	
	int total;
	
	total = GetChainSize( myGrid, x, y, color );
	CleanSize( myGrid, x, y, color );
	
	return total;
}

int GetChainSize( signed char myGrid[kGridAcross][kGridDown], int x, int y, int color )
{
	int total;
	
	if( (x<0) || (x>=kGridAcross) || (y<0) || (y>=kGridDown) ) return 0;
	if( myGrid[x][y] != color ) return 0;

	myGrid[x][y] = -color;
	
	total = 1 + GetChainSize( myGrid, x-1, y, color )
			  + GetChainSize( myGrid, x+1, y, color )
			  + GetChainSize( myGrid, x, y-1, color )
			  + GetChainSize( myGrid, x, y+1, color );
	
	return total;
}

void CleanWithPolish( signed char myGrid[kGridAcross][kGridDown], signed char polish[kGridAcross][kGridDown], int x, int y, int color )
{
	if( (x<0) || (x>=kGridAcross) || (y<0) || (y>=kGridDown) ) return;
	
	if( myGrid[x][y] == -color )
	{
		myGrid[x][y] = color;
		polish[x][y] = true;
		
		CleanWithPolish( myGrid, polish, x-1, y, color );
		CleanWithPolish( myGrid, polish, x+1, y, color );
		CleanWithPolish( myGrid, polish, x, y-1, color );
		CleanWithPolish( myGrid, polish, x, y+1, color );
	}
}

void CleanSize( signed char myGrid[kGridAcross][kGridDown], int x, int y, int color )
{
	if( (x<0) || (x>=kGridAcross) || (y<0) || (y>=kGridDown) ) return;
	
	if( myGrid[x][y] == -color )
	{
		myGrid[x][y] = color;
		
		CleanSize( myGrid, x-1, y, color );
		CleanSize( myGrid, x+1, y, color );
		CleanSize( myGrid, x, y-1, color );
		CleanSize( myGrid, x, y+1, color );
	}
}

void CleanChunks( int player, int x, int y, int level, int style )
{
	int count, color, type;
	MRect chunkRect;
	
    Gfx_AcquireSurface( playerSpriteSurface[player] );
		
    for( count=-3; count<=3; count++ )
    {
        if( count != 0 )
        {
            if( level > 0 )
            {
                CalcBlobRect( x, y, &chunkRect );
                GetZapStyle( player, &chunkRect, &color, &type, count, level-1, style );
                CleanSpriteArea( player, &chunkRect );
            }
            
            if( level < kZapFrames )
            {
                CalcBlobRect( x, y, &chunkRect );
                GetZapStyle( player, &chunkRect, &color, &type, count, level, style );
                CleanSpriteArea( player, &chunkRect );
            }
        }
    }
    
    Gfx_ReleaseSurface( playerSpriteSurface[player] );
}

void DrawChunks( int player, int x, int y, int level, int style )
{
	int count, color, type;
	MRect chunkRect;
	
    Gfx_AcquireSurface( playerSpriteSurface[player] );
    
    for( count=-3; count<=3; count++ )
    {
        if( count != 0 )
        {
            CalcBlobRect( x, y, &chunkRect );
            color = grid[player][x][y];
            GetZapStyle( player, &chunkRect, &color, &type, count, level, style );
            SurfaceDrawSprite( &chunkRect, color, type );
        }
    }
    
    Gfx_ReleaseSurface( playerSpriteSurface[player] );
}

void CleanSplat( int player, int x, int y, int level )
{
	int count, color, type;
	MRect chunkRect;

    Gfx_AcquireSurface( playerSpriteSurface[player] );
    
    for( count=-2; count<=2; count++ )
    {
        if( count != 0 )
        {
            if( level > 0 )
            {
                CalcBlobRect( x, y, &chunkRect );
                GetZapStyle( player, &chunkRect, &color, &type, count, level-1, 4 );
                CleanSpriteArea( player, &chunkRect );
            }
            
            if( level < kZapFrames )
            {
                CalcBlobRect( x, y, &chunkRect );
                GetZapStyle( player, &chunkRect, &color, &type, count, level, 4 );
                CleanSpriteArea( player, &chunkRect );
            }
        }
    }
    
    Gfx_ReleaseSurface( playerSpriteSurface[player] );
}

void DrawSplat( int player, int x, int y, int level )
{
	int count, color = kGray, type;
	MRect chunkRect;
	
    Gfx_AcquireSurface( playerSpriteSurface[player] );
    
    for( count=-2; count<=2; count++ )
    {
        if( level < kZapFrames && count != 0 )
        {
            CalcBlobRect( x, y, &chunkRect );
            GetZapStyle( player, &chunkRect, &color, &type, count, level, 4 );
            SurfaceDrawSprite( &chunkRect, kGray, type );
        }
    }
    
    Gfx_ReleaseSurface( playerSpriteSurface[player] );
}

void InitZapStyle( void )
{
	zapFont    = GetFont( picZapFont );
	zapOutline = GetFont( picZapOutlineFont );
	
    // Initialize the upward-spark position data.
	for( int frame=0; frame<kZapFrames; frame++ )
	{
		for( int which=0; which<7; which++ )
		{
            static const int angle[7] =
            {
                -30, -50, -70, 0, -110, -130, -150
            };

            double theta = d2r(angle[which]);
            float  position =  ((0.0302892 * frame - 1.79625) * frame + 35.4627) * frame - 6.36705; // 3-scale
				
			zapOffsetX[which][frame] = (int) floorf( 0.5f + position * cosf( theta ) );
			zapOffsetY[which][frame] = (int) floorf( 0.5f + position * sinf( theta ) );
		}
	}
}


void GetZapStyle( int player, MRect *myRect, int *color, int *type, int which, int frame, int style )
{
	const int chunkGraphic[] = { kSquish1, kSquish1, kSquish1, kSquish1, kSquish1, 
								 kSquish2, kSquish2, kSquish2, kSquish2, kSquish2,
								 kSquish3, kSquish3, kSquish3, kSquish3, kSquish3,
								 kSquish4, kSquish4, kSquish4, kSquish4, kSquish4 };
	
	(void) color; // later
	*type = chunkGraphic[frame];
	
    switch (style)
	{
		case 0:
		{
            // Asterisk shape *
            
			static const int direction[7][2] = { {0, -2}, {-2,-1}, {-2,1}, {0,0}, {2,-1}, {2,1}, {0, 2} };

            float position = (-0.398496 * frame + 14.5488) * frame - 0.368421; // 3-scale
			
			OffsetMRect( myRect, direction[which+3][0] * position,
								 direction[which+3][1] * position );
			break;
		}

				
		case 1:
		{
            // Traditional fountain
            float yOffset = 0;
            switch (which)
            {
                case 1: case -1:
                    yOffset = (0.619959 * frame - 18.8523) * frame + 11.4842; // 3-scale
                    break;
                    
                case 2: case -2:
                    yOffset = (0.746583 * frame - 16.3392) * frame + 8.47632; // 3-scale
                    break;
                    
                case 3: case -3:
                    yOffset = (0.301267 * frame - 6.25212) * frame + 0.965791; // 3-scale
                    break;
            }
            
            int xOffset = frame * which * 3;
            
			OffsetMRect( myRect, _HD(xOffset), yOffset);
			
			break;
		}
		
		case 2:
		{
            // Vertical bar |
            float position = (-0.398496 * frame + 14.5488) * frame - 0.368421; // 3-scale
           
			OffsetMRect( myRect, 0, position * which );
			break;
		}
		
		case 3:
		{
            // Blown away
            
			double fFrame = ((double)frame) / 2;
            int    xOffset = (int)((player? -1.0f: 1.0f) * abs(which) * fFrame * (fFrame-1));
            int    yOffset = (int)((which-3) * fFrame);
			OffsetMRect( myRect, _HD(xOffset), _HD(yOffset));
			break;
		}
		
		case 4:
		{
            // Upward spark
			OffsetMRect( myRect, zapOffsetX[which+3][frame], zapOffsetY[which+3][frame]);
			break;
		}
	}
}
