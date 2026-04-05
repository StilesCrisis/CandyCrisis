// soundfx.c

#include "stdafx.h"

#include <string.h>
#include <stdio.h>

#include "main.h"
#include "music.h"
#include "gworld.h"
#include "gameticks.h"
#include "soundfx.h"
#include "graphics.h"
#include "gworld.h"

#ifndef __EMSCRIPTEN__
#include "fmod.hpp"
#include "fmod_errors.h"
#endif


const int               k_noMusic = -1;
const int               k_songs = 14;

bool                    musicOn = true;
int                     musicSelection = k_noMusic;
bool                    soundOn = true;

#ifndef __EMSCRIPTEN__

static MBoolean         s_musicFast = false;
int                     s_musicPaused = 0;
static FMOD::Channel*   s_musicChannel = NULL;
static FMOD::Sound*     s_musicModule = NULL;

static FMOD::System    *g_fmod;
static FMOD::Sound     *s_sound[kNumSounds];

void FMOD_ERRCHECK(int result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(FMOD_RESULT(result)));
        abort();
    }
}

void InitSound( void )
{
    FMOD_RESULT   result = FMOD::System_Create(&g_fmod);
    FMOD_ERRCHECK(result);
    
    unsigned int  version;
    result = g_fmod->getVersion(&version);
    FMOD_ERRCHECK(result);
    
    if (version < FMOD_VERSION)
    {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        abort();
    }
    
    result = g_fmod->init(64, FMOD_INIT_NORMAL, 0);
    FMOD_ERRCHECK(result);
    
    for (int index=0; index<kNumSounds; index++)
    {
        /* NOTE: don't replace the sound flags with FMOD_DEFAULT! This will make some WAVs loop (and fail to release their channels). */
        result = g_fmod->createSound(QuickResourceName("snd", index+128, ".wav"), FMOD_LOOP_OFF | FMOD_2D, 0, &s_sound[index]);
        FMOD_ERRCHECK(result);
    }
}


void PlayMono( short which )
{
    PlayStereoFrequency(2, which, 0);
}

void PlayStereo( short player, short which )
{
    PlayStereoFrequency(player, which, 0);
}

void PlayStereoFrequency( short player, short which, float freq )
{
    struct SpeakerMix
    {
        float left, right, center;
    };
    
    SpeakerMix speakerMixForPlayer[] =
    {
        { 1.0, 0.0, 0.0 },
        { 0.0, 1.0, 0.0 },
        { 0.0, 0.0, 1.0 },
    };
    
    const SpeakerMix& mix = speakerMixForPlayer[player];
    
    if (soundOn)
    {
        FMOD::Channel*    channel = NULL;
        FMOD_RESULT       result = g_fmod->playSound(s_sound[which], nullptr, true, &channel);
        FMOD_ERRCHECK(result);
        
        // setSpeakerMix removed in FMOD 2.x; use setPan(-1=left, 0=center, +1=right)
        result = channel->setPan(mix.right - mix.left);
        FMOD_ERRCHECK(result);
        
        float channelFrequency;
        result = s_sound[which]->getDefaults(&channelFrequency, nullptr);
        FMOD_ERRCHECK(result);
        
        result = channel->setFrequency((channelFrequency * (16 + freq)) / 16);
        FMOD_ERRCHECK(result);
        
        result = channel->setPaused(false);
        FMOD_ERRCHECK(result);
        
        UpdateSound();
    }
}

void UpdateSound()
{
    g_fmod->update();
}

void EnableMusic( bool on )
{
    if (s_musicChannel)
    {
        FMOD_RESULT result = s_musicChannel->setVolume(on? 0.75f: 0.0f);
        FMOD_ERRCHECK(result);
    }
}

void FastMusic( void )
{
    if (s_musicModule && !s_musicFast)
    {
        FMOD_RESULT result = s_musicModule->setMusicSpeed(1.3f);
        FMOD_ERRCHECK(result);
        
        s_musicFast = true;
    }
}

void SlowMusic( void )
{
    if (s_musicModule && s_musicFast)
    {
        FMOD_RESULT result = s_musicModule->setMusicSpeed(1.0f);
        FMOD_ERRCHECK(result);
        
        s_musicFast = false;
    }
}

void PauseMusic( void )
{
    if (s_musicChannel)
    {
        FMOD_RESULT result = s_musicChannel->setPaused(true);
        FMOD_ERRCHECK(result);
        
        s_musicPaused++;
    }
}

void ResumeMusic( void )
{
    if (s_musicChannel)
    {
        FMOD_RESULT result = s_musicChannel->setPaused(false);
        FMOD_ERRCHECK(result);
        
        s_musicPaused--;
    }
}

void ChooseMusic( short which )
{
    if (s_musicChannel != NULL)
    {
        s_musicChannel->stop();
        s_musicChannel = NULL;
    }
    
    if (s_musicModule != NULL)
    {
        s_musicModule->release();
        s_musicModule = NULL;
    }
    
    musicSelection = -1;
    
    if (which >= 0 && which <= k_songs)
    {
        FMOD_RESULT result = g_fmod->createSound(QuickResourceName("mod", which+128, ""), FMOD_DEFAULT, 0, &s_musicModule);
        FMOD_ERRCHECK(result);
        
        result = g_fmod->playSound(s_musicModule, nullptr, true, &s_musicChannel);
        FMOD_ERRCHECK(result);
        
        result = s_musicChannel->setPriority(10); // prioritize music first--WAVs should never knock out a MOD
        FMOD_ERRCHECK(result);
        
        EnableMusic(musicOn);
        
        result = s_musicModule->setLoopCount(-1);
        FMOD_ERRCHECK(result);
        
        result = s_musicChannel->setPaused(false);
        FMOD_ERRCHECK(result);
        
        musicSelection = which;
        s_musicPaused  = 0;
    }
}

#else // __EMSCRIPTEN__ — audio via libopenmpt (music) + Web Audio API (SFX)
//
// libopenmpt is compiled into the WASM binary and renders MOD/S3M PCM frames
// into two float buffers that audio.js drains via a ScriptProcessorNode.
// Sound effects use Web Audio API AudioBufferSourceNode with playbackRate,
// giving us full pitch control matching FMOD's (16 + freq) / 16 formula.
//
// BSD 2-Clause notice (libopenmpt):
//   Copyright (c) 2004-2024, OpenMPT Project Developers and Contributors
//   Copyright (c) 1997-2003, Olivier Lapicque
//   All rights reserved.  Redistribution and use in source and binary forms,
//   with or without modification, are permitted provided that the conditions
//   of the BSD 2-Clause licence are met.

#include "libopenmpt/libopenmpt.h"

int s_musicPaused = 0;

static openmpt_module* s_openmptMod    = nullptr;
static int             s_sampleRate    = 44100;
static MBoolean        s_musicFast     = false;

void FMOD_ERRCHECK(int) {}

// ---------------------------------------------------------------------------
// Called from the ScriptProcessorNode in audio.js on every audio callback.
// Renders one buffer of stereo float PCM from the current libopenmpt module.
// Must not call emscripten_sleep() or any ASYNCIFY function — it runs on the
// browser's audio thread (actually main thread for ScriptProcessorNode).
// ---------------------------------------------------------------------------
extern "C" EMSCRIPTEN_KEEPALIVE
void CandyCrisisAudio_RenderMusic(float* left, float* right, int frames)
{
    if (s_openmptMod && s_musicPaused == 0)
    {
        openmpt_module_read_float_stereo(s_openmptMod, s_sampleRate, frames, left, right);
    }
    else
    {
        memset(left,  0, frames * sizeof(float));
        memset(right, 0, frames * sizeof(float));
    }
}

void InitSound()
{
    // Initialise the JS-side Web Audio context and ScriptProcessorNode.
    EM_ASM({ CandyCrisisAudio.init(); });

    // Store the AudioContext's native sample rate for libopenmpt rendering.
    s_sampleRate = EM_ASM_INT({ return CandyCrisisAudio.getSampleRate(); });

    // Decode each WAV into a Web Audio AudioBuffer (JS side, from Emscripten FS).
    for (int i = 0; i < kNumSounds; i++)
    {
        EM_ASM({ CandyCrisisAudio.loadSound($0, UTF8ToString($1)); },
               i, QuickResourceName("snd", i + 128, ".wav"));
    }
}

void PlayStereoFrequency(short player, short which, float freq)
{
    if (!soundOn || which < 0 || which >= kNumSounds) return;

    // Replicate FMOD's pitch formula: newFreq = originalFreq * (16 + freq) / 16
    float pitchRatio = (16.0f + freq) / 16.0f;

    // pan: -1 = full left (player 0), +1 = full right (player 1), 0 = centre
    float pan = (player == 0) ? -1.0f : (player == 1) ? 1.0f : 0.0f;

    EM_ASM({ CandyCrisisAudio.playSound($0, $1, $2); },
           which, pitchRatio, pan);
}

void PlayMono(short which)
{
    PlayStereoFrequency(2, which, 0);
}

void PlayStereo(short player, short which)
{
    PlayStereoFrequency(player, which, 0);
}

void UpdateSound() {}

void EnableMusic(bool on)
{
    // Only controls volume — does NOT write musicOn (user preference).
    // Matches FMOD behaviour.
    EM_ASM({ CandyCrisisAudio.setMusicVolume($0 ? 0.75 : 0.0); }, on ? 1 : 0);
}

void FastMusic()
{
    if (s_openmptMod && !s_musicFast)
    {
        openmpt_module_ctl_set_floatingpoint(s_openmptMod, "play.tempo_factor", 1.3);
        s_musicFast = true;
    }
}

void SlowMusic()
{
    if (s_openmptMod && s_musicFast)
    {
        openmpt_module_ctl_set_floatingpoint(s_openmptMod, "play.tempo_factor", 1.0);
        s_musicFast = false;
    }
}

void PauseMusic()
{
    s_musicPaused++;
    // No JS call needed — the render function checks s_musicPaused directly.
}

void ResumeMusic()
{
    if (s_musicPaused > 0)
        s_musicPaused--;
}

void ChooseMusic(short which)
{
    if (s_openmptMod)
    {
        openmpt_module_destroy(s_openmptMod);
        s_openmptMod = nullptr;
    }
    s_musicFast    = false;
    musicSelection = k_noMusic;

    if (which < 0 || which > k_songs) return;

    const char* path = QuickResourceName("mod", which + 128, "");
    FILE* f = fopen(path, "rb");
    if (!f)
    {
        fprintf(stderr, "ChooseMusic: cannot open %s\n", path);
        return;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    void* data = malloc(size);
    fread(data, 1, size, f);
    fclose(f);

    s_openmptMod = openmpt_module_create_from_memory2(
        data, (size_t)size,
        nullptr, nullptr,  // log callback + user
        nullptr, nullptr,  // error callback + user
        nullptr,           // error out
        nullptr,           // error message out
        nullptr);          // ctls
    free(data);

    if (!s_openmptMod)
    {
        fprintf(stderr, "ChooseMusic: openmpt failed to load mod %d\n", which + 128);
        return;
    }

    openmpt_module_set_repeat_count(s_openmptMod, -1);  // loop forever

    musicSelection = which;
    s_musicPaused  = 0;
    EnableMusic(musicOn);
}

#endif // __EMSCRIPTEN__