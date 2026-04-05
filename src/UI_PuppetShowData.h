//
//  UI_PuppetShowData.h
//  Candy Crisis HD / Candy Crisis TV (shared)
//
//  Puppet show choreography data shared between the SDL/Windows and iOS builds.
//  Include this header from exactly one translation unit per binary:
//    - Windows: UI_PuppetShow.cpp
//    - iOS:     PuppetShowViewController.mm
//

#pragma once

#include "opponent.h"  // k_opponentWidth, k_opponentHeight
#include "Globals.h"   // arrsize
#include "level.h"     // control[], kPlayerControl, kAIControl, kNobodyControl

// ---------------------------------------------------------------------------
// Cross-platform types
// ---------------------------------------------------------------------------

struct PuppetShowPoint { float x, y; };
#define PUPPET_STR(x) x
typedef const char*   PuppetText;

// ---------------------------------------------------------------------------
// Command enum
// ---------------------------------------------------------------------------

#define DECLARE_PUPPET_CMDS(N)  \
    e_Command_SetPuppet##N,     \
    e_Command_MovePuppet##N,    \
    e_Command_FlipPuppet##N,    \
    e_Command_UnflipPuppet##N,  \
    e_Command_FadePuppet##N,    \
    e_Command_FirePuppet##N,    \
    e_Command_FlingPuppet##N,   \
    e_Command_FloatPuppet##N,   \
    e_Command_SpinPuppet##N,

enum PuppetShowCommand
{
    e_Command_SetOpponentName,
    e_Command_RaiseCurtain,
    e_Command_ShowBalloonLeftwards,
    e_Command_ShowBalloonRightwards,
    e_Command_HideBalloon,
    e_Command_Wait,
    e_Command_Talk,
    e_Command_Talk1POnly,
    DECLARE_PUPPET_CMDS(1)
    DECLARE_PUPPET_CMDS(2)
    DECLARE_PUPPET_CMDS(3)
    DECLARE_PUPPET_CMDS(4)
    DECLARE_PUPPET_CMDS(5)
    DECLARE_PUPPET_CMDS(6)
    DECLARE_PUPPET_CMDS(7)
    DECLARE_PUPPET_CMDS(8)
};

#undef DECLARE_PUPPET_CMDS

// ---------------------------------------------------------------------------
// Data struct
// ---------------------------------------------------------------------------

struct PuppetShowPattern
{
    PuppetShowCommand  cmd;
    PuppetShowPoint    loc;
    PuppetText         text;
};

struct PuppetShowLevel
{
    const PuppetShowPattern* patterns;
    int                      count;
};

// ---------------------------------------------------------------------------
// Position constants  (all in 1920×1080 space)
// x = center-X of the puppet; y = vertical offset from bottom (0=hidden, -678=visible)
// ---------------------------------------------------------------------------

static const float k_puppetXStageLeft      = (1920 * -0.2f);
static const float k_puppetXFarLeft        = (1920 * 0.1f);
static const float k_puppetXSolitaire      = (1920 * 0.2299f);
static const float k_puppetXMoreLeft       = (1920 * 0.25f);
static const float k_puppetXABitMoreLeft   = (1920 * 0.3f);
static const float k_puppetXLeft           = (1920 * 0.35f);
static const float k_puppetXTinyBitLeft    = (1920 * 0.4f);
static const float k_puppetXCenter         = (1920 * 0.5f);
static const float k_puppetXTinyBitRight   = (1920 * 0.52f);
static const float k_puppetXRight          = (1920 * 0.65f);
static const float k_puppetXABitMoreRight  = (1920 * 0.7f);
static const float k_puppetXMoreRight      = (1920 * 0.75f);
static const float k_puppetXFarRight       = (1920 * 0.9f);
static const float k_puppetXStageRight     = (1920 * 1.2f);

static const float k_puppetYHidden         = 0.0f;
static const float k_puppetYPeek           = (-k_opponentHeight * 1.0f / 2.0f);
static const float k_puppetYHalfVisible    = (-k_opponentHeight * 2.0f / 3.0f);
static const float k_puppetYMoreVisible    = (-k_opponentHeight * 4.0f / 5.0f);
static const float k_puppetYVisible        = (-(float)k_opponentHeight);

static const PuppetShowPoint k_puppetFarLeftHidden        = { k_puppetXFarLeft,        k_puppetYHidden };
static const PuppetShowPoint k_puppetMoreLeftHidden       = { k_puppetXMoreLeft,       k_puppetYHidden };
static const PuppetShowPoint k_puppetTinyBitLeftHidden    = { k_puppetXTinyBitLeft,    k_puppetYHidden };
static const PuppetShowPoint k_puppetLeftHidden           = { k_puppetXLeft,           k_puppetYHidden };
static const PuppetShowPoint k_puppetCenterHidden         = { k_puppetXCenter,         k_puppetYHidden };
static const PuppetShowPoint k_puppetRightHidden          = { k_puppetXRight,          k_puppetYHidden };
static const PuppetShowPoint k_puppetMoreRightHidden      = { k_puppetXMoreRight,      k_puppetYHidden };
static const PuppetShowPoint k_puppetFarRightHidden       = { k_puppetXFarRight,       k_puppetYHidden };

static const PuppetShowPoint k_puppetExitStageLeft        = { k_puppetXStageLeft,      k_puppetYVisible };
static const PuppetShowPoint k_puppetFarLeftVisible       = { k_puppetXFarLeft,        k_puppetYVisible };
static const PuppetShowPoint k_puppetMoreLeftVisible      = { k_puppetXMoreLeft,       k_puppetYVisible };
static const PuppetShowPoint k_puppetABitMoreLeftVisible  = { k_puppetXABitMoreLeft,   k_puppetYVisible };
static const PuppetShowPoint k_puppetLeftVisible          = { k_puppetXLeft,           k_puppetYVisible };
static const PuppetShowPoint k_puppetTinyBitLeftVisible   = { k_puppetXTinyBitLeft,    k_puppetYVisible };
static const PuppetShowPoint k_puppetCenterVisible        = { k_puppetXCenter,         k_puppetYVisible };
static const PuppetShowPoint k_puppetTinyBitRightVisible  = { k_puppetXTinyBitRight,   k_puppetYVisible };
static const PuppetShowPoint k_puppetRightVisible         = { k_puppetXRight,          k_puppetYVisible };
static const PuppetShowPoint k_puppetABitMoreRightVisible = { k_puppetXABitMoreRight,  k_puppetYVisible };
static const PuppetShowPoint k_puppetMoreRightVisible     = { k_puppetXMoreRight,      k_puppetYVisible };
static const PuppetShowPoint k_puppetFarRightVisible      = { k_puppetXFarRight,       k_puppetYVisible };
static const PuppetShowPoint k_puppetExitStageRight       = { k_puppetXStageRight,     k_puppetYVisible };

static const PuppetShowPoint k_puppetCenterFireball       = { k_puppetXCenter - 60,    -k_opponentHeight * 75.0f / 100.0f };
static const PuppetShowPoint k_puppetSolitaireFireball    = { k_puppetXSolitaire - 60, -k_opponentHeight * 75.0f / 100.0f };

// ---------------------------------------------------------------------------
// Choreography data — one entry per 1-based level (12 total)
// ---------------------------------------------------------------------------

static const PuppetShowPattern s_puppetLevelData_0[] =   // Level 1: Mister Pugglesworth
{
    { e_Command_SetOpponentName,        {},                                   PUPPET_STR("Mister Pugglesworth") },
    { e_Command_RaiseCurtain },
    { e_Command_Wait,                   { 0.3f } },
    { e_Command_SetPuppet1,             k_puppetCenterHidden,                 PUPPET_STR("PICT_5000/N-1A.png") },
    { e_Command_MovePuppet1,            k_puppetCenterVisible },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_ShowBalloonLeftwards,   k_puppetABitMoreRightVisible },
    { e_Command_Wait,                   { 0.3f } },
    { e_Command_Talk,                   {},                                   PUPPET_STR("Hi there!\nI'm Mister Pugglesworth.~~~~~\nUp for a game?") },
    { e_Command_Talk1POnly,             {},                                   PUPPET_STR("The one who lasts\nthe longest wins!\n") },
    { e_Command_SetPuppet1,             k_puppetCenterVisible,                PUPPET_STR("PICT_5000/H-1A.png") },
    { e_Command_Talk,                   {},                                   PUPPET_STR("I'm colorblind, so\nsometimes I make\nsilly mistakes!") },
    { e_Command_SetPuppet1,             k_puppetCenterVisible,                PUPPET_STR("PICT_5000/N-1A.png") },
    { e_Command_HideBalloon },
    { e_Command_Wait,                   { 0.2f } },
};

static const PuppetShowPattern s_puppetLevelData_1[] =   // Level 2: Darby
{
    { e_Command_SetPuppet1,             k_puppetMoreRightHidden,              PUPPET_STR("PICT_5000/S-1A.png") },
    { e_Command_SetPuppet2,             k_puppetCenterHidden,                 PUPPET_STR("PICT_5001/H-1A.png") },
    { e_Command_SetOpponentName,        {},                                   PUPPET_STR("Darby") },
    { e_Command_RaiseCurtain },
    { e_Command_Wait,                   { 0.3f } },
    { e_Command_MovePuppet1,            { k_puppetXMoreRight, k_puppetYHalfVisible } },
    { e_Command_MovePuppet2,            k_puppetCenterVisible },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_ShowBalloonLeftwards,   k_puppetABitMoreRightVisible },
    { e_Command_Wait,                   { 0.3f } },
    { e_Command_Talk,                   {},                                   PUPPET_STR("Hey mister!~~~~~ Thanks\nfor finding\nMister Pugglesworth!") },
    { e_Command_HideBalloon },
    { e_Command_FlipPuppet1 },
    { e_Command_MovePuppet1,            { k_puppetXStageRight, k_puppetYHalfVisible } },
    { e_Command_ShowBalloonLeftwards,   k_puppetABitMoreRightVisible },
    { e_Command_Wait,                   { 0.3f } },
    { e_Command_SetPuppet2,             k_puppetCenterVisible,                PUPPET_STR("PICT_5001/H-6A.png") },
    { e_Command_Talk,                   {},                                   PUPPET_STR("How about playing a\ngame with me?\nI've been practicing!") },
    { e_Command_SetPuppet2,             k_puppetCenterVisible,                PUPPET_STR("PICT_5001/N-1A.png") },
    { e_Command_HideBalloon },
    { e_Command_Wait,                   { 0.1f } },
    { e_Command_FadePuppet1,            { 0.0f, 0.5f } },  // Pugglesworth still sliding off; fade him out
};

static const PuppetShowPattern s_puppetLevelData_2[] =   // Level 3: Porkchop
{
    { e_Command_SetPuppet1,             k_puppetCenterHidden,                 PUPPET_STR("PICT_5002/S-6C.png") },
    { e_Command_SetOpponentName,        {},                                   PUPPET_STR("Porkchop") },
    { e_Command_RaiseCurtain },
    { e_Command_Wait,                   { 0.3f } },
    { e_Command_MovePuppet1,            k_puppetCenterVisible },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_ShowBalloonLeftwards,   k_puppetABitMoreRightVisible },
    { e_Command_Talk,                   {},                                   PUPPET_STR("Raaarrrr!\nI'll bite your arms off!\n") },
    { e_Command_SetPuppet1,             k_puppetCenterVisible,                PUPPET_STR("PICT_5002/N-2A.png") },
    { e_Command_HideBalloon },
    { e_Command_Wait,                   { 0.1f } },
};

static const PuppetShowPattern s_puppetLevelData_3[] =   // Level 4: Mei
{
    { e_Command_SetPuppet1,             k_puppetCenterHidden,                 PUPPET_STR("PICT_5003/N-1A.png") },
    { e_Command_SetPuppet2,             { k_puppetXTinyBitLeft, k_puppetYHidden },  PUPPET_STR("PICT_5003/TeaPlatter.png") },
    { e_Command_SetPuppet3,             { k_puppetXTinyBitLeft, k_puppetYHidden },  PUPPET_STR("PICT_5003/TeaPot.png") },
    { e_Command_SetPuppet4,             { k_puppetXTinyBitLeft, k_puppetYHidden },  PUPPET_STR("PICT_5003/TeaCup1.png") },
    { e_Command_SetPuppet5,             { k_puppetXTinyBitLeft, k_puppetYHidden },  PUPPET_STR("PICT_5003/TeaCup2.png") },
    { e_Command_SetPuppet6,             { k_puppetXTinyBitLeft, k_puppetYHidden },  PUPPET_STR("PICT_5003/TeaCup3.png") },
    { e_Command_SetPuppet7,             { k_puppetXTinyBitLeft, k_puppetYHidden },  PUPPET_STR("PICT_5003/TeaLemon.png") },
    { e_Command_SetOpponentName,        {},                                   PUPPET_STR("Mei") },
    { e_Command_RaiseCurtain },
    { e_Command_Wait,                   { 0.3f } },
    { e_Command_MovePuppet1,            k_puppetCenterVisible },
    { e_Command_MovePuppet2,            { k_puppetXTinyBitLeft, k_puppetYHalfVisible } },
    { e_Command_MovePuppet3,            { k_puppetXTinyBitLeft, k_puppetYHalfVisible } },
    { e_Command_MovePuppet4,            { k_puppetXTinyBitLeft, k_puppetYHalfVisible } },
    { e_Command_MovePuppet5,            { k_puppetXTinyBitLeft, k_puppetYHalfVisible } },
    { e_Command_MovePuppet6,            { k_puppetXTinyBitLeft, k_puppetYHalfVisible } },
    { e_Command_MovePuppet7,            { k_puppetXTinyBitLeft, k_puppetYHalfVisible } },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_SetPuppet1,             k_puppetCenterVisible,                PUPPET_STR("PICT_5003/H-4A.png") },
    { e_Command_ShowBalloonLeftwards,   k_puppetABitMoreRightVisible },
    { e_Command_Talk,                   {},                                   PUPPET_STR("Would you like some\n~~~~~tea~~~~~ with your candies?\n") },
    { e_Command_SetPuppet1,             k_puppetCenterVisible,                PUPPET_STR("PICT_5003/H-1A.png") },
    { e_Command_HideBalloon },
    { e_Command_MovePuppet2,            { k_puppetXTinyBitLeft + 20, k_puppetYHalfVisible + 20 } },
    { e_Command_MovePuppet3,            { k_puppetXTinyBitLeft + 20, k_puppetYHalfVisible + 20 } },
    { e_Command_MovePuppet4,            { k_puppetXTinyBitLeft + 20, k_puppetYHalfVisible + 20 } },
    { e_Command_MovePuppet5,            { k_puppetXTinyBitLeft + 20, k_puppetYHalfVisible + 20 } },
    { e_Command_MovePuppet6,            { k_puppetXTinyBitLeft + 20, k_puppetYHalfVisible + 20 } },
    { e_Command_MovePuppet7,            { k_puppetXTinyBitLeft + 20, k_puppetYHalfVisible + 20 } },
    { e_Command_Wait,                   { 0.2f } },
    { e_Command_FlingPuppet2,           { -8, -5 } },
    { e_Command_FlingPuppet3,           { -22, -11 } },
    { e_Command_FlingPuppet4,           { -24, -7 } },
    { e_Command_FlingPuppet5,           { -32, -4 } },
    { e_Command_FlingPuppet6,           { -36, -8 } },
    { e_Command_FlingPuppet7,           { -12, -14 } },
    { e_Command_SpinPuppet2,            { 0.5f } },
    { e_Command_SpinPuppet3,            { 3.47f } },
    { e_Command_SpinPuppet4,            { -2.35f } },
    { e_Command_SpinPuppet5,            { -1.25f } },
    { e_Command_SpinPuppet6,            { 2.69f } },
    { e_Command_SpinPuppet7,            { -3.31f } },
    { e_Command_Wait,                   { 2.0f } },
};

static const PuppetShowPattern s_puppetLevelData_4[] =   // Level 5: Calamari Squad
{
    { e_Command_SetPuppet1,             k_puppetCenterHidden,                 PUPPET_STR("PICT_5004/X-UA.png") },
    { e_Command_SetPuppet2,             k_puppetCenterHidden,                 PUPPET_STR("PICT_5004/S-1A.png") },
    { e_Command_SetPuppet3,             k_puppetExitStageLeft,                PUPPET_STR("PICT_5004/SquidPuppetsLittle.png") },
    { e_Command_SetPuppet4,             k_puppetExitStageRight,               PUPPET_STR("PICT_5004/SquidPuppetsLittle.png") },
    { e_Command_FlipPuppet4 },
    { e_Command_SetOpponentName,        {},                                   PUPPET_STR("Calamari Squad") },
    { e_Command_RaiseCurtain },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_MovePuppet3,            { 0, k_puppetYVisible } },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_MovePuppet4,            { 1920, k_puppetYVisible } },
    { e_Command_Wait,                   { 1.0f } },
    { e_Command_MovePuppet3,            { 80, k_puppetYVisible } },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_MovePuppet4,            { (1920 - 80), k_puppetYVisible } },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_MovePuppet1,            { k_puppetXCenter, k_puppetYHidden - 80 } },
    { e_Command_MovePuppet2,            { k_puppetXCenter, k_puppetYHidden - 80 } },
    { e_Command_Wait,                   { 1.0f } },
    { e_Command_MovePuppet1,            { k_puppetXCenter, k_puppetYHidden - 150 } },
    { e_Command_MovePuppet2,            { k_puppetXCenter, k_puppetYHidden - 150 } },
    { e_Command_MovePuppet3,            { 120, k_puppetYVisible } },
    { e_Command_MovePuppet4,            { (1920 - 120), k_puppetYVisible } },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_MovePuppet1,            { k_puppetXCenter, k_puppetYHidden - 250 } },
    { e_Command_MovePuppet2,            { k_puppetXCenter, k_puppetYHidden - 250 } },
    { e_Command_MovePuppet3,            { 160, k_puppetYVisible } },
    { e_Command_MovePuppet4,            { (1920 - 160), k_puppetYVisible } },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_MovePuppet1,            k_puppetCenterVisible },
    { e_Command_MovePuppet2,            k_puppetCenterVisible },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_SetPuppet2,             k_puppetCenterVisible,                PUPPET_STR("PICT_5004/N-1A.png") },
    { e_Command_FadePuppet3,            { 0.0f, 0.5f } },
    { e_Command_FadePuppet4,            { 0.0f, 0.5f } },
    { e_Command_Wait,                   { 0.2f } },
};

static const PuppetShowPattern s_puppetLevelData_5[] =   // Level 6: Tan-chan
{
    { e_Command_SetPuppet1,             k_puppetCenterHidden,                 PUPPET_STR("PICT_5005/N-1A.png") },
    { e_Command_SetPuppet2,             k_puppetMoreRightHidden,              PUPPET_STR("PICT_5006/N-1A.png") },
    { e_Command_FlipPuppet2 },
    { e_Command_SetOpponentName,        {},                                   PUPPET_STR("Tan-chan") },
    { e_Command_RaiseCurtain },
    { e_Command_Wait,                   { 0.3f } },
    { e_Command_MovePuppet1,            k_puppetCenterVisible },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_ShowBalloonLeftwards,   k_puppetABitMoreRightVisible },
    { e_Command_Talk,                   {},                                   PUPPET_STR("This is our forest\nnow.~~~~~~~~~~~ Go back home!\n") },
    { e_Command_SetPuppet1,             k_puppetCenterVisible,                PUPPET_STR("PICT_5005/H-6A.png") },
    { e_Command_MovePuppet2,            { k_puppetXMoreRight, k_puppetYPeek } },
    { e_Command_Talk,                   {},                                   PUPPET_STR("No?~~~~~~~~~ Maybe some candy\nwill change your mind!\n") },
    { e_Command_SetPuppet1,             k_puppetCenterVisible,                PUPPET_STR("PICT_5005/N-1A.png") },
    { e_Command_HideBalloon },
    { e_Command_Wait,                   { 0.2f } },
    { e_Command_MovePuppet2,            k_puppetMoreRightHidden },
};

static const PuppetShowPattern s_puppetLevelData_6[] =   // Level 7: Fluffy
{
    { e_Command_SetPuppet1,             k_puppetMoreRightHidden,              PUPPET_STR("PICT_5005/N-1A.png") },
    { e_Command_SetPuppet2,             k_puppetCenterHidden,                 PUPPET_STR("PICT_5006/N-1A.png") },
    { e_Command_SetOpponentName,        {},                                   PUPPET_STR("Fluffy") },
    { e_Command_RaiseCurtain },
    { e_Command_Wait,                   { 0.3f } },
    { e_Command_MovePuppet1,            k_puppetMoreRightVisible },
    { e_Command_MovePuppet2,            { k_puppetXCenter, k_puppetYPeek } },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_ShowBalloonRightwards,  k_puppetCenterVisible },
    { e_Command_Talk,                   {},                                   PUPPET_STR("If you thought you\nhad me beaten,\nthink again!") },
    { e_Command_HideBalloon },
    { e_Command_SetPuppet1,             k_puppetMoreRightVisible,             PUPPET_STR("PICT_5005/H-1A.png") },
    { e_Command_MovePuppet1,            { k_puppetXMoreRight + 80, k_puppetYVisible } },
    { e_Command_MovePuppet2,            k_puppetCenterVisible },
    { e_Command_ShowBalloonRightwards,  { k_puppetXCenter + 80, k_puppetYVisible } },
    { e_Command_Talk,                   {},                                   PUPPET_STR("No one gets past\nmy pet tiger!\n") },
    { e_Command_HideBalloon },
    { e_Command_FadePuppet1,            { 0.0f, 0.5f } },
};

static const PuppetShowPattern s_puppetLevelData_7[] =   // Level 8: Tan-chan's Big Sister
{
    { e_Command_SetPuppet1,             k_puppetMoreLeftHidden,               PUPPET_STR("PICT_5005/S-1A.png") },
    { e_Command_SetPuppet2,             k_puppetCenterHidden,                 PUPPET_STR("PICT_5006/S-3A.png") },
    { e_Command_SetPuppet3,             k_puppetMoreRightHidden,              PUPPET_STR("PICT_5007/N-2A.png") },
    { e_Command_FlipPuppet1 },
    { e_Command_SetOpponentName,        {},                                   PUPPET_STR("Tan-chan's Big Sister") },
    { e_Command_RaiseCurtain },
    { e_Command_Wait,                   { 0.3f } },
    { e_Command_MovePuppet1,            k_puppetMoreLeftVisible },
    { e_Command_MovePuppet2,            { k_puppetXCenter, k_puppetYHalfVisible } },
    { e_Command_MovePuppet3,            k_puppetMoreRightVisible },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_ShowBalloonRightwards,  k_puppetCenterVisible },
    { e_Command_Talk,                   {},                                   PUPPET_STR("Sis, that good-for-\nnothing tiger is a\ntotal softie!") },
    { e_Command_HideBalloon },
    { e_Command_MovePuppet2,            { k_puppetXCenter, k_puppetYHalfVisible + 80 } },
    { e_Command_SetPuppet3,             k_puppetMoreRightVisible,             PUPPET_STR("PICT_5007/N-3A.png") },
    { e_Command_ShowBalloonLeftwards,   k_puppetCenterVisible },
    { e_Command_Talk,                   {},                                   PUPPET_STR("I told you, I don't\nneed your help! Fluffy\nis a good kitty!") },
    { e_Command_HideBalloon },
    { e_Command_MovePuppet2,            { k_puppetXCenter, k_puppetYHalfVisible + 160 } },
    { e_Command_ShowBalloonRightwards,  k_puppetCenterVisible },
    { e_Command_Talk,                   {},                                   PUPPET_STR("You two are hopeless!\nI'll take care of\nthis myself.") },
    { e_Command_HideBalloon },
    { e_Command_SetPuppet3,             k_puppetMoreRightVisible,             PUPPET_STR("PICT_5007/N-1A.png") },
    { e_Command_FadePuppet1,            { 0.0f, 0.5f } },
    { e_Command_MovePuppet2,            k_puppetCenterHidden },
    { e_Command_MovePuppet3,            k_puppetCenterVisible },
    { e_Command_Wait,                   { 0.2f } },
};

static const PuppetShowPattern s_puppetLevelData_8[] =   // Level 9: Bobo the Bat
{
    { e_Command_SetPuppet1,             k_puppetCenterHidden,                 PUPPET_STR("PICT_5008/X-UA.png|PICT_5008/X-UB.png|PICT_5008/X-UC.png|PICT_5008/X-UD.png|PICT_5008/X-UE.png|PICT_5008/X-UF.png|PICT_5008/X-UG.png") },
    { e_Command_SetPuppet2,             k_puppetCenterHidden,                 PUPPET_STR("PICT_5008/N-1A.png") },
    { e_Command_SetOpponentName,        {},                                   PUPPET_STR("Bobo the Bat") },
    { e_Command_RaiseCurtain },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_MovePuppet1,            k_puppetCenterVisible },
    { e_Command_MovePuppet2,            k_puppetCenterVisible },
    { e_Command_ShowBalloonLeftwards,   k_puppetABitMoreRightVisible },
    { e_Command_Talk,                   {},                                   PUPPET_STR("Beware! This mountain\n~~~~~is haunted!\n") },
    { e_Command_HideBalloon },
    { e_Command_Wait,                   { 0.2f } },
};

static const PuppetShowPattern s_puppetLevelData_9[] =   // Level 10: Yuki
{
    { e_Command_SetPuppet1,             k_puppetCenterVisible,                PUPPET_STR("PICT_5009/X-UA.png|PICT_5009/X-UB.png|PICT_5009/X-UC.png|PICT_5009/X-UD.png|PICT_5009/X-UE.png|PICT_5009/X-UF.png|PICT_5009/X-UG.png|PICT_5009/X-UH.png") },
    { e_Command_SetPuppet2,             k_puppetCenterVisible,                PUPPET_STR("PICT_5009/N-1A.png") },
    { e_Command_FadePuppet1,            {} },
    { e_Command_FadePuppet2,            {} },
    { e_Command_SetOpponentName,        {},                                   PUPPET_STR("Yuki") },
    { e_Command_RaiseCurtain },
    { e_Command_Wait,                   { 1.5f } },
    { e_Command_FadePuppet2,            { 1.0f, 2.0f } },
    { e_Command_Wait,                   { 1.0f } },
    { e_Command_FadePuppet1,            { 1.0f, 2.0f } },
    { e_Command_Wait,                   { 2.0f } },
    { e_Command_ShowBalloonLeftwards,   k_puppetABitMoreRightVisible },
    { e_Command_Talk,                   {},                                   PUPPET_STR("The spirits~~~~~~~ are\nrestless tonight...\n") },
    { e_Command_HideBalloon },
    { e_Command_SetPuppet2,             k_puppetCenterVisible,                PUPPET_STR("PICT_5009/N-5A.png") },
    { e_Command_Wait,                   { 0.4f } },
};

static const PuppetShowPattern s_puppetLevelData_10[] =  // Level 11: Tailypo
{
    { e_Command_SetPuppet1,             k_puppetCenterHidden,                 PUPPET_STR("PICT_5010/H-1A.png") },
    { e_Command_SetOpponentName,        {},                                   PUPPET_STR("Tailypo") },
    { e_Command_RaiseCurtain },
    { e_Command_Wait,                   { 0.3f } },
    { e_Command_MovePuppet1,            k_puppetCenterVisible },
    { e_Command_Wait,                   { 0.5f } },
    { e_Command_ShowBalloonLeftwards,   k_puppetABitMoreRightVisible },
    { e_Command_Talk,                   {},                                   PUPPET_STR("Too much candy will\ngive you n~i~g~h~t~m~a~r~e~s~.~.~.~\n") },
    { e_Command_HideBalloon },
    { e_Command_SetPuppet2,             { k_puppetXCenter,       k_puppetYVisible - 1100 },     PUPPET_STR("PICT_5010/CandyRain1.png") },
    { e_Command_SetPuppet3,             { k_puppetXCenter - 100, k_puppetYVisible - 1160 },     PUPPET_STR("PICT_5010/CandyRain2.png") },
    { e_Command_SetPuppet4,             { k_puppetXCenter + 100, k_puppetYVisible - 1100 },     PUPPET_STR("PICT_5010/CandyRain3.png") },
    { e_Command_SetPuppet5,             { k_puppetXCenter,       k_puppetYVisible - 1140 },     PUPPET_STR("PICT_5010/CandyRain4.png") },
    { e_Command_SetPuppet6,             { k_puppetXCenter - 75,  k_puppetYVisible - 1100 },     PUPPET_STR("PICT_5010/CandyRain1.png") },
    { e_Command_SetPuppet7,             { k_puppetXCenter + 120, k_puppetYVisible - 1150 },     PUPPET_STR("PICT_5010/CandyRain2.png") },
    { e_Command_SetPuppet8,             { k_puppetXCenter - 90,  k_puppetYVisible - 1100 },     PUPPET_STR("PICT_5010/CandyRain3.png") },
    { e_Command_FlingPuppet2,           { 0, 5.0f } },
    { e_Command_Wait,                   { 0.2f } },
    { e_Command_FlingPuppet3,           { -1, 5.5f } },
    { e_Command_Wait,                   { 0.2f } },
    { e_Command_FlingPuppet4,           { 1, 5.5f } },
    { e_Command_Wait,                   { 0.2f } },
    { e_Command_FlingPuppet5,           { 0, 5.0f } },
    { e_Command_Wait,                   { 0.2f } },
    { e_Command_FlingPuppet6,           { 0, 5.5f } },
    { e_Command_Wait,                   { 0.2f } },
    { e_Command_FlingPuppet7,           { -1, 5.0f } },
    { e_Command_Wait,                   { 0.2f } },
    { e_Command_FlingPuppet8,           { 1, 5.7f } },
    { e_Command_Wait,                   { 0.5f } },
    // Candy rain is mid-flight; fade it out during the gameplay crossfade.
    { e_Command_FadePuppet2,            { 0.0f, 0.6f } },
    { e_Command_FadePuppet3,            { 0.0f, 0.6f } },
    { e_Command_FadePuppet4,            { 0.0f, 0.6f } },
    { e_Command_FadePuppet5,            { 0.0f, 0.6f } },
    { e_Command_FadePuppet6,            { 0.0f, 0.6f } },
    { e_Command_FadePuppet7,            { 0.0f, 0.6f } },
    { e_Command_FadePuppet8,            { 0.0f, 0.6f } },
};

static const PuppetShowPattern s_puppetLevelData_11[] =  // Level 12: Leviathan
{
    { e_Command_SetPuppet8,             k_puppetCenterVisible,                PUPPET_STR("PICT_5011/N-1A.png") },
    { e_Command_FadePuppet8,            {} },
    { e_Command_SetOpponentName,        {},                                   PUPPET_STR("Leviathan") },
    { e_Command_RaiseCurtain },
    { e_Command_Wait,                   { 0.3f } },
    { e_Command_FadePuppet8,            { 1.0f, 1.0f } },
    { e_Command_Wait,                   { 1.5f } },
    { e_Command_SetPuppet8,             k_puppetCenterVisible,                PUPPET_STR("PICT_5011/S-1A.png") },
    { e_Command_Wait,                   { 0.2f } },
    { e_Command_SetPuppet8,             k_puppetCenterVisible,                PUPPET_STR("PICT_5011/S-5A.png") },
    { e_Command_SetPuppet1,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Fireball1.png|PICT_5011/Fireball2.png|PICT_5011/Fireball3.png|PICT_5011/Fireball4.png") },
    { e_Command_FirePuppet1,            { -30, 0 } },
    { e_Command_Wait,                   { 0.1f } },
    { e_Command_SetPuppet2,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Fireball1.png|PICT_5011/Fireball2.png|PICT_5011/Fireball3.png|PICT_5011/Fireball4.png") },
    { e_Command_FirePuppet2,            { -30, -1 } },
    { e_Command_Wait,                   { 0.1f } },
    { e_Command_SetPuppet3,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Fireball1.png|PICT_5011/Fireball2.png|PICT_5011/Fireball3.png|PICT_5011/Fireball4.png") },
    { e_Command_FirePuppet3,            { -30, -2 } },
    { e_Command_Wait,                   { 0.1f } },
    { e_Command_SetPuppet4,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Fireball1.png|PICT_5011/Fireball2.png|PICT_5011/Fireball3.png|PICT_5011/Fireball4.png") },
    { e_Command_FirePuppet4,            { -30, -1 } },
    { e_Command_Wait,                   { 0.1f } },
    { e_Command_SetPuppet5,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Fireball1.png|PICT_5011/Fireball2.png|PICT_5011/Fireball3.png|PICT_5011/Fireball4.png") },
    { e_Command_FirePuppet5,            { -30, 0 } },
    { e_Command_Wait,                   { 0.1f } },
    { e_Command_SetPuppet6,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Fireball1.png|PICT_5011/Fireball2.png|PICT_5011/Fireball3.png|PICT_5011/Fireball4.png") },
    { e_Command_FirePuppet6,            { -30, 1 } },
    { e_Command_Wait,                   { 0.1f } },
    { e_Command_SetPuppet7,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Fireball1.png|PICT_5011/Fireball2.png|PICT_5011/Fireball3.png|PICT_5011/Fireball4.png") },
    { e_Command_FirePuppet7,            { -30, 2 } },
    { e_Command_Wait,                   { 0.1f } },
    { e_Command_SetPuppet1,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Fireball1.png|PICT_5011/Fireball2.png|PICT_5011/Fireball3.png|PICT_5011/Fireball4.png") },
    { e_Command_FirePuppet1,            { -30, 1 } },
    { e_Command_Wait,                   { 0.1f } },
    { e_Command_SetPuppet2,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Fireball1.png|PICT_5011/Fireball2.png|PICT_5011/Fireball3.png|PICT_5011/Fireball4.png") },
    { e_Command_FirePuppet2,            { -30, 0 } },
    { e_Command_Wait,                   { 0.3f } },
    { e_Command_SetPuppet8,             k_puppetCenterVisible,                PUPPET_STR("PICT_5011/N-1A.png") },
    { e_Command_SetPuppet4,             k_puppetCenterVisible,                PUPPET_STR("PICT_5011/N-1A.png") },
    { e_Command_FadePuppet1,            { 0.2f, 0 } },
    { e_Command_FadePuppet2,            { 0.2f, 0 } },
    { e_Command_FadePuppet3,            { 0.2f, 0 } },
    { e_Command_Wait,                   { 0.2f } },
    { e_Command_SetPuppet8,             k_puppetCenterHidden },
    { e_Command_SetPuppet4,             k_puppetCenterVisible,                PUPPET_STR("PICT_5011/S-1A.png") },
    { e_Command_Wait,                   { 0.2f } },
    { e_Command_SetPuppet4,             k_puppetCenterVisible,                PUPPET_STR("PICT_5011/S-5A.png") },
    { e_Command_SetPuppet5,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Blob1.png") },
    { e_Command_FlingPuppet5,           { -6, 0 } },
    { e_Command_SpinPuppet5,            { 2.0f, 0 } },
    { e_Command_Wait,                   { 0.05f } },
    { e_Command_SetPuppet6,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Blob2.png") },
    { e_Command_FlingPuppet6,           { -10, -0.5f } },
    { e_Command_SpinPuppet6,            { -2.0f } },
    { e_Command_Wait,                   { 0.05f } },
    { e_Command_SetPuppet7,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Blob3.png") },
    { e_Command_FlingPuppet7,           { -5, -1 } },
    { e_Command_SpinPuppet7,            { 1.0f } },
    { e_Command_SetPuppet1,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Poof1.png|PICT_5011/Poof1.png|PICT_5011/Poof2.png|PICT_5011/Poof2.png|PICT_5011/Poof3.png|PICT_5011/Poof3.png|PICT_5011/Poof2.png|PICT_5011/Poof2.png") },
    { e_Command_FloatPuppet1,           { -4, 0 } },
    { e_Command_SpinPuppet1,            { 3.0f } },
    { e_Command_FadePuppet1,            { 0.75f, 0.5f } },
    { e_Command_Wait,                   { 0.25f } },
    { e_Command_SetPuppet2,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Poof1.png|PICT_5011/Poof1.png|PICT_5011/Poof2.png|PICT_5011/Poof2.png|PICT_5011/Poof3.png|PICT_5011/Poof3.png|PICT_5011/Poof2.png|PICT_5011/Poof2.png") },
    { e_Command_FloatPuppet2,           { -4, 0 } },
    { e_Command_SpinPuppet2,            { 3.0f } },
    { e_Command_FadePuppet2,            { 0.75f, 0.5f } },
    { e_Command_Wait,                   { 0.25f } },
    { e_Command_SetPuppet3,             k_puppetCenterFireball,               PUPPET_STR("PICT_5011/Poof1.png|PICT_5011/Poof1.png|PICT_5011/Poof2.png|PICT_5011/Poof2.png|PICT_5011/Poof3.png|PICT_5011/Poof3.png|PICT_5011/Poof2.png|PICT_5011/Poof2.png") },
    { e_Command_FloatPuppet3,           { -4, 0 } },
    { e_Command_SpinPuppet3,            { 3.0f } },
    { e_Command_FadePuppet1,            { 0.0f, 0.5f } },
    { e_Command_FadePuppet3,            { 0.75f, 0.5f } },
    { e_Command_Wait,                   { 0.25f } },
    { e_Command_FadePuppet2,            { 0.0f, 0.5f } },
    { e_Command_Wait,                   { 0.25f } },
    { e_Command_FadePuppet3,            { 0.0f, 0.5f } },
    { e_Command_Wait,                   { 1.0f } },
};

// ---------------------------------------------------------------------------
// Level index table
// ---------------------------------------------------------------------------

static const PuppetShowLevel s_puppetShowLevels[] =
{
    { s_puppetLevelData_0,  arrsize(s_puppetLevelData_0)  },  // Level  1
    { s_puppetLevelData_1,  arrsize(s_puppetLevelData_1)  },  // Level  2
    { s_puppetLevelData_2,  arrsize(s_puppetLevelData_2)  },  // Level  3
    { s_puppetLevelData_3,  arrsize(s_puppetLevelData_3)  },  // Level  4
    { s_puppetLevelData_4,  arrsize(s_puppetLevelData_4)  },  // Level  5
    { s_puppetLevelData_5,  arrsize(s_puppetLevelData_5)  },  // Level  6
    { s_puppetLevelData_6,  arrsize(s_puppetLevelData_6)  },  // Level  7
    { s_puppetLevelData_7,  arrsize(s_puppetLevelData_7)  },  // Level  8
    { s_puppetLevelData_8,  arrsize(s_puppetLevelData_8)  },  // Level  9
    { s_puppetLevelData_9,  arrsize(s_puppetLevelData_9)  },  // Level 10
    { s_puppetLevelData_10, arrsize(s_puppetLevelData_10) },  // Level 11
    { s_puppetLevelData_11, arrsize(s_puppetLevelData_11) },  // Level 12
};

static const int k_numPuppetShowLevels = arrsize(s_puppetShowLevels);
