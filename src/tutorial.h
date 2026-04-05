// tutorial.h

#pragma once

const int kTutorialLevel = 14;

enum AutoCommand
{
	e_AutoCommand_Message = 0,
	e_AutoCommand_Wait,
	e_AutoCommand_Retrieve,
	e_AutoCommand_Position,
	e_AutoCommand_Spin,
	e_AutoCommand_BlockUntilLand,
	e_AutoCommand_kBlockUntilDrop,
	e_AutoCommand_Punish,
	e_AutoCommand_ShowStartMenu,
    e_AutoCommand_BlockUntilComplete,
    e_AutoCommand_UpdateBackdrop,
};

typedef struct
{
	AutoCommand command;
	int d1, d2;
	const char *message;
}
AutoPattern, *AutoPatternPtr;

// Legacy names used in Platform_SDL.cpp tutorial data
constexpr AutoCommand kMessage        = e_AutoCommand_Message;
constexpr AutoCommand kIdleTicks      = e_AutoCommand_Wait;
constexpr AutoCommand kRetrieve       = e_AutoCommand_Retrieve;
constexpr AutoCommand kPosition       = e_AutoCommand_Position;
constexpr AutoCommand kSpin           = e_AutoCommand_Spin;
constexpr AutoCommand kBlockUntilLand = e_AutoCommand_BlockUntilLand;
constexpr AutoCommand kBlockUntilDrop = e_AutoCommand_kBlockUntilDrop;
constexpr AutoCommand kPunish         = e_AutoCommand_Punish;
constexpr AutoCommand kComplete       = e_AutoCommand_BlockUntilComplete;

extern AutoPatternPtr autoPattern;
extern int tutorialTime;

void InitTutorial( void );
void StartBalloon( const char *message );
void StopBalloon( void );
void UpdateBalloon( void );
void EndTutorial( void );
