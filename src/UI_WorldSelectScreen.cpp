///
///  UI_WorldSelectScreen.cpp
///

#include "stdafx.h"
#include "UI_WorldSelectScreen.h"
#include "blitter.h"
#include "gworld.h"
#include "Globals.h"
#include "level.h"
#include "main.h"
#include "music.h"
#include "soundfx.h"
#include "SDLU.h"
#include "Platform.h"
#include "UI_PuppetShow.h"
#include "font.h"
#include <cmath>
#include <algorithm>

using std::min;
using std::max;


// Storyboard-derived layout constants (1920×1080 coordinate space)
static const short kWorldButtonLeft[3]     = { 630, 630, 630 };
static const short kWorldButtonTop[3]      = {  50, 415, 781 };
static const short kWorldButtonRight[3]    = { 1289, 1289, 1289 };
static const short kWorldButtonBottom[3]   = { 136,  501,  867 };

static const short kWorldLabelLeft[3]      = { 497, 482, 482 };
static const short kWorldLabelTop[3]       = { 155, 521, 899 };
static const short kWorldLabelRight[3]     = { 1422, 1438, 1438 };
static const short kWorldLabelBottom[3]    = { 300, 666, 1044 };

static const short kStarLeft[3]            = { 159, 159, 159 };
static const short kStarTop[3]             = {  84, 450, 824 };

static const short kStarburstLeft[3]       = { 129, 129, 109 };
static const short kStarburstTop[3]        = {  54, 420, 774 };

// Conga base Y positions and animation parameters
static const int   kCongaBaseY[4]          = { 327, 327, 692, 692 }; // Blue, Green, Red, Purple
static const float kCongaBounce[4]         = { 20.f, 25.f, 20.f, 25.f };
static const float kCongaPhase[4]          = { 0.f, (float)M_PI, 0.f, (float)M_PI };
static const bool  kCongaScrollRight[4]    = { true, true, false, false };

// Minimum g_levelBeaten to unlock each world
static const int   kWorldUnlockLevel[3]    = { 0, 4, 8 };

// Back button — top-left corner, touch platforms only.
// Derived from storyboard rect (-44,-13,237×88) on 1024×768, scaled to 1920×1080.
static const MRect kBackButtonRect         = { 0, 30, 110, 380 };

// Text Strings — line splits match the Apple TV screenshot exactly.
static const char* kWorld1ButtonText = "World 1";
static const char* kWorld1Lines[3] = {
    "A catastrophe at Candy Mountain has mysteriously",
    "led to mutant candies across the land. A daring",
    "adventure begins to discover the cause!",
};

static const char* kWorld2ButtonTextLocked   = "World 2 - Locked";
static const char* kWorld2ButtonTextUnlocked = "World 2";
static const char* kWorld2LabelTextLocked    = "Defeat World 1 to unlock!";
static const char* kWorld2Lines[3] = {
    "The journey continues past the safety of the village,",
    "through the Candy Forest, and all the way to the base",
    "of Candy Mountain.",
};

static const char* kWorld3ButtonTextLocked   = "World 3 - Locked";
static const char* kWorld3ButtonTextUnlocked = "World 3";
static const char* kWorld3LabelTextLocked    = "Defeat World 2 to unlock!";
static const char* kWorld3Lines[3] = {
    "Only the bravest adventurers have dared scale Candy",
    "Mountain. Can the dragon possibly be stopped? Or",
    "will mutant candy overrun the world?",
};

static void DrawCongaLine(CC_RGBSurface* drawSurf, CC_RGBSurface* congaSurf,
                            int baseY, int srcX)
{
    if (!congaSurf) return;

    int W = congaSurf->w;
    int H = congaSurf->h;
    int destX = 0;

    while (destX < 1920)
    {
        int remSrc  = W - srcX;          // pixels available before image wraps
        int remDest = 1920 - destX;      // pixels remaining to fill on screen
        int blit    = min(remSrc, remDest);

        MRect srcRect = { 0,           (short)srcX,           (short)H,            (short)(srcX  + blit) };
        MRect dstRect = { (short)baseY, (short)destX, (short)(baseY + H), (short)(destX + blit) };
        SurfaceBlendOver(drawSurf, &dstRect, congaSurf, &srcRect);

        destX += blit;
        srcX   = 0;  // next iteration continues from start of image
    }
}


static void RenderWorldSelectFrame(CC_RGBSurface* bgSurface, CC_RGBSurface* drawSurface,
                                   CC_RGBSurface* starburstSurface, CC_RGBSurface* (&starSurface)[5],
                                   CC_RGBSurface* (&congaSurface)[4],
                                   int (&buttonHighlight)[3], int& backHighlight,
                                   SDL_Texture* capsuleTexture,
                                   MPoint mouse, uint32_t t_ms)
{
    // 1. Draw background
    Gfx_BlitSurface(bgSurface, &bgSurface->clip_rect, drawSurface, &drawSurface->clip_rect);

    // 2. Draw starbursts (show when world is fully beaten)
    if (starburstSurface)
    {
        double angle = (double)t_ms * 0.05;
        SDL_Texture* prevTarget = SDL_GetRenderTarget(g_renderer);
        SDL_SetRenderTarget(g_renderer, drawSurface->texture);

        for (int w = 0; w < 3; ++w)
        {
            if (g_levelBeaten >= (w + 1) * 4)
            {
                // The World 3 starburst is larger in the storyboard (280x280 vs 240x240).
                int dstW = starburstSurface->w;
                int dstH = starburstSurface->h;
                if (w == 2) {
                    dstW = 280;
                    dstH = 280;
                }
                SDL_Rect srcRect = { 0, 0, starburstSurface->w, starburstSurface->h };
                SDL_Rect dstRect = { kStarburstLeft[w], kStarburstTop[w], dstW, dstH };
                SDL_RenderCopyEx(g_renderer, starburstSurface->texture, &srcRect, &dstRect, angle, nullptr, SDL_FLIP_NONE);
            }
        }
        SDL_SetRenderTarget(g_renderer, prevTarget);
    }

    // 3. Draw stars (progress indicator per world)
    for (int w = 0; w < 3; ++w)
    {
        int base = w * 4;
        int starIdx;
        if      (g_levelBeaten >= base + 4) starIdx = 4;
        else if (g_levelBeaten >= base + 3) starIdx = 3;
        else if (g_levelBeaten >= base + 2) starIdx = 2;
        else if (g_levelBeaten >= base + 1) starIdx = 1;
        else                                starIdx = 0;

        CC_RGBSurface* star = starSurface[starIdx];
        if (star)
        {
            // Stars are scaled to 180x180 in the storyboard, but the source image is 240x240.
            MRect srcRect = { 0, 0, (short)star->h, (short)star->w };
            MRect dstRect = {
                kStarTop[w],
                kStarLeft[w],
                (short)(kStarTop[w]  + 180),
                (short)(kStarLeft[w] + 180)
            };
            SurfaceBlendOver(drawSurface, &dstRect, star, &srcRect);
        }
    }

    // 4. Draw animated conga lines (scrolling + vertical bounce)
    float hopCycle = (t_ms / 800.0f) * 2.0f * (float)M_PI;
    for (int i = 0; i < 4; ++i)
    {
        if (!congaSurface[i]) continue;

        int W = congaSurface[i]->w;

        // Compute horizontal source offset for scrolling
        int offset = (int)(t_ms * 240 / 1000) % W;
        int srcX;
        if (kCongaScrollRight[i])
            srcX = (W - offset % W) % W;
        else
            srcX = offset % W;

        // Compute vertical bounce (hop ≤ 0 means jump upward)
        float cycle   = hopCycle + kCongaPhase[i];
        float hopRaw  = -kCongaBounce[i] * sinf(cycle);
        int   hop     = (hopRaw < 0.0f) ? (int)hopRaw : 0;
        int   baseY  = kCongaBaseY[i] + hop;

        DrawCongaLine(drawSurface, congaSurface[i], baseY, srcX);
    }

    // 5. Apply button darkening or highlight glow
    for (int w = 0; w < 3; ++w)
    {
        MRect buttonRect = {
            kWorldButtonTop[w], kWorldButtonLeft[w],
            kWorldButtonBottom[w], kWorldButtonRight[w]
        };

        bool unlocked = (g_levelBeaten >= kWorldUnlockLevel[w]);
        bool hovered  = unlocked && MPointInMRect(mouse, &buttonRect);

        // Update Highlight (0 = normal, 255 = full capsule)
        int targetHighlight = hovered ? 255 : 0;
        if      (buttonHighlight[w] < targetHighlight) { buttonHighlight[w] += 15; if (buttonHighlight[w] > targetHighlight) buttonHighlight[w] = targetHighlight; }
        else if (buttonHighlight[w] > targetHighlight) { buttonHighlight[w] -= 15; if (buttonHighlight[w] < targetHighlight) buttonHighlight[w] = targetHighlight; }

        if (buttonHighlight[w] > 0 && capsuleTexture)
        {
            SDL_Texture* prevTarget = SDL_GetRenderTarget(g_renderer);
            SDL_SetRenderTarget(g_renderer, drawSurface->texture);
            SDL_SetTextureAlphaMod(capsuleTexture, (Uint8)buttonHighlight[w]);
            SDL_Rect dstR = { kWorldButtonLeft[w], kWorldButtonTop[w],
                              kWorldButtonRight[w] - kWorldButtonLeft[w],
                              kWorldButtonBottom[w] - kWorldButtonTop[w] };
            SDL_RenderCopy(g_renderer, capsuleTexture, nullptr, &dstR);
            SDL_SetTextureAlphaMod(capsuleTexture, 255);
            SDL_SetRenderTarget(g_renderer, prevTarget);
        }
    }

    // 6. Draw text
    Gfx_AcquireSurface(drawSurface);
    SkittlesFont* buttonFont = GetFont(picSkiaBoldFont);
    SkittlesFont* labelFont  = GetFont(picSkiaFont);
    const float kLabelScale  = 0.65f;
    const int   kLineSpacing = 4;

    for (int w = 0; w < 3; ++w)
    {
        bool unlocked = (g_levelBeaten >= kWorldUnlockLevel[w]);

        MRect buttonRect = { kWorldButtonTop[w], kWorldButtonLeft[w], kWorldButtonBottom[w], kWorldButtonRight[w] };
        MRect labelRect  = { kWorldLabelTop[w],  kWorldLabelLeft[w],  kWorldLabelBottom[w],  kWorldLabelRight[w] };

        int hv        = buttonHighlight[w];
        int baseColor = unlocked ? 255 : 128;
        int btnColor  = baseColor - (baseColor * hv / 255);
        int btnShadow = (hv >= 128) ? 0 : 4;

        // Button label
        static const char* kButtonTexts[3][2] = {
            { kWorld1ButtonText,        kWorld1ButtonText        },
            { kWorld2ButtonTextLocked,  kWorld2ButtonTextUnlocked },
            { kWorld3ButtonTextLocked,  kWorld3ButtonTextUnlocked },
        };
        DrawCenteredText(buttonFont, kButtonTexts[w][unlocked ? 1 : 0], buttonRect, btnColor, btnColor, btnColor, btnShadow);

        // Description: 3 lines (unlocked) or 1 centered line (locked)
        if (!unlocked)
        {
            static const char* kLockedText[3] = { nullptr, kWorld2LabelTextLocked, kWorld3LabelTextLocked };
            if (kLockedText[w])
                DrawScaledCenteredText(labelFont, kLockedText[w], labelRect, kLabelScale, 255, 255, 255, 2);
        }
        else
        {
            static const char* const* kLines[3] = { kWorld1Lines, kWorld2Lines, kWorld3Lines };
            const char* const* lines = kLines[w];
            int fontH      = (int)(labelFont->h * kLabelScale);
            int totalH     = 3 * fontH + 2 * kLineSpacing;
            int blockTop   = labelRect.top + (labelRect.bottom - labelRect.top - totalH) / 2;
            for (int li = 0; li < 3; ++li)
            {
                MRect lr = labelRect;
                lr.top    = (short)(blockTop + li * (fontH + kLineSpacing));
                lr.bottom = (short)(lr.top + fontH);
                DrawScaledCenteredText(labelFont, lines[li], lr, kLabelScale, 255, 255, 255, 2);
            }
        }
    }
    // 7. Draw back button (touch platforms only)
    if (Platform_IsTouchDevice())
    {
        bool backHovered = MPointInMRect(mouse, &kBackButtonRect);
        int target = backHovered ? 255 : 0;
        if      (backHighlight < target) { backHighlight += 15; if (backHighlight > target) backHighlight = target; }
        else if (backHighlight > target) { backHighlight -= 15; if (backHighlight < target) backHighlight = target; }

        int color  = 255 - (255 * backHighlight / 255);
        int shadow = (backHighlight >= 128) ? 0 : 2;
        DrawScaledCenteredText(buttonFont, "\x02 Back", kBackButtonRect, 0.7f, color, color, color, shadow);
    }
    Gfx_ReleaseSurface(drawSurface);

    // 8. Draw cursor
    SurfaceBlitCursor(mouse, drawSurface);

    // 7. Present
    // 8. Present
    CC_Rect fullCCRect = { 0, 0, 1920, 1080 };
    Gfx_BlitFrontSurface(drawSurface, &fullCCRect, &fullCCRect);
}

bool UI_ShowWorldSelectScreen()
{
    // Load surfaces
    CC_RGBSurface* bgSurface = LoadPICTAsRGBSurface(picWorldSelect);
    if (!bgSurface)
    {
        Platform_FadeIn();
        return false;
    }

    CC_RGBSurface* drawSurface = Gfx_InitRGBSurface(1920, 1080);

    CC_RGBSurface* congaSurface[4] = {
        LoadPICTAsRGBSurface(picCongaBlue),
        LoadPICTAsRGBSurface(picCongaGreen),
        LoadPICTAsRGBSurface(picCongaRed),
        LoadPICTAsRGBSurface(picCongaPurple),
    };

    CC_RGBSurface* starSurface[5] = {
        LoadPICTAsRGBSurface(picStar0),
        LoadPICTAsRGBSurface(picStar1),
        LoadPICTAsRGBSurface(picStar2),
        LoadPICTAsRGBSurface(picStar3),
        LoadPICTAsRGBSurface(picStar4),
    };

    CC_RGBSurface* starburstSurface = LoadPICTAsRGBSurface(picStarburst);

    // Button state
    int buttonHighlight[3] = { 0, 0, 0 };
    int backHighlight       = 0;

    // Single capsule texture shared by all three buttons (they're all the same size).
    int btnW = kWorldButtonRight[0] - kWorldButtonLeft[0];
    int btnH = kWorldButtonBottom[0] - kWorldButtonTop[0];
    SDL_Texture* capsuleTexture = CreateCapsuleTexture(btnW, btnH);

    MPoint mouse;
    SDLU_GetMouse(&mouse);

    RenderWorldSelectFrame(bgSurface, drawSurface, starburstSurface, starSurface, congaSurface, buttonHighlight, backHighlight, capsuleTexture, mouse, Platform_GetTicks());

    Platform_FadeIn();
    SDLU_WaitForRelease();
    SDLU_StartWatchingTyping();
    DoFullRepaint = nullptr;

    while (!finished)
    {
        UpdateSound();

        uint32_t t_ms = Platform_GetTicks();
        SDLU_GetMouse(&mouse);

        RenderWorldSelectFrame(bgSurface, drawSurface, starburstSurface, starSurface, congaSurface, buttonHighlight, backHighlight, capsuleTexture, mouse, t_ms);
        SDLU_Present();

        // 8. Handle input
        if (SDLU_Button())
        {
            if (Platform_IsTouchDevice() && MPointInMRect(mouse, &kBackButtonRect))
            {
                Platform_FadeOut();
                break;
            }

            for (int w = 0; w < 3; ++w)
            {
                MRect buttonRect = {
                    kWorldButtonTop[w], kWorldButtonLeft[w],
                    kWorldButtonBottom[w], kWorldButtonRight[w]
                };

                if (g_levelBeaten >= kWorldUnlockLevel[w] && MPointInMRect(mouse, &buttonRect))
                {
                    int startLevel = w * 4 + 1;
                    PlayMono(kChime);
                    Platform_FadeOut();

                    SDLU_StopWatchingTyping();
                    Gfx_FreeSurface(bgSurface);
                    Gfx_FreeSurface(drawSurface);
                    for (int i = 0; i < 4; i++) if (congaSurface[i]) Gfx_FreeSurface(congaSurface[i]);
                    for (int i = 0; i < 5; i++) if (starSurface[i])  Gfx_FreeSurface(starSurface[i]);
                    if (starburstSurface) Gfx_FreeSurface(starburstSurface);
                    if (capsuleTexture) SDL_DestroyTexture(capsuleTexture);

                    InitGame(kPlayerControl, kAIControl, startLevel);
                    UI_ShowPuppetShowForLevel(startLevel, e_AfterPuppetShow_StartGame);
                    return true;
                }
            }
        }

        SDL_Keycode key = SDLK_UNKNOWN;
        if (SDLU_CheckSDLTyping(&key) && key == SDLK_ESCAPE)
        {
            Platform_FadeOut();
            break;
        }

        SDLU_Yield();
    }

    SDLU_StopWatchingTyping();
    Gfx_FreeSurface(bgSurface);
    Gfx_FreeSurface(drawSurface);
    for (int i = 0; i < 4; i++) if (congaSurface[i]) Gfx_FreeSurface(congaSurface[i]);
    for (int i = 0; i < 5; i++) if (starSurface[i])  Gfx_FreeSurface(starSurface[i]);
    if (starburstSurface) Gfx_FreeSurface(starburstSurface);
    if (capsuleTexture) SDL_DestroyTexture(capsuleTexture);
    return false;
}
