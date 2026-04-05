// main.h

#pragma once

#include "MTypes.h"
#include "Globals.h"

void Initialize( void );
void RetrieveResources( void );
void CenterRectOnScreen( MRect *rect, double locationX, double locationY );
int Warp( void );
void GoToBackground();
void InitRegistration();
void NeedRefresh();


extern char candyCrisisResources[1024];
