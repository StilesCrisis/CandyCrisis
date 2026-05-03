// main.h

#pragma once

#include "MTypes.h"
#include "Globals.h"

void Initialize( void );
void RetrieveResources( void );
void CenterRectOnScreen( MRect *rect, double locationX, double locationY );
void NeedRefresh();
void GameLoop();


extern char candyCrisisResources[1024];
