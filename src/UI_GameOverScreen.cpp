///
///  UI_GameOverScreen.cpp
///

#include "stdafx.h"
#include "UI_GameOverScreen.h"
#include "main.h"
#include "gworld.h"
#include "SDLU.h"
#include "gameticks.h"

void UI_ShowGameOverScreen()
{
    unsigned int  timer = MTickCount() + (60*3);
    
    Platform_FadeOut();
    
    DrawPICTInSurface( g_frontSurface, picGameOver );
    
    Platform_FadeIn( );
    do
    {
        if( MTickCount() > timer ) break;
        SDLU_Yield();
    }
    while( !SDLU_AnyKeyIsPressed( ) && !SDLU_Button() );
    Platform_FadeOut( );
    
    showStartMenu = true;
}
