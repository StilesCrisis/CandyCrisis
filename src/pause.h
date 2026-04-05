// pause.h

#pragma once

enum UIDialogType
{
    kPauseDialog = 0,
    kHiScoreDialog,
    kContinueDialog,
    kControlsDialog,
    kNumDialogs
};

void UI_ShowDialog(UIDialogType type);

enum TransitionAfterUIScreen
{
    e_AfterUIScreen_GameOver,
    e_AfterUIScreen_Game,
    e_AfterUIScreen_Title,
    e_AfterUIScreen_WorldSelect,
    e_AfterUIScreen_Victory_World1,
    e_AfterUIScreen_Victory_World2,
    e_AfterUIScreen_Victory_World3,
    e_AfterUIScreen_AddHighScore,
    e_AfterUIScreen_Start2PGame,
    e_AfterUIScreen_StartSolitaireGame,
    e_AfterUIScreen_StartWorld2,
    e_AfterUIScreen_StartWorld3,

    e_AfterPuppetShow_StartGame,
    e_AfterPuppetShow_Continue1PGame,
    e_AfterPuppetShow_ContinueSolitaireGame,
    e_AfterPuppetShow_UnlockSolitaire,
    e_AfterPuppetShow_UnlockOnePlayer,
    e_AfterPuppetShow_UnlockTwoPlayer,
};

void UI_AddHighScoreScreen(int score, TransitionAfterUIScreen transition);
void UI_DoTransitionAfterUIScreen(TransitionAfterUIScreen transition);
void UI_ShowWorld1VictoryScreen(TransitionAfterUIScreen transition);
void UI_ShowWorld2VictoryScreen(TransitionAfterUIScreen transition);
void UI_ShowWorld3VictoryScreen(TransitionAfterUIScreen transition);
void UI_ShowCreditsScreen(TransitionAfterUIScreen transition);
void UI_ShowPostCreditsScreen(TransitionAfterUIScreen transition);
