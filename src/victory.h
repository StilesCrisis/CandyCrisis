// victory.h

void EndRound( int player );
void BeginVictory( int player );
void InitVictory();
void Lose( int player );
void Win( int player );
void DropLosers( int player, int skip );
void DrawTimerCount( int player );
void DrawTimerBonus( int player );
void ReportWorldClearAchievements();

#define kNumLetters 11
#define kLetterHorizSize 13
#define kLetterVertSize 20
