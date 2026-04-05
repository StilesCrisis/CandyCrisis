// graymonitor.h

#pragma once

struct CC_RGBSurface;


void InitGrayMonitors();
void ShowGrayMonitor( short player );


extern MRect            grayMonitorZRect, grayMonitorRect[2];
extern MBoolean         grayMonitorVisible[2];
extern CC_RGBSurface*   grayMonitorSurface[2];
