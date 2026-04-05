// hiscore.h

#pragma once
#include "Globals.h"

const int kNameLength = 40;

struct Combo
{
	signed char grid[kGridAcross][kGridDown];
	signed char a, b, m, g, lv, x, r, player;
	int value;
	char name[kNameLength];
};

struct HighScore
{
	char name[kNameLength];
	int score;
};

void ShowBestCombo();
void SubmitCombo( Combo *in );
void InitPotentialCombos();

extern char g_recentHighScoreNames[3][kNameLength];
extern Combo best, defaultBest, evenBetter, potentialCombo[2];
extern HighScore scores[10], defaultScores[10];
