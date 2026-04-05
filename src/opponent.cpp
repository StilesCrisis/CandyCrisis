// opponent.c

#include "stdafx.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "level.h"
#include "opponent.h"
#include "gworld.h"
#include "graphics.h"
#include "random.h"
#include "control.h"
#include "players.h"
#include "gameticks.h"
#include "blitter.h"
#include <vector>

enum OpponentFrameTypes
{
    e_OppFrameTypeIdle,
    e_OppFrameTypeLookLeft,
    e_OppFrameTypeLookRight,
    e_OppFrameTypeBlink1,
    e_OppFrameTypeBlink2,
    e_OppFrameTypeLevelOver,
    e_OppFrameTypeChatter,
    e_NumOppFrameTypes,
};

enum OpponentEmotionGfx
{
    e_EmotionNeutral,
    e_EmotionSad,
    e_EmotionHappy,
    e_NumEmotionGfx,
};

enum OpponentStateMachine
{
    e_StateIdle = 0,
    e_StateShiftyEyes,
    e_StateBlink1,
    e_StateBlink2,
    e_StateBlink3,
    e_StateChatter,
    e_StatePissed,
};

static std::vector<std::vector<CC_RGBSurface*>>  s_opponentSurface[e_NumEmotionGfx][e_NumOppFrameTypes]; // s_opponentSurface[emotion][frame type][underlayer frame][animation frame]
static MRect                        s_opponentWindowZRect;

MRect opponentWindowRect;

// There are three animation timers running here:
// -- The underlayer cycle has its own timer running at a constant rate.
// -- The top layer has "state" and "anim" frames, for lack of better terms.
// The "state" is tied to behavior, e.g. blinking and shifty-eyes.
// The "anim" selects a frame associated with the state art.
// i.e. the "Idle" graphic is a state frame, and if "Idle" cycles through multiple pictures, that cycling happens in "anim."
static OpponentStateMachine s_opponentState;
static OpponentEmotionGfx   s_opponentStateEmotion;
static OpponentFrameTypes   s_opponentStateFrame;
static unsigned int         s_opponentStateTime;
static unsigned int         s_opponentAnimTime;
static int                  s_opponentAnimFrame;
static unsigned int         s_opponentUnderlayerTime;
static int                  s_opponentUnderlayerFrame;
static unsigned int         s_opponentUnderlayerAnimSpeed;

static int                  s_opponentBobMax = 0;
static int                  s_opponentBobSpeed = 0;
static int                  s_opponentBobCurrent = 0;
static unsigned int         s_opponentBobTime;

// The glow arrays don't really belong here any more. Opponents no longer glow.
//int heavyGlowArray[kGlowArraySize], lightGlowArray[kGlowArraySize];
int glowArray[kGlowArraySize];


void InitOpponent()
{
    s_opponentWindowZRect = {0, 0, k_opponentHeight, k_opponentWidth};
	opponentWindowRect = s_opponentWindowZRect;
		
	for (int index=0; index<kGlowArraySize; index++ )
	{
		float value = sinf(float(index) * pi / kGlowArraySize );
		value *= value;
		
//		heavyGlowArray[index] = (int)(value * 0.75  * 256);
//		lightGlowArray[index] = (int)(value * 0.375 * 256);
        glowArray     [index] = (int)(value * 0.50  * 256);
	}
}

void BeginOpponent(int which, int bobAmount, int bobSpeed, int underlayerAnimSpeed)
{
    // For each of the opponent picture types...
    for (int emotionGfx=0; emotionGfx<e_NumEmotionGfx; ++emotionGfx)
    {
        // First, load new underlayer frames.
        std::vector<CC_RGBSurface*> underlayerVector;

        for (char underlayerFrame = 0;; ++underlayerFrame)
        {
            // First, try "N-UA".
            char suffix[64];
            snprintf(suffix, arrsize(suffix), "/%c-U%c", "NSH"[emotionGfx], 'A' + underlayerFrame);
            
            CC_RGBSurface* surface = LoadPICTAsRGBSurface(5000 + which, suffix);
            if (surface == nullptr)
            {
                // Fallback to "X-UA".
                snprintf(suffix, arrsize(suffix), "/X-U%c", 'A' + underlayerFrame);
                surface = LoadPICTAsRGBSurface(5000 + which, suffix);
            }
            
            if (surface == nullptr)
                break;
                
            if (surface->w != k_opponentWidth || surface->h != k_opponentHeight)
            {
                Gfx_FreeSurface(surface);
                break;
            }
            
            // We found one. Hold on to it.
            underlayerVector.push_back(surface);
        }

        // If no under-layers exist, synthesize a single blank one.
        if (underlayerVector.empty())
        {
            CC_RGBSurface* surface = Gfx_InitRGBSurface(k_opponentWidth, k_opponentHeight);
            underlayerVector.push_back(surface);
        }
        
        // Now load the opponent animations.
        for (int frameType=0; frameType<e_NumOppFrameTypes; ++frameType)
        {
            std::vector<std::vector<CC_RGBSurface*>>& surfaceUAVector = s_opponentSurface[emotionGfx][frameType];
            
            // ... first, nuke any data we had before.
            for (std::vector<CC_RGBSurface*>& surfaceAVector: surfaceUAVector)
            {
                for (CC_RGBSurface* surface: surfaceAVector)
                {
                    Gfx_FreeSurface(surface);
                }
            }
            
            surfaceUAVector.clear();
            surfaceUAVector.resize(underlayerVector.size());
            
            // Now, load as many animations as we find as 5000/N-1A, 5000/N-1B, 5000/N-1C, etc.
            for (char anim = 0;;)
            {
                char suffix[64];
                snprintf(suffix, arrsize(suffix), "/%c-%d%c", "NSH"[emotionGfx], frameType + 1, 'A' + anim);

                CC_RGBSurface* surface = LoadPICTAsRGBSurface(5000 + which, suffix);
                if (surface == nullptr)
                    break;

                if (surface->w != k_opponentWidth || surface->h != k_opponentHeight)
                {
                    Gfx_FreeSurface(surface);
                    break;
                }
                
                // We found an anim frame! Blend it with each underlayer frame.
                for (int underlayerIndex = int(underlayerVector.size()); underlayerIndex--; )
                {
                    if (underlayerIndex > 0)
                    {
                        CC_Rect opponentCCRect = { 0, 0, k_opponentWidth, k_opponentHeight };
                        CC_RGBSurface* combinedSurface = Gfx_InitRGBSurface(k_opponentWidth, k_opponentHeight);
                        Gfx_BlitSurface(surface, &opponentCCRect, combinedSurface, &opponentCCRect);
                        SurfaceBlendUnder(combinedSurface, &s_opponentWindowZRect, underlayerVector[underlayerIndex], &s_opponentWindowZRect);
                        surfaceUAVector[underlayerIndex].push_back(combinedSurface);
                    }
                    else
                    {
                        SurfaceBlendUnder(surface, &s_opponentWindowZRect, underlayerVector[underlayerIndex], &s_opponentWindowZRect);
                        surfaceUAVector[underlayerIndex].push_back(surface);
                    }
                }
                
                // Increment.
                ++anim;
            }
        }

        // Dump the under-layers.
        for (CC_RGBSurface* oldSurface: underlayerVector)
        {
            Gfx_FreeSurface(oldSurface);
        }
    }
    
    // Set up opponent data.
    s_opponentState = e_StateIdle;
    s_opponentStateEmotion = e_EmotionNeutral;
    s_opponentStateFrame = e_OppFrameTypeIdle;
    s_opponentStateTime = GameTickCount();
    s_opponentAnimTime = s_opponentStateTime;
    s_opponentAnimFrame = 0;
    s_opponentUnderlayerTime = s_opponentStateTime;
    s_opponentUnderlayerFrame = 0;
    s_opponentBobMax = bobAmount;
    s_opponentBobSpeed = bobSpeed;
    s_opponentUnderlayerAnimSpeed = underlayerAnimSpeed;
    s_opponentBobCurrent = 0;
    s_opponentBobTime = s_opponentStateTime;
	
	emotions[0] = emotions[1] = kEmotionNeutral;
}

void OpponentPissed( void )
{
	s_opponentState = e_StatePissed;
	s_opponentStateTime = GameTickCount();
}

void OpponentChatter( MBoolean on )
{
	switch( on )
	{
		case true:
			s_opponentState = e_StateChatter;
			s_opponentStateTime = GameTickCount();
			break;
			
		case false:
			s_opponentState = e_StateIdle;
			s_opponentStateTime = GameTickCount();
			break;
	}
}

void UpdateOpponent()
{
    static const OpponentEmotionGfx emotiMap[] = { e_EmotionNeutral, e_EmotionSad, e_EmotionHappy, e_EmotionSad /*panic*/};
    bool                            draw = false;
    unsigned int                    now = GameTickCount();
    
    // Update the "bob" at 30Hz.
    if (s_opponentBobMax > 0 && now > s_opponentBobTime)
    {
        int opponentBobOld = s_opponentBobCurrent;
        
        s_opponentBobCurrent = int(s_opponentBobMax + s_opponentBobMax * cos(float(now) / float(s_opponentBobSpeed)));
        s_opponentBobTime += 2;
        
        if (opponentBobOld != s_opponentBobCurrent)
        {
            draw = true;
        }
    }
    
    // First: increment the "underlayer" frame at 10Hz.
    int numUnderlayerFrames = int(s_opponentSurface[s_opponentStateEmotion][s_opponentStateFrame].size());
    if (numUnderlayerFrames > 1 && now > s_opponentUnderlayerTime)
    {
        s_opponentUnderlayerTime += s_opponentUnderlayerAnimSpeed;
        s_opponentUnderlayerFrame = (s_opponentUnderlayerFrame + 1) % numUnderlayerFrames;
        draw = true;
    }

    // Next: increment the "anim" frame at 10Hz. (This COULD be off-beat from the underlayer cycle.
    // That's life!)
    int numAnimFrames = int(s_opponentSurface[s_opponentStateEmotion][s_opponentStateFrame][s_opponentUnderlayerFrame].size());
    if (numAnimFrames > 1 && now > s_opponentAnimTime)
    {
        s_opponentAnimTime += 6;
        s_opponentAnimFrame = (s_opponentAnimFrame + 1) % numAnimFrames;
        draw = true;
    }

    // Next: update the "state" frame as desired (which can reset the anim frame)
	if (now > s_opponentStateTime)
	{
        s_opponentStateEmotion = emotiMap[emotions[1]];

        switch( s_opponentState )
		{
			case e_StateIdle:
                if (role[1] == kWinning || role[1] == kLosing)
                {
                    s_opponentStateTime += 6000;
                    s_opponentState = e_StateIdle;
                    s_opponentStateFrame = e_OppFrameTypeLevelOver;
                }
                else
                {
                    s_opponentStateTime += 60 + RandomBefore(180);
                    s_opponentState = RandomBefore(2)? e_StateShiftyEyes: e_StateBlink1;
                    s_opponentStateFrame = e_OppFrameTypeIdle;
                }
				break;
			
			case e_StateShiftyEyes:
				s_opponentStateTime += 40 + RandomBefore(60);
				s_opponentState = e_StateIdle;
                s_opponentStateFrame = RandomBefore(2)? e_OppFrameTypeLookRight: e_OppFrameTypeLookLeft;
				break;

			case e_StateBlink1:	
				s_opponentStateTime += 3;
				s_opponentState = e_StateBlink2;
				s_opponentStateFrame = e_OppFrameTypeBlink1;
				break;
			
			case e_StateBlink2:
				s_opponentStateTime += 3;
				s_opponentState = e_StateBlink3;
				s_opponentStateFrame = e_OppFrameTypeBlink2;
				break;
			
			case e_StateBlink3:
				s_opponentStateTime += 3;
				s_opponentState = e_StateIdle;
				s_opponentStateFrame = e_OppFrameTypeBlink1;
				break;
			
			case e_StateChatter:
				s_opponentStateTime += 6000;
				s_opponentState = e_StateIdle;
                s_opponentStateEmotion = e_EmotionNeutral /* we don't make chatter frames for other emotions */;
                s_opponentStateFrame = e_OppFrameTypeChatter;
				break;
			
			case e_StatePissed:
				s_opponentStateTime += 60;
                s_opponentStateEmotion = e_EmotionSad;
                s_opponentStateFrame = e_OppFrameTypeLookLeft;
				s_opponentState = e_StateIdle;
				break;
		}
		
        s_opponentAnimFrame = 0;
		s_opponentAnimTime = now;
		draw = true;
	}
		
	if (draw)
	{
        DrawOpponent();
	}
}

void DrawOpponent()
{
    CC_Rect srcSDLRect, dstSDLRect;
    
    Gfx_BlitFrontSurface(backdropSurface,
                         Gfx_MRectToCCRect( &opponentWindowRect, &srcSDLRect ),
                         Gfx_MRectToCCRect( &opponentWindowRect, &dstSDLRect )  );

    if (s_opponentUnderlayerFrame < s_opponentSurface[s_opponentStateEmotion][s_opponentStateFrame].size() &&
        s_opponentAnimFrame       < s_opponentSurface[s_opponentStateEmotion][s_opponentStateFrame][s_opponentUnderlayerFrame].size())
    {
        MRect opponentBobbedRect = opponentWindowRect;
        OffsetMRect(&opponentBobbedRect, 0, s_opponentBobCurrent);
        
        MRect opponentSourceRect = s_opponentWindowZRect;
        int truncateAmount = opponentBobbedRect.bottom - 1080;
        if (truncateAmount > 0)
        {
            opponentBobbedRect.bottom -= truncateAmount;
            opponentSourceRect.bottom -= truncateAmount;
        }
        
        SurfaceBlendOver(g_frontSurface, &opponentBobbedRect,
                         s_opponentSurface[s_opponentStateEmotion][s_opponentStateFrame][s_opponentUnderlayerFrame][s_opponentAnimFrame], &opponentSourceRect);
    }
}

