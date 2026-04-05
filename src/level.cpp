// level.c

#include "stdafx.h"

#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <vector>

#include "main.h"
#include "level.h"
#include "score.h"
#include "random.h"
#include "grays.h"
#include "gameticks.h"
#include "players.h"
#include "graymonitor.h"
#include "opponent.h"
#include "gworld.h"
#include "graphics.h"
#include "music.h"
#include "control.h"
#include "tweak.h"
#include "soundfx.h"
#include "next.h"
#include "hiscore.h"
#include "victory.h"
#include "blitter.h"
#include "zap.h"
#include "keyselect.h"
#include "tutorial.h"
#include "pause.h"
#include "UI_VictoryScreen.h"
#include "UI_GameOverScreen.h"
#include <algorithm>

const int kEasyLevel   = 50;
const int kMediumLevel = 70;
const int kHardLevel   = 90;
const int kUltraLevel  = 110;

static std::vector<Character>   s_characterList;
MRect                           stageWindowZRect, stageWindowRect;
bool                            stageWindowVisible = true;
int                             stageTime, stageFrame;
CC_RGBSurface*                  stageMapSurface;

Character character[2];
int level, players, credits, difficulty[2] = { kMediumLevel, kMediumLevel };
int difficultyTicks, backdropTicks, backdropFrame;
int backgroundID = -1;

MBoolean     playerWindowVisible[2] = { true, true };

const int k_stageMapHeight = 256;
const int k_stageMapWidth = 256;

void InitStage( void )
{
    stageWindowZRect = { 0, 0, k_stageMapHeight, k_stageMapWidth };
    stageMapSurface = Gfx_InitRGBSurface(k_stageMapWidth, k_stageMapHeight);
    
    stageWindowRect = stageWindowZRect;
}

void UpdateStage()
{
    if( GameTickCount( ) >= stageTime )
    {
        stageTime = GameTickCount() + 15;
        
        stageFrame++;
        DrawStage();
    }
}

void DrawStage()
{
    if (stageWindowVisible && stageMapSurface != nullptr)
    {
        int numStageFrames = stageMapSurface->w / stageWindowZRect.right;
        int frameToDraw    = stageFrame % numStageFrames;

        MRect  stageWindowOffsetRect = stageWindowZRect;
        OffsetMRect(&stageWindowOffsetRect, frameToDraw * stageWindowZRect.right, 0);
        
        CC_Rect stageWindowOffsetCCRect;
        CC_Rect stageWindowCCRect;
        
        Gfx_BlitFrontSurface(stageMapSurface,
                             Gfx_MRectToCCRect(&stageWindowOffsetRect, &stageWindowOffsetCCRect),
                             Gfx_MRectToCCRect(&stageWindowRect,       &stageWindowCCRect));
    }
}

void InitGame( int player1, int player2, int startingLevel )
{
	playerWindowVisible[0] = true;
    nextWindowVisible[0] = (player1 == kAutoControl)? false: true;
	scoreWindowVisible[0] = true;
	grayMonitorVisible[0] = true;
    
	if( player2 == kNobodyControl )
	{
        g_iPadScreenLeftEdge = 128; /* adjusted slightly left to show character better */
#if __APPLE__
        g_iPadPauseButtonCenter = { 800, 106 }; /* to the right of the playfield */
#endif
        Platform_SetPauseButtonPosition({ 149, 1500 });
        Platform_SetPauseButtonVisible(true);
        Platform_NotifyScreenLeftEdge(g_iPadScreenLeftEdge);

        playerWindowVisible[1] = false;
		nextWindowVisible[1] = false;
		scoreWindowVisible[1] = false;
		grayMonitorVisible[1] = false;
        stageWindowVisible = false;
        
        CenterRectOnScreen( &playerWindowRect[0], 0.5, 0.5  );
        CenterRectOnScreen( &scoreWindowRect[0],  0.5, kScoreWindowVertCenter );
        CenterRectOnScreen( &grayMonitorRect[0],  0.5, kGrayMonitorVertCenter );
        CenterRectOnScreen( &nextWindowRect[0],   0.335, kNextWindowVertCenter1P );
        
        CenterRectOnScreen( &opponentWindowRect,  0.126, 1.0 );
	}
	else
	{
        g_iPadScreenLeftEdge = 204; /* dead center */
#if __APPLE__
        g_iPadPauseButtonCenter = { 512, 106/*60*/ }; /* dead center */
#endif
        Platform_SetPauseButtonPosition({ 149, 960 });
        Platform_SetPauseButtonVisible(true);
        Platform_NotifyScreenLeftEdge(g_iPadScreenLeftEdge);

        playerWindowVisible[1] = true;
		nextWindowVisible[1] = true;
		scoreWindowVisible[1] = true;
		grayMonitorVisible[1] = true;
        stageWindowVisible = true;

        CenterRectOnScreen( &playerWindowRect[0], kLeftPlayerWindowCenter, 0.5  );
        CenterRectOnScreen( &scoreWindowRect[0],  kLeftPlayerWindowCenter, kScoreWindowVertCenter );
        CenterRectOnScreen( &grayMonitorRect[0],  kLeftPlayerWindowCenter, kGrayMonitorVertCenter );
        CenterRectOnScreen( &nextWindowRect[0],   0.38, kNextWindowVertCenter2P );
        
        CenterRectOnScreen( &playerWindowRect[1], kRightPlayerWindowCenter, 0.5  );
        CenterRectOnScreen( &scoreWindowRect[1],  kRightPlayerWindowCenter, kScoreWindowVertCenter );
        CenterRectOnScreen( &grayMonitorRect[1],  kRightPlayerWindowCenter, kGrayMonitorVertCenter );
        CenterRectOnScreen( &nextWindowRect[1],   0.62, kNextWindowVertCenter2P );
        
        CenterRectOnScreen( &stageWindowRect,     0.5, kNextWindowVertCenter2P );
        CenterRectOnScreen( &opponentWindowRect,  0.5, 1.0 );
	}

    // For the tutorial, we shove the screen to the left edge so we can see what Miss Gumdrop has to say.
    if (startingLevel == kTutorialLevel)
    {
        g_iPadScreenLeftEdge = 0;
#if __APPLE__
        g_iPadPauseButtonCenter = { 892, 106 }; /* to the right of the chalkboard */
#endif
        Platform_SetPauseButtonPosition({ 149, 1673 });
        Platform_SetPauseButtonVisible(true);
        Platform_NotifyScreenLeftEdge(g_iPadScreenLeftEdge);
    }
    
	players = (player1 == kPlayerControl) + (player2 == kPlayerControl);
	
	if( players < 2 )
	{
		difficulty[0] = difficulty[1] = kHardLevel;
	}
	
	control[0] = player1;
	control[1] = player2;
	
	score[0] = score[1] = displayedScore[0] = displayedScore[1] = 0;	
	roundStartScore[0] = roundStartScore[1] = 0;
	
	level = startingLevel;
	credits = (player2 == kNobodyControl)? 1: 5;
}

void InitLevelData(const char* filename)
{
    s_characterList.clear();
    
    FILE* f = fopen(QuickResourceName(filename, 0, ".txt"), "r");
    if (f == nullptr)
    {
        Platform_Error(QuickResourceName(filename, 0, ".txt - file could not be opened"));
    }
        
    char line[1000];
    fgets(line, arrsize(line), f);
    
    for (;;)
    {
        if (nullptr == fgets(line, arrsize(line), f))
            break;
        
        Character  c;
        char	   unused;
        
        if (17 != sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %c",
                         &c.picture,
                         &c.bobAmount,
                         &c.bobSpeed,
                         &c.underlayerAnimSpeed,
                         &c.dropSpeed,
                         &c.intellect,
                         &c.speedNormal,
                         &c.speedRush,
                         &c.autoSetup[0],
                         &c.autoSetup[1],
                         &c.autoSetup[2],
                         &c.autoSetup[3],
                         &c.autoSetup[4],
                         &c.autoSetup[5],
                         &c.music,
                         &c.zapStyle,
                         &c.hints,
                         &unused))
        {
            break;
        }
        
        s_characterList.push_back(c);
    }
    
    if (s_characterList.empty())
    {
        Platform_Error(QuickResourceName(filename, 0, ".txt - parse failed"));
    }
    
    fclose(f);
}

Character* GetCharacterData(int level)
{
    if (level < 0 || level >= s_characterList.size())
        return nullptr;

    return &s_characterList[level];
}

MBoolean InitCharacter(int player, int level)
{
    Character* c = GetCharacterData(level);
    if (c == nullptr)
        return false;
    
    character[player] = *c;
    return true;
}

static void PrepareStageGraphics_Board(const char* picName)
{
    DrawPICTInSurface(boardSurface[0], backgroundID, picName);
    Gfx_BlitSurface( boardSurface[0], &boardSurface[0]->clip_rect, boardSurface[1], &boardSurface[1]->clip_rect);
    DrawPICTInSurface(boardSurface[1], backgroundID, picName, "-R");
    
    SurfaceBlendUnder(boardSurface[0], &playerWindowZRect, backdropSurface, &playerWindowRect[0]);
    SurfaceBlendUnder(boardSurface[1], &playerWindowZRect, backdropSurface, &playerWindowRect[1]);
}

void PrepareStageGraphics(const Character& character)
{
	int player;
	                            
	MRect blobBoard = { 0, 0, kGridDown * kBlobVertSize, kGridAcross * kBlobHorizSize };
	
    backgroundID = 5000 + character.picture;

    // In Solitaire mode, we support a custom backdrop for Level 12.
    bool drawnBackdrop = false;
    if (control[0] == kPlayerControl && control[1] == kNobodyControl)
    {
        drawnBackdrop = DrawPICTInSurface( backdropSurface, backgroundID, "/BackdropSolitaire" );
    }
    
    if (!drawnBackdrop)
    {
        drawnBackdrop = DrawPICTInSurface( backdropSurface, backgroundID, "/Backdrop" );
    }
    
    // Load the scoreboard graphics for this level.
    PrepareStageGraphics_Score();

    // Draw the boards and alpha-blend them.
    PrepareStageGraphics_Board("/Board");
    
    // Load the NEXT surfaces and alpha-blend them.
    DrawPICTInSurface(nextSurface[0], backgroundID, "/BoardNext");
    Gfx_BlitSurface(nextSurface[0], &nextSurface[0]->clip_rect, nextSurface[1], &nextSurface[1]->clip_rect);
    DrawPICTInSurface(nextSurface[1], backgroundID, "/BoardNext-R");
    
    SurfaceBlendUnder(nextSurface[0], &nextWindowZRect, backdropSurface, &nextWindowRect[0]);
    SurfaceBlendUnder(nextSurface[1], &nextWindowZRect, backdropSurface, &nextWindowRect[1]);

    // Load the gray monitor surfaces and alpha-blend them.
    DrawPICTInSurface(grayMonitorSurface[0], backgroundID, "/BoardGrays");
    Gfx_BlitSurface(grayMonitorSurface[0], &grayMonitorSurface[0]->clip_rect, grayMonitorSurface[1], &grayMonitorSurface[1]->clip_rect);
    DrawPICTInSurface(grayMonitorSurface[1], backgroundID, "/BoardGrays-R");
    
    SurfaceBlendUnder(grayMonitorSurface[0], &grayMonitorZRect, backdropSurface, &grayMonitorRect[0]);
    SurfaceBlendUnder(grayMonitorSurface[1], &grayMonitorZRect, backdropSurface, &grayMonitorRect[1]);
        
    // Load the stage map.
    if (stageMapSurface != nullptr)
    {
        Gfx_FreeSurface(stageMapSurface);
    }

    // Alpha blend the stage map with the backdrop image so we can draw it as a straight blit.
    stageMapSurface = LoadPICTAsRGBSurface(backgroundID, "/StageMap");
    if (stageMapSurface != nullptr)
    {
        int    numStageFrames = stageMapSurface->w / stageWindowZRect.right;
        MRect  stageWindowOffsetRect = stageWindowZRect;
        for (int index=0; index<numStageFrames; ++index)
        {
            CC_Rect stageWindowOffsetCCRect;
            CC_Rect stageWindowCCRect;
            
            SurfaceBlendUnder(stageMapSurface, &stageWindowOffsetRect, backdropSurface, &stageWindowRect);
            Gfx_BlitFrontSurface(stageMapSurface,
                                 Gfx_MRectToCCRect(&stageWindowOffsetRect, &stageWindowOffsetCCRect),
                                 Gfx_MRectToCCRect(&stageWindowRect,       &stageWindowCCRect));

            OffsetMRect(&stageWindowOffsetRect, stageWindowZRect.right, 0);
        }
    }
    
    for( player=0; player<=1; player++ )
	{
		Gfx_AcquireSurface( playerSurface[player] );
		SurfaceDrawBoard( player, &blobBoard );
		Gfx_ReleaseSurface( playerSurface[player] );
		
		CleanSpriteArea( player, &blobBoard );
	}
	
	BeginOpponent(character.picture, character.bobAmount, character.bobSpeed, character.underlayerAnimSpeed);

	RedrawBoardContents( 0 );
	RedrawBoardContents( 1 );
	
	RefreshAll( );

	backdropTicks = MTickCount( );
	backdropFrame = 0;
}

void BeginRound( MBoolean changeMusic )
{
	int player, count, count2;
	
	InitGrays( );
	InitPotentialCombos( );
	
	switch( players )
	{
		case 0:
		case 1:
			if( InitCharacter( 1, level ) )
			{
				score[1] = roundStartScore[1] = displayedScore[1] = 0;
				character[0] = character[1];
				character[0].zapStyle = RandomBefore(5);
			}
			else
			{
                UI_ShowGameOverScreen();
                return;
			}
			
			if (control[1] == kNobodyControl)
			{
				InitRandom(3);
			}
			else if (control[1] == kAIControl)
			{
				InitRandom(5);
			}
			break;
			
		case 2:
			score[0] = score[1] = roundStartScore[0] = roundStartScore[1] = displayedScore[0] = displayedScore[1] = 0;	

			InitRandom( 5 );

			SelectRandomLevel( );
			InitCharacter( 0, level );
			
			SelectRandomLevel( );
			InitCharacter( 1, level );
			
			character[0].hints = (difficulty[0] == kEasyLevel) || (difficulty[0] == kMediumLevel);
			character[1].hints = (difficulty[1] == kEasyLevel) || (difficulty[1] == kMediumLevel);
			break;
	}
	
	for( player=0; player<=1; player++ )
	{
		for( count=0; count<kGridAcross; count++ )
		{
			grays[player][count] = 0;
			
			for( count2=0; count2<kGridDown; count2++ )
			{
				grid[player][count][count2] = kEmpty;
				suction[player][count][count2] = kNoSuction;
				charred[player][count][count2] = kNoCharring;
				glow[player][count][count2] = false;
			}
		}
    
		nextA[player] = GetPiece( player );
		nextB[player] = GetPiece( player );
		nextM[player] = false;
		nextG[player] = false;
		
		halfway[player] = false;
		
		unallocatedGrays[player] = 0;
		anim[player] = 0;
		lockGrays[player] = 0;
		roundStartScore[player] = score[player];
		
		RedrawBoardContents(player);
		
		if( control[player] != kNobodyControl )
		{
			role[player] = kWaitForRetrieval;
		}
		else
		{
			role[player] = kIdlePlayer;
		}
	}
	
    PrepareStageGraphics(character[1]);
	if( changeMusic ) ChooseMusic( character[1].music );
	
	blobTime[0]     = blobTime[1]     = 
	boredTime[0]    = boredTime[1]    = 
	hintTime[0]     = hintTime[1]     =
	timeAI[0]       = timeAI[1]       =
	fadeCharTime[0] = fadeCharTime[1] = 
	messageTime     = startTime       =
	blinkTime[0]    = blinkTime[1]    = GameTickCount( );
	
	blinkTime[1] += 60;
	
	if( players == 2 )
		InitDifficulty( );
}

void IncrementLevel( void )
{
	level++;
}

void SelectRandomLevel( void )
{
    level = 1 + RandomBefore(std::min<>(kLevels, g_levelBeaten + 1));
}

void InitDifficulty( )
{
	MRect blobBoard = { 0, 0, kGridDown * kBlobVertSize, kGridAcross * kBlobHorizSize };
	int player;
	const int selectionRow = 5;
	
    PrepareStageGraphics_Board("/BoardSelect");

    for( player=0; player<=1; player++ )
	{
		// Set up variables
		role[player] = kChooseDifficulty;
		colorA[player] = RandomBefore(kBlobTypes)+1;
		colorB[player] = kEmpty;
		switch( difficulty[player] )
		{
			case kEasyLevel:      blobX[player] = 1; break;
			case kMediumLevel:    blobX[player] = 2; break;
			case kHardLevel:      blobX[player] = 3; break;
			case kUltraLevel:     blobX[player] = 4; break;
		}
		
		blobY[player] = selectionRow;
		blobR[player] = upRotate;
		blobTime[player] = GameTickCount( ) + (60*8);
		animTime[player] = GameTickCount( );
		shadowDepth[player] = kBlobShadowDepth;
		magic[player] = false;
		grenade[player] = false;
		
		Gfx_AcquireSurface( playerSurface[player] );

		SurfaceDrawBoard( player, &blobBoard );
		
		grid[player][0][selectionRow] = kGray;
		suction[player][0][selectionRow] = kEasyGray;
		charred[player][0][selectionRow] = kNoCharring;
		
		grid[player][kGridAcross-1][selectionRow] = kGray;	
		suction[player][kGridAcross-1][selectionRow] = kHardGray;
		charred[player][kGridAcross-1][selectionRow] = kNoCharring;

        Gfx_ReleaseSurface( playerSurface[player] );
		
		DrawSpriteBlobs( player, kNoSuction );
		CleanSpriteArea( player, &blobBoard );
	}
}

void ChooseDifficulty( int player )
{
    const int selectionRow = 5;
	const int difficultyMap[kGridAcross] = {kEasyLevel, kEasyLevel, kMediumLevel, kHardLevel, kUltraLevel, kUltraLevel};
	const int fallingSpeed[kGridAcross] = {0, 25, 18, 10, 7,  0};
	const int startGrays[kGridAcross]   = {0, 0,  0,  6,  10, 0};
	const int difficultyFrame[] = { kNoSuction, blobBlinkAnimation, blobBlinkAnimation,
                                    blobJiggleAnimation, blobCryAnimation, kNoSuction };
	int oldX = blobX[player];
	
	PlayerControl( player );
	if( blobX[player] != oldX ) anim[player] = 0;
	
	UpdateTweak( player, difficultyFrame[blobX[player]] );

	if( GameTickCount( ) >= blobTime[player] )
	{
        PrepareStageGraphics_Board("/Board");
        
        static const MRect blobBoard = { 0, 0, kGridDown * kBlobVertSize, kGridAcross * kBlobHorizSize };

        Gfx_AcquireSurface( playerSurface[player] );
		SurfaceDrawBoard( player, &blobBoard );
		Gfx_ReleaseSurface( playerSurface[player] );
		
		CleanSpriteArea( player, &blobBoard );
	
		grid[player][0][selectionRow] = kEmpty;
		grid[player][5][selectionRow] = kEmpty;
												
		suction[player][0][selectionRow] = kNoSuction;
		suction[player][5][selectionRow] = kNoSuction;
		
		difficulty[player]          = difficultyMap[ blobX[player] ];
		character[player].dropSpeed = fallingSpeed[ blobX[player] ];
		unallocatedGrays[player]    = lockGrays[player] = startGrays[blobX[player]];
		character[player].hints     = (startGrays[blobX[player]] == 0);
		role[player] = kWaitingToStart;
		
		PlayStereoFrequency( player, kPause, player );
	}
}


void RefreshAll()
{
    DrawBackdrop( );
    
    ShowGrayMonitor( 0 );
    ShowGrayMonitor( 1 );
    
    RefreshNext( 0 );
    RefreshNext( 1 );
    
    RefreshPlayerWindow( 0 );
    RefreshPlayerWindow( 1 );
    
    DrawOpponent( );
    DrawStage( );
    
    ShowScore( 0 );
    ShowScore( 1 );
}


void RefreshPlayerWindow( short player )
{
    MRect fullUpdate = {0, 0, kGridDown * kBlobVertSize, kGridAcross * kBlobHorizSize };
    
    if( control[player] == kNobodyControl )
    {
        MaskRect( &playerWindowRect[player] );
    }
    else
    {
        SetUpdateRect( player, &fullUpdate );
        UpdatePlayerWindow( player );
    }
}

