// next.h


void InitNext( void );
void ShowNext( int player );
void RefreshNext( int player );
void UpdateNext( int player );
void PullNext( int player );
void ShowPull( int player );

struct CC_RGBSurface;

extern CC_RGBSurface* nextSurface[2];
extern CC_RGBSurface* nextDrawSurface;

extern MBoolean nextWindowVisible[2];
extern MRect nextWindowZRect, nextWindowRect[2];
