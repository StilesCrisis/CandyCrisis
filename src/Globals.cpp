///
///  Globals.cpp
///

#include "stdafx.h"
#include "Globals.h"
#include "gworld.h"


signed char  nextA[2], nextB[2], nextM[2], nextG[2], colorA[2], colorB[2],
             blobX[2], blobY[2], blobR[2], blobSpin[2], speed[2], role[2], halfway[2],
             control[2], dropping[2], magic[2], grenade[2], anim[2];
int          blobTime[2], startTime, endTime;
MBoolean     finished = false, pauseKey = false, showStartMenu = true;
signed char  grid[2][kGridAcross][kGridDown], suction[2][kGridAcross][kGridDown], charred[2][kGridAcross][kGridDown], glow[2][kGridAcross][kGridDown];
MRect        playerWindowRect[2], playerWindowZRect;
KeyList      hitKey[2];
void         (*DoFullRepaint)() = nullptr;
int          g_levelBeaten = 0;
int          g_allTimeLongestChain = 0;
int          g_allTimeMaxClusters = 0;
int          g_allTimeBiggestBomb = 0;
bool         g_dropLockEnabled = false;
bool         g_cheatsEnabled = false;
int          g_iPadScreenLeftEdge;
#if __APPLE__
CGPoint      g_iPadPauseButtonCenter;
iPadVirtualController g_iPadVirtualController[2];
#endif
