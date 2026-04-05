// music.h

#pragma once

void EnableMusic( bool on );
void PauseMusic();
void ResumeMusic();
void FastMusic();
void SlowMusic();
void ChooseMusic( short which );


extern bool     musicOn;
extern int      musicSelection;

