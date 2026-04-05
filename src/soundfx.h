// soundfx.h

#pragma once

void FMOD_ERRCHECK(int result);

void InitSound();
void PlayStereo( short player, short which );
void PlayStereoFrequency( short player, short which, float freq );
void PlayMono( short which );
void UpdateSound();

enum
{
	kShift = 0,
	kRotate,
	kPlace,
	kSquishy,
	kBounce,
	kSplop,
	kWhistle,
	kPause,
	kLoss,
	kVictory,
	kMagic,
	kWhomp,
	kChime,
	kClick,
	kLevelUp,
	kContinueSnd,
	kNumSounds
};

namespace FMOD { class System; }

extern bool             soundOn;
