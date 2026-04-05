///
///  Globals.h
///

#pragma once
#include "MTypes.h"
#include "Platform.h"
#if __APPLE__
#include <CoreGraphics/CoreGraphics.h>

struct iPadVirtualController
{
    CGPoint   m_translation;
    int       m_tap;

    // internal use:
    uint32_t  m_tapStartTime;
    CGPoint   m_tapStartPoint;
};
#endif


struct KeyList
{
    short left, right, drop, rotate;
};


#define kGridAcross 6
#define kGridDown 13


enum
{
    picBlob = 200,
    picBlobMask = 201,
    picCharMask = 202,
    picBlast = 206,
    picBlastMask = 207,
    picFont = 250,
    picSkiaFont,
    picContinueFont,
    picBalloonFont_OBSOLETE, // was New Berolina MT
    picZapFont,
    picZapOutlineFont,
    picVictoryFont,
    picDashedLineFont,
    picSkiaBoldFont,
    picLastFont,
    picTitle = 300,
    picGameStart = 302,
    picGameOver = 303,
    picBlackStripe = 400,
    picLogo,
    picTextBalloon,         // 402 — speech balloon used in puppet show
    picPauseOverlay,        // 403
    picPauseIcon,           // 404

    // World Select screen assets (sourced from Candy Crisis TV storyboard assets)
    picWorldSelect   = 500,
    picCongaBlue     = 501,
    picCongaGreen    = 502,
    picCongaRed      = 503,
    picCongaPurple   = 504,
    picStar0         = 510,
    picStar1         = 511,
    picStar2         = 512,
    picStar3         = 513,
    picStar4         = 514,
    picStarburst     = 520,

    // Blob rain assets (World 1 victory screen)
    picBlob0 = 530, picBlob1, picBlob2, picBlob3, picBlob4, picBlob5, picBlob6,
    picSplat0 = 537, picSplat1, picSplat2, picSplat3, picSplat4, picSplat5, picSplat6,
    picTextHalo = 544,

    // World 1 victory layers
    picWorld1Layer1 = 600, picWorld1Layer2, picWorld1Layer3, picWorld1Layer4,
    picWorld1Layer5, picWorld1Layer6, picWorld1Layer7, picWorld1Layer8,
    picWorld1Layer9, picWorld1Layer10,

    // World 2 victory layers
    picWorld2Layer1 = 610,
    picWorld2Layer2L, picWorld2Layer2R, picWorld2Layer3L, picWorld2Layer3R,
    picWorld2Layer4, picWorld2Layer5, picWorld2Layer6, picWorld2Layer7,
    picWorld2Layer8, picWorld2Layer9, picWorld2Layer10,

    // World 3 victory layers (layers 6-10 reuse World 2 PICT_617-621)
    picWorld3Layer1 = 630,
    picWorld3Layer2L, picWorld3Layer2R, picWorld3Layer3L, picWorld3Layer3R,
    picWorld3Layer4, picWorld3Layer5,
    picWorld3Composite      = 637,
    picMarshmallowsFarther  = 638,
    picMarshmallowsFarLight = 639,
    picMarshmallowsFar      = 640,
    picMarshmallowsNear     = 641,

    // Credits screen layers (Campfire-Layer-1..5)
    picCreditsLayer1    = 642,  // Campfire-Layer-1.png  (1920×1080, static)
    picCreditsLayer2    = 643,  // Campfire-Layer-2.png  (1920×1599, Y: +400→-250)
    picCreditsLayer3    = 644,  // Campfire-Layer-3.png  (1920×1599, X: -500→+500)
    picCreditsLayer4    = 645,  // Campfire-Layer-4.png  (1920×1599, Y: -500→-200)
    picCreditsLayer5    = 646,  // Campfire-Layer-5.png  (1920×1599, Y: +400→+300)

    // PostCredits screen layers (Marshmallow-Roast-Layer-1..3, all 1920×1080)
    picMarshmallowRoast1 = 647,
    picMarshmallowRoast2 = 648,
    picMarshmallowRoast3 = 649,
};

enum
{
    winPlayer = 128,
    winBackdrop,
    winNext,
    winScore,
    winOpponent,
    winTitle,
    winVictory,
    winLevel
};

enum
{
    rightRotate = 0,
    downRotate,
    leftRotate,
    upRotate
};

enum
{
    kEmpty = 0,
    kBlob,
    kBlob2,
    kBlob3,
    kBlob4,
    kBlob5,
    kBlob6,
    kBlob7,
    kBombTop,
    kBombBottom,
    kGray,
    kLight,
    kSun
};

enum
{
    kNoSuction		 = 0,
    kUp				 = 1,
    kRight			 = 2,
    kUpRight		 = 3,
    kDown			 = 4,
    kUpDown			 = 5,
    kRightDown		 = 6,
    kUpRightDown	 = 7,
    kLeft			 = 8,
    kLeftUp			 = 9,
    kLeftRight 		 = 10,
    kLeftUpRight	 = 11,
    kLeftDown		 = 12,
    kLeftUpDown		 = 13,
    kLeftRightDown	 = 14,
    kLeftUpRightDown = 15,
    kDying			 = 16,
    kSquish			 = 17,
    kSquash 		 = 18,
    kSquish1         = 19,
    kSquish2         = 20,
    kSquish3         = 21,
    kSquish4         = 22,
    kBlinkBlob       = 23,
    kSobBlob         = 24,
    kSob2Blob        = 25,
    kFlashDarkBlob   = 26,
    kFlashBrightBlob = 27,
    kJiggle1         = 28,
    kJiggle2         = 29,
    kJiggle3		 = 30,
    kJiggle4		 = 31,
    kJiggle5		 = 32,
    kJiggle6		 = 33,
    kJiggle7		 = 34,
    kJiggle8		 = 35,
    kInDoubt		 = 36,
    kInDeath		 = 37
};

enum
{
    kNoCharring      = 0,
    kBombFuse1       = 0,
    kBombFuse2       = 1,
    kBombFuse3       = 2,
    kBlinkBomb1      = 3,
    kBlinkBomb2      = 4,
    kBlinkBomb3      = 5,
    kChar11,
    kChar31,
    kChar12,
    kChar32,
    kChar13,
    kChar33,
    kChar14,
    kChar24,
    kChar34
};

enum
{
    kDarkChar = 0xF0,
    kLightestChar = 0x00
};

enum
{
    kFlashAnimation  = 0,
    kJiggleAnimation
};

#define kBlobFrames (kFlashBrightBlob+1)

enum
{
    kGrayNoBlink = 0,
    kGrayBlink1,
    kGrayBlink2,
    kGrayBlink3,
    kSunGlow1,
    kSunGlow2,
    kSunGlow3,
    kSunGlow4,
    kSmallGray1,
    kSmallGray1b,
    kSmallGray2,
    kSmallGray2b,
    kSmallGray3,
    kSmallGray3b,
    kSmallGray4,
    kSmallGray4b,
    kSmallGray5,
    kSmallGray5b,
    kEasyGray = 25,
    kHardGray,
    kStageGray
};
#define kGrayFrames (kGrayDying4+1)

#define kFirstBlob kBlob
#define kLastBlob kBlob7
#define kBlobTypes (kLastBlob - kFirstBlob + 1)

#define pi M_PI

#define arrsize(x) int(sizeof((x)) / sizeof((x)[0]))


extern signed char nextA[2], nextB[2], nextM[2], nextG[2], colorA[2], colorB[2],
                   blobX[2], blobY[2], blobR[2], blobSpin[2], speed[2], role[2], halfway[2],
                   control[2], dropping[2], magic[2], grenade[2], anim[2];
extern int blobTime[2], startTime, endTime;
extern MBoolean finished, pauseKey, showStartMenu;
extern signed char grid[2][kGridAcross][kGridDown], suction[2][kGridAcross][kGridDown],
                   charred[2][kGridAcross][kGridDown], glow[2][kGridAcross][kGridDown];
extern MRect playerWindowRect[2], playerWindowZRect;
extern KeyList hitKey[2];
extern int backgroundID;
extern void (*DoFullRepaint)();
extern int     g_levelBeaten;
extern int     g_allTimeLongestChain;
extern int     g_allTimeMaxClusters;
extern int     g_allTimeBiggestBomb;
extern bool    g_dropLockEnabled;
extern bool    g_cheatsEnabled;
extern int     g_iPadScreenLeftEdge;
#if __APPLE__
extern CGPoint g_iPadPauseButtonCenter;
extern iPadVirtualController g_iPadVirtualController[2];
#endif

#define kLeftPlayerWindowCenter  0.15
#define kRightPlayerWindowCenter 0.85
#define kScoreWindowVertCenter   0.98
#define kGrayMonitorVertCenter   0.015
#define kNextWindowVertCenter1P  0.13
#define kNextWindowVertCenter2P  0.13
