// opponent.h

#pragma once
#include "level.h"

void InitOpponent( void );
void BeginOpponent(int which, int bobAmount, int bobSpeed, int underlayerAnimSpeed);
void UpdateOpponent( void );
void DrawOpponent();
void OpponentChatter( MBoolean on );
void OpponentPissed( void );

const int kGlowArraySize   = 30;
const int k_opponentWidth  = 533;
const int k_opponentHeight = 678;

extern int glowArray[kGlowArraySize];

extern MRect opponentWindowRect;
