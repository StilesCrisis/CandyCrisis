// level.h

#pragma once
#include "MTypes.h"

#define kLevels 12


struct Character
{
    int picture;
    int bobAmount;
    int bobSpeed;
    int underlayerAnimSpeed;
    int dropSpeed;
    int intellect;
    int speedNormal;
    int speedRush;
    int autoSetup[6];
    int music;
    int zapStyle;
    int hints;
};


void InitGame( int player1, int player2, int startingLevel );
void InitLevelData(const char* filename);
Character* GetCharacterData(int level);
MBoolean InitCharacter( int player, int level );
void PrepareStageGraphics(const Character& character);
void BeginRound( MBoolean changeMusic );
void InitDifficulty( void );
void ChooseDifficulty( int player );
void SelectRandomLevel( void );
void IncrementLevel( void );
void InitStage( void );
void UpdateStage();
void DrawStage();
void RefreshAll();
void RefreshPlayerWindow( short player );


enum
{
	kPlayerControl = 0,
	kAIControl,
	kNobodyControl,
	kAutoControl
};

extern Character character[2];
extern int level, players, credits, difficulty[2];
extern int difficultyTicks, backdropTicks, backdropFrame;
extern MBoolean playerWindowVisible[2];


