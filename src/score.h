// score.h

#pragma once
#include "MTypes.h"

struct CC_RGBSurface;

void InitScore();
void UpdateScore( int player );
void ShowScore( int player );
void PrepareStageGraphics_Score();

extern CC_RGBSurface*   scoreSurface[2];
extern MRect            scoreWindowZRect, scoreWindowRect[2];
extern MBoolean         scoreWindowVisible[2];
extern int              roundStartScore[2], score[2], displayedScore[2];
