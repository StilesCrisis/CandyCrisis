//
//  UI_PuppetShow.cpp
//  Candy Crisis HD — Windows/SDL puppet show implementation.
//

#include "stdafx.h"
#include "UI_PuppetShow.h"


#include "UI_PuppetShowData.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <algorithm>

#include "SDLU.h"
#include "gworld.h"
#include "blitter.h"
#include "font.h"
#include "graphics.h"
#include "main.h"
#include "gameticks.h"
#include "level.h"
#include "music.h"
#include "Platform.h"

using std::min;
using std::max;

#if !_WIN32
// strtok_s is the Windows name; POSIX uses strtok_r with identical signature.
#define strtok_s strtok_r
#endif

static inline const char* PuppetCStr(const char* s) { return s; }

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

static const int   k_fps           = 60;
static const float k_msPerTick     = 1000.0f / k_fps;
static const int   k_moveDurMs     = 500;    // puppet slide duration
static const int   k_balloonFadeMs = 500;    // balloon fade-in/out duration
static const int   k_hideBalloonMs = 300;    // hide balloon duration
static const int   k_curtainMs     = 1000;   // curtain raise duration
static const int   k_msPerChar     = 30;     // talk text: ms per character
static const int   k_talkPauseMs   = 750;    // extra pause after text finishes
static const int   k_tildePauseMs  = 30;     // pause duration for '~' character

// Speech balloon drawn at this size (in 1920×1080 space)
static const int   k_balloonW      = 630;
static const int   k_balloonH      = 365;
// Text area inside balloon (margins from balloon top-left, matching storyboard layout)
static const int   k_textMarginX   = 60;
static const int   k_textMarginY   = 55;
static const int   k_textLineSpacing = 15;
static const int   k_titleOffsetY  = 40;

// ---------------------------------------------------------------------------
// Puppet state
// ---------------------------------------------------------------------------

struct PuppetState
{
    std::vector<CC_RGBSurface*> frames;
    Uint32  frameStartMs   = 0;
    int     frameCount     = 0;

    float   x = 0, y = 0;          // current: centerX, y-offset-from-bottom
    float   startX = 0, startY = 0;
    float   targetX = 0, targetY = 0;
    Uint32  moveStartMs  = 0;
    bool    moving       = false;

    float   alpha = 1.0f;
    float   startAlpha = 1.0f;
    float   targetAlpha = 1.0f;
    Uint32  fadeStartMs  = 0;
    Uint32  fadeDurMs    = 0;
    bool    fading       = false;

    float   vx = 0, vy = 0;        // pixels per tick (60fps)
    float   gravity = 0;
    bool    launched = false;

    float   spin = 0;               // current angle (degrees, clockwise)
    float   spinPerMs = 0;          // degrees per ms
    bool    spinning = false;

    bool    flipped = false;

    void reset()
    {
        for (CC_RGBSurface* s : frames) Gfx_FreeSurface(s);
        frames.clear();
        frameCount  = 0;
        alpha       = 1.0f;
        moving = fading = launched = spinning = flipped = false;
        vx = vy = gravity = spin = spinPerMs = 0;
    }
};

// ---------------------------------------------------------------------------
// Module state
// ---------------------------------------------------------------------------

static PuppetState          s_puppets[8];
static CC_RGBSurface*       s_backdrop       = nullptr;
static CC_RGBSurface*       s_balloonSurf    = nullptr;
static SkittlesFont*        s_smallFont      = nullptr;
static SkittlesFont*        s_titleFont      = nullptr;

// Script
static const PuppetShowPattern* s_scriptPtr  = nullptr;
static const PuppetShowPattern* s_scriptEnd  = nullptr;
static Uint32                   s_scriptNextMs = 0;

// Curtain (black overlay fading out)
static float  s_curtainAlpha  = 1.0f;  // 1 = fully black
static Uint32 s_curtainStartMs = 0;
static bool   s_curtainRaising = false;

// Balloon
static float  s_balloonAlpha     = 0.0f;
static float  s_balloonFromAlpha = 0.0f;
static float  s_balloonToAlpha   = 0.0f;
static Uint32 s_balloonFadeStart = 0;
static Uint32 s_balloonFadeDur   = 0;
static bool   s_balloonFading    = false;
static bool   s_balloonFlipped   = false;
static float  s_balloonTipX      = 0;   // X of speech tip (center of balloon)
static float  s_balloonTipY      = 0;   // Y of speech tip (bottom of puppet)

// Talk text
static char   s_talkFull[1024]   = "";
static char   s_talkDisplayed[1024] = "";  // chars revealed so far
static int    s_talkLen          = 0;
static int    s_talkShown        = 0;
static Uint32 s_talkNextMs       = 0;
static bool   s_talkActive       = false;

// Opponent name
static char   s_opponentName[256] = "";

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static float EaseOut(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }

// Parse "PICT_5000/N-1A.png" → LoadPICTAsRGBSurface(5000, "/N-1A")
static CC_RGBSurface* LoadPuppetFrame(const char* path)
{
    int pictID;
    char filename[256];
    if (sscanf(path, "PICT_%d/%255s", &pictID, filename) == 2)
    {
        char* dot = strrchr(filename, '.');
        if (dot) *dot = '\0';
        char suffix[258];
        snprintf(suffix, sizeof(suffix), "/%s", filename);
        return LoadPICTAsRGBSurface(pictID, suffix);
    }
    return nullptr;
}

static void LoadPuppetFrames(PuppetState& puppet, const char* text)
{
    puppet.reset();
    if (!text || !text[0]) return;

    // Split on '|' to get multiple animation frames
    char buf[2048];
    snprintf(buf, sizeof(buf), "%s", text);
    char* ctx = nullptr;
    char* tok = strtok_s(buf, "|", &ctx);
    while (tok)
    {
        CC_RGBSurface* frame = LoadPuppetFrame(tok);
        if (frame)
            puppet.frames.push_back(frame);
        tok = strtok_s(nullptr, "|", &ctx);
    }
    puppet.frameCount  = (int)puppet.frames.size();
    puppet.frameStartMs = SDL_GetTicks();
}


static void SetPuppetPosition(PuppetState& puppet, const PuppetShowPoint& loc)
{
    puppet.x = loc.x;
    puppet.y = loc.y;
    puppet.moving = false;
}

// ---------------------------------------------------------------------------
// Script command processor — returns delay in ms before advancing to next cmd
// ---------------------------------------------------------------------------

static Uint32 ProcessCommand(const PuppetShowPattern& cmd, Uint32 now)
{
#define PUPPET_IDX(N) ((N)-1)

#define IMPL_SET(N) \
    case e_Command_SetPuppet##N: { \
        PuppetState& p = s_puppets[PUPPET_IDX(N)]; \
        SetPuppetPosition(p, cmd.loc); \
        p.alpha = 1.0f; p.fading = false; p.launched = false; p.spinning = false; p.spin = 0; \
        LoadPuppetFrames(p, cmd.text); \
        return 0; }

#define IMPL_MOVE(N) \
    case e_Command_MovePuppet##N: { \
        PuppetState& p = s_puppets[PUPPET_IDX(N)]; \
        p.startX = p.x; p.startY = p.y; \
        p.targetX = cmd.loc.x; p.targetY = cmd.loc.y; \
        p.moveStartMs = now; p.moving = true; \
        return 0; }

#define IMPL_FLIP(N) \
    case e_Command_FlipPuppet##N:   s_puppets[PUPPET_IDX(N)].flipped = true;  return 0; \
    case e_Command_UnflipPuppet##N: s_puppets[PUPPET_IDX(N)].flipped = false; return 0;

#define IMPL_FADE(N) \
    case e_Command_FadePuppet##N: { \
        PuppetState& p = s_puppets[PUPPET_IDX(N)]; \
        float targetA = cmd.loc.x; \
        float durSec  = cmd.loc.y; \
        if (durSec <= 0.0f) { p.alpha = targetA; p.fading = false; } \
        else { p.startAlpha = p.alpha; p.targetAlpha = targetA; \
               p.fadeStartMs = now; p.fadeDurMs = (Uint32)(durSec * 1000); p.fading = true; } \
        return 0; }

#define IMPL_LAUNCH(N, GRAV) \
    case e_Command_FirePuppet##N:   { PuppetState& p = s_puppets[PUPPET_IDX(N)]; p.vx = cmd.loc.x; p.vy = cmd.loc.y; p.gravity = 0.0f;   p.launched = true; p.moving = false; return 0; } \
    case e_Command_FlingPuppet##N:  { PuppetState& p = s_puppets[PUPPET_IDX(N)]; p.vx = cmd.loc.x; p.vy = cmd.loc.y; p.gravity = 0.4f;   p.launched = true; p.moving = false; return 0; } \
    case e_Command_FloatPuppet##N:  { PuppetState& p = s_puppets[PUPPET_IDX(N)]; p.vx = cmd.loc.x; p.vy = cmd.loc.y; p.gravity = -0.1f;  p.launched = true; p.moving = false; return 0; }

#define IMPL_SPIN(N) \
    case e_Command_SpinPuppet##N: { \
        float secPerSpin = cmd.loc.x; \
        PuppetState& p = s_puppets[PUPPET_IDX(N)]; \
        p.spinPerMs = (secPerSpin != 0) ? (360.0f / (secPerSpin * 1000.0f)) : 0; \
        p.spinning = true; \
        return 0; }

    switch (cmd.cmd)
    {
        case e_Command_SetOpponentName:
            if (cmd.text) snprintf(s_opponentName, sizeof(s_opponentName), "%s", PuppetCStr(cmd.text));
            return 0;

        case e_Command_RaiseCurtain:
            s_curtainRaising  = true;
            s_curtainStartMs  = now;
            return 0;

        case e_Command_ShowBalloonLeftwards:
        case e_Command_ShowBalloonRightwards:
            s_balloonFlipped  = (cmd.cmd == e_Command_ShowBalloonRightwards);
            s_balloonTipX     = cmd.loc.x;
            s_balloonTipY     = cmd.loc.y;
            s_balloonFromAlpha = s_balloonAlpha;
            s_balloonToAlpha  = 0.75f;
            s_balloonFadeStart = now;
            s_balloonFadeDur  = k_balloonFadeMs;
            s_balloonFading   = true;
            return 0;

        case e_Command_HideBalloon:
            s_talkActive      = false;
            s_talkFull[0]     = '\0';
            s_talkDisplayed[0] = '\0';
            s_balloonFromAlpha = s_balloonAlpha;
            s_balloonToAlpha  = 0.0f;
            s_balloonFadeStart = now;
            s_balloonFadeDur  = k_hideBalloonMs;
            s_balloonFading   = true;
            return k_hideBalloonMs;

        case e_Command_Wait:
            return (Uint32)(cmd.loc.x * 1000.0f);

        case e_Command_Talk1POnly:
            if (!(control[0] == kPlayerControl && control[1] == kAIControl))
                return 0;
            [[fallthrough]];

        case e_Command_Talk:
        {
            if (!cmd.text) return 0;
            snprintf(s_talkFull, sizeof(s_talkFull), "%s", PuppetCStr(cmd.text));
            s_talkDisplayed[0] = '\0';
            s_talkLen   = 0;
            s_talkShown = 0;
            
            int tildeCount = 0;
            // Count displayable characters (skip '~')
            for (int i = 0; s_talkFull[i]; ++i)
                if (s_talkFull[i] != '~') ++s_talkLen;
                else ++tildeCount;
            
            s_talkActive   = true;
            s_talkNextMs   = now;
            Uint32 textDur = (Uint32)(s_talkLen * k_msPerChar + tildeCount * k_tildePauseMs + k_talkPauseMs);
            return textDur;
        }

        IMPL_SET(1) IMPL_SET(2) IMPL_SET(3) IMPL_SET(4)
        IMPL_SET(5) IMPL_SET(6) IMPL_SET(7) IMPL_SET(8)

        IMPL_MOVE(1) IMPL_MOVE(2) IMPL_MOVE(3) IMPL_MOVE(4)
        IMPL_MOVE(5) IMPL_MOVE(6) IMPL_MOVE(7) IMPL_MOVE(8)

        IMPL_FLIP(1) IMPL_FLIP(2) IMPL_FLIP(3) IMPL_FLIP(4)
        IMPL_FLIP(5) IMPL_FLIP(6) IMPL_FLIP(7) IMPL_FLIP(8)

        IMPL_FADE(1) IMPL_FADE(2) IMPL_FADE(3) IMPL_FADE(4)
        IMPL_FADE(5) IMPL_FADE(6) IMPL_FADE(7) IMPL_FADE(8)

        IMPL_LAUNCH(1,) IMPL_LAUNCH(2,) IMPL_LAUNCH(3,) IMPL_LAUNCH(4,)
        IMPL_LAUNCH(5,) IMPL_LAUNCH(6,) IMPL_LAUNCH(7,) IMPL_LAUNCH(8,)

        IMPL_SPIN(1) IMPL_SPIN(2) IMPL_SPIN(3) IMPL_SPIN(4)
        IMPL_SPIN(5) IMPL_SPIN(6) IMPL_SPIN(7) IMPL_SPIN(8)

        default: return 0;
    }

#undef PUPPET_IDX
#undef IMPL_SET
#undef IMPL_MOVE
#undef IMPL_FLIP
#undef IMPL_FADE
#undef IMPL_LAUNCH
#undef IMPL_SPIN
}

// ---------------------------------------------------------------------------
// Per-frame updates
// ---------------------------------------------------------------------------

static void UpdatePuppets(Uint32 now, float dtMs)
{
    for (PuppetState& p : s_puppets)
    {
        // Movement lerp (ease-out over k_moveDurMs)
        if (p.moving)
        {
            float t = std::clamp((float)(now - p.moveStartMs) / k_moveDurMs, 0.f, 1.f);
            float e = EaseOut(t);
            p.x = p.startX + (p.targetX - p.startX) * e;
            p.y = p.startY + (p.targetY - p.startY) * e;
            if (t >= 1.0f) { p.moving = false; p.x = p.targetX; p.y = p.targetY; }
        }

        // Alpha lerp
        if (p.fading)
        {
            float t = (p.fadeDurMs > 0) ? std::clamp((float)(now - p.fadeStartMs) / p.fadeDurMs, 0.f, 1.f) : 1.0f;
            p.alpha = p.startAlpha + (p.targetAlpha - p.startAlpha) * t;
            if (t >= 1.0f) { p.fading = false; p.alpha = p.targetAlpha; }
        }

        // Ballistic motion (per tick at 60fps; dtMs ≈ 16.67ms → 1 tick)
        if (p.launched)
        {
            float ticks = dtMs / k_msPerTick;
            // Symplectic integration: update position using current velocity, then update velocity
            p.x  += p.vx * ticks;
            p.y  += p.vy * ticks;
            p.vy += p.gravity * ticks;
        }

        // Spin
        if (p.spinning)
            p.spin += p.spinPerMs * dtMs;

        // Multi-frame animation: 100ms per frame
        if (p.frameCount > 1)
        {
            int frameIdx = (int)((now - p.frameStartMs) / 100) % p.frameCount;
            // just read frameIdx at render time, no state needed beyond frameStartMs
            (void)frameIdx;
        }
    }
}

static void UpdateCurtain(Uint32 now)
{
    if (s_curtainRaising)
    {
        float t = std::clamp((float)(now - s_curtainStartMs) / k_curtainMs, 0.f, 1.f);
        s_curtainAlpha = 1.0f - t;
        if (t >= 1.0f) { s_curtainRaising = false; s_curtainAlpha = 0.0f; }
    }
}

static void UpdateBalloon(Uint32 now)
{
    if (s_balloonFading)
    {
        float t = (s_balloonFadeDur > 0) ? std::clamp((float)(now - s_balloonFadeStart) / s_balloonFadeDur, 0.f, 1.f) : 1.0f;
        s_balloonAlpha = s_balloonFromAlpha + (s_balloonToAlpha - s_balloonFromAlpha) * t;
        if (t >= 1.0f) { s_balloonFading = false; s_balloonAlpha = s_balloonToAlpha; }
    }
}

static void UpdateTalkText(Uint32 now)
{
    if (!s_talkActive) return;

    // Reveal characters one by one, handling '~' as pause
    while (s_talkFull[s_talkShown] && now >= s_talkNextMs)
    {
        char c = s_talkFull[s_talkShown];
        if (c == '~')
        {
            s_talkNextMs += k_tildePauseMs;
            s_talkShown++;
        }
        else
        {
            int len = (int)strlen(s_talkDisplayed);
            if (len < sizeof(s_talkDisplayed) - 1)
            {
                s_talkDisplayed[len] = c;
                s_talkDisplayed[len+1] = '\0';
            }
            s_talkNextMs += k_msPerChar;
            s_talkShown++;
        }
    }
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------

static void RenderPuppetFrame(PuppetState& p, Uint32 now)
{
    if (p.frames.empty()) return;
    if (p.alpha <= 0.001f) return;

    int frameIdx = (p.frameCount > 1) ? ((int)((now - p.frameStartMs) / 100) % p.frameCount) : 0;
    CC_RGBSurface* surf = p.frames[frameIdx];
    if (!surf || !surf->texture) return;

    int w = surf->w;
    int h = surf->h;

    // Puppet coord: (x = center-X, y = offset from bottom where 0 = hidden below screen)
    // y=0  → top edge at 1080 (fully off-screen), y=-h → bottom edge at 1080 (just visible)
    int dstX = (int)(p.x - w * 0.5f);
    int dstY = (int)(1080.0f + p.y);

    SDL_Rect dstR = { dstX, dstY, w, h };

    Uint8 a8 = (Uint8)(std::clamp(p.alpha, 0.f, 1.f) * 255);
    SDL_SetTextureAlphaMod(surf->texture, a8);
    SDL_SetTextureColorMod(surf->texture, a8, a8, a8);
    SDL_SetTextureBlendMode(surf->texture, Gfx_PremultipliedBlendMode());

    SDL_RendererFlip flip = p.flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(g_renderer, surf->texture, nullptr, &dstR, (double)p.spin, nullptr, flip);

    SDL_SetTextureAlphaMod(surf->texture, 255);
    SDL_SetTextureColorMod(surf->texture, 255, 255, 255);
}

static void RenderBalloonAndText()
{
    if (s_balloonAlpha < 0.01f) return;
    if (!s_balloonSurf || !s_balloonSurf->texture) return;

    // s_balloonTipX is the balloon CENTER X (storyboard default center = 0, offset by loc.x).
    // Y is always the fixed storyboard frame-top value (189).
    int bx = (int)(s_balloonTipX - k_balloonW * 0.5f);
    int by = 189;

    // When flipped, mirror the balloon so the pointer tip is on the right
    SDL_Rect dstR = { bx, by, k_balloonW, k_balloonH };
    Uint8 a8 = (Uint8)(std::clamp(s_balloonAlpha, 0.f, 1.f) * 255);
    SDL_SetTextureAlphaMod(s_balloonSurf->texture, a8);
    SDL_SetTextureColorMod(s_balloonSurf->texture, a8, a8, a8);
    SDL_SetTextureBlendMode(s_balloonSurf->texture, Gfx_PremultipliedBlendMode());

    SDL_RendererFlip flip = s_balloonFlipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(g_renderer, s_balloonSurf->texture, nullptr, &dstR, 0.0, nullptr, flip);

    SDL_SetTextureAlphaMod(s_balloonSurf->texture, 255);
    SDL_SetTextureColorMod(s_balloonSurf->texture, 255, 255, 255);

    // Draw text inside the balloon
    if (s_smallFont && s_talkDisplayed[0])
    {
        // Split into lines on '\n'
        char lines[4][256];
        int  lineCount = 0;
        char buf[1024];
        snprintf(buf, sizeof(buf), "%s", s_talkDisplayed);
        char* ctx  = nullptr;
        char* line = strtok_s(buf, "\n", &ctx);
        while (line && lineCount < 4)
        {
            snprintf(lines[lineCount++], 256, "%s", line);
            line = strtok_s(nullptr, "\n", &ctx);
        }

        int lineH = s_smallFont->h + k_textLineSpacing;
        int textY = by + k_textMarginY;
        int textAlpha = (int)(std::clamp(s_balloonAlpha, 0.f, 1.f) * 256);

        for (int li = 0; li < lineCount; ++li)
        {
            int ty = textY + li * lineH;
            float dx = (float)(bx + k_textMarginX);
            float sx = dx + 4, sy = ty + 4;
            BlitString(s_smallFont, lines[li], &sx, sy, 255, 255, 255, textAlpha);
            BlitString(s_smallFont, lines[li], &dx, ty, 0,   0,   0,   textAlpha);
        }
    }
}

static void RenderOpponentName()
{
    if (!s_titleFont || !s_opponentName[0]) return;

    // Dark semi-transparent bar across the top of the screen for the name.
    static const int k_nameBandH = 115;
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 160);
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect bandR = { 0, k_titleOffsetY, 1920, k_nameBandH };
    SDL_RenderFillRect(g_renderer, &bandR);
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_NONE);

    MRect nameRect = { (short)k_titleOffsetY, 0, (short)(k_titleOffsetY + k_nameBandH), 1920 };
    DrawCenteredText(s_titleFont, s_opponentName, nameRect, 255, 255, 255, 4);
}

static void RenderCurtain()
{
    if (s_curtainAlpha < 0.01f) return;
    Uint8 a8 = (Uint8)(s_curtainAlpha * 255);
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, a8);
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect fullR = { 0, 0, 1920, 1080 };
    SDL_RenderFillRect(g_renderer, &fullR);
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_NONE);
}

static void RenderFrame(Uint32 now)
{
    SDL_SetRenderTarget(g_renderer, g_frontTexture);

    // Draw backdrop
    if (s_backdrop && s_backdrop->texture)
    {
        SDL_SetTextureBlendMode(s_backdrop->texture, SDL_BLENDMODE_NONE);
        SDL_RenderCopy(g_renderer, s_backdrop->texture, nullptr, nullptr);
    }
    else
    {
        SDL_SetRenderDrawColor(g_renderer, 30, 30, 60, 255);
        SDL_RenderClear(g_renderer);
    }

    // Draw puppets (back to front by slot index)
    Gfx_AcquireSurface(g_frontSurface);
    for (int i = 0; i < 8; ++i)
        RenderPuppetFrame(s_puppets[i], now);

    // Curtain (black fade over everything else during raise)
    RenderCurtain();

    // Balloon + text
    RenderBalloonAndText();

    // Opponent name
    RenderOpponentName();

    Gfx_ReleaseSurface(g_frontSurface);
}

// ---------------------------------------------------------------------------
// Cleanup
// ---------------------------------------------------------------------------

static void PuppetShowCleanup()
{
    for (PuppetState& p : s_puppets) 
    {
        p.reset();
    }
    if (s_backdrop)
    {
        Gfx_FreeSurface(s_backdrop);
        s_backdrop    = nullptr;
    }
    if (s_balloonSurf)
    {
        Gfx_FreeSurface(s_balloonSurf);
        s_balloonSurf = nullptr;
    }
    s_opponentName[0]    = '\0';
    s_talkFull[0]        = '\0';
    s_talkDisplayed[0]   = '\0';
    s_talkActive         = false;
    s_balloonAlpha       = 0.0f;
    s_curtainAlpha       = 1.0f;
    s_curtainRaising     = false;
    s_scriptPtr          = nullptr;
    s_scriptEnd          = nullptr;
}

// ---------------------------------------------------------------------------
// Gameplay transition
// ---------------------------------------------------------------------------

// Transitions from the end of the puppet show into gameplay.
// gameplaySnap is a pre-built GPU texture of the first gameplay frame,
// captured before the puppet show started to avoid a hitch at transition time.
// If nullptr (e.g. invalid level, early exit), gameplay setup runs here instead.
// Owns the crossfade/fade-in so the caller does not need to call Platform_FadeIn.
static void TransitionToGameplay(TransitionAfterUIScreen behavior, SDL_Texture* gameplaySnap)
{
    bool isGameplay = (behavior == e_AfterPuppetShow_StartGame ||
                       behavior == e_AfterPuppetShow_Continue1PGame ||
                       behavior == e_AfterPuppetShow_ContinueSolitaireGame);

    if (!gameplaySnap)
    {
        // Fallback: pre-show setup didn't happen (invalid level or no show data).
        // Run full gameplay setup with music change now, then fade in.
        switch (behavior)
        {
            case e_AfterPuppetShow_StartGame:
            case e_AfterPuppetShow_Continue1PGame:
                BeginRound(true);
                break;
            case e_AfterPuppetShow_ContinueSolitaireGame:
                PrepareStageGraphics(character[1]);
                ChooseMusic(character[1].music);
                break;
            default:
                break;
        }
        PuppetShowCleanup();
        if (isGameplay) Platform_SetPauseButtonVisible(true);
        Platform_FadeIn();
        return;
    }

    // Gameplay frame is pre-built. Trigger music at the start of the crossfade.
    if (isGameplay) ChooseMusic(character[1].music);

    // If the show was skipped (key held on entry), RenderFrame ran once and drew
    // a black curtain over g_frontTexture. Restore from the gameplay snapshot.
    if (s_puppets[0].frames.empty())
    {
        if (gameplaySnap)
        {
            SDL_SetRenderTarget(g_renderer, g_frontTexture);
            SDL_RenderCopy(g_renderer, gameplaySnap, nullptr, nullptr);
        }
        SDL_DestroyTexture(gameplaySnap);
        PuppetShowCleanup();
        if (isGameplay) Platform_SetPauseButtonVisible(true);
        Platform_FadeIn();
        return;
    }

    // Clear elements that should not show during the transition.
    s_opponentName[0] = '\0';
    s_curtainAlpha    = 0.0f;

    // Crossfade: show-data-driven fades continue via UpdatePuppets while
    // the gameplay snapshot fades in.
    const Uint32 kDurationMs = 300;
    Uint32 start   = SDL_GetTicks();
    Uint32 prevNow = start;
    for (;;)
    {
        Uint32 now = SDL_GetTicks();
        float dtMs = std::min(100.0f, (float)(now - prevNow));
        prevNow = now;
        float t = std::min(1.0f, (float)(now - start) / (float)kDurationMs);

        // Keep physics/fades running so puppets continue naturally.
        UpdatePuppets(now, dtMs);

        // Draw puppet show elements (backdrop + puppets at current alphas).
        RenderFrame(now);

        // Overlay gameplay snapshot with increasing alpha.
        SDL_SetRenderTarget(g_renderer, g_frontTexture);
        SDL_SetTextureAlphaMod(gameplaySnap, (Uint8)(255.0f * t));
        SDL_RenderCopy(g_renderer, gameplaySnap, nullptr, nullptr);

        SDLU_Present();

        if (t >= 1.0f) break;
        SDLU_Yield();
    }

    SDL_DestroyTexture(gameplaySnap);
    PuppetShowCleanup();
    if (isGameplay) Platform_SetPauseButtonVisible(true);
}

// ---------------------------------------------------------------------------
// Public entry point
// ---------------------------------------------------------------------------

void UI_ShowPuppetShowForLevel(int level, TransitionAfterUIScreen behavior)
{
    Platform_SetPauseButtonVisible(false);

    // Validate and look up script.
    int levelIdx = level - 1;  // 0-based
    if (levelIdx < 0 || levelIdx >= k_numPuppetShowLevels)
    {
        TransitionToGameplay(behavior, nullptr);
        return;
    }

    const PuppetShowLevel& lvl = s_puppetShowLevels[levelIdx];
    if (lvl.count == 0)
    {
        TransitionToGameplay(behavior, nullptr);
        return;
    }

    s_scriptPtr = lvl.patterns;
    s_scriptEnd = lvl.patterns + lvl.count;

    // Find backdrop.
    Character* charData = GetCharacterData(level);
    if (!charData)
    {
        TransitionToGameplay(behavior, nullptr);
        return;
    }

    int levelID = 5000 + charData->picture;
    s_backdrop = nullptr;

    if (control[0] == kPlayerControl && control[1] == kAIControl)
        s_backdrop = LoadPICTAsRGBSurface(levelID, "/BackdropPuppet-1P");

    if (!s_backdrop)
        s_backdrop = LoadPICTAsRGBSurface(levelID, "/BackdropPuppet");

    if (!s_backdrop)
        s_backdrop = LoadPICTAsRGBSurface(levelID, "/Backdrop");

    s_balloonSurf = LoadPICTAsRGBSurface(picTextBalloon);
    s_smallFont   = GetFont(picFont);
    s_titleFont   = GetFont(picSkiaBoldFont);

    // Reset puppet-show state.
    for (PuppetState& p : s_puppets) p.reset();
    s_opponentName[0]  = '\0';
    s_talkActive       = false;
    s_talkFull[0]      = '\0';
    s_talkDisplayed[0] = '\0';
    s_balloonAlpha     = 0.0f;
    s_curtainAlpha     = 1.0f;
    s_curtainRaising   = false;
    s_balloonFading    = false;

    // Run gameplay setup before the show begins to avoid a hitch at transition
    // time. Heavy work (texture loading, RefreshAll) happens here while the
    // screen is still transitioning in. Music is deferred until the crossfade.
    switch (behavior)
    {
        case e_AfterPuppetShow_StartGame:
        case e_AfterPuppetShow_Continue1PGame:
            BeginRound(false);
            break;
        case e_AfterPuppetShow_ContinueSolitaireGame:
            PrepareStageGraphics(character[1]);
            break;
        default:
            break;
    }

    // Snapshot the resulting gameplay frame (GPU→GPU) before the puppet show
    // overwrites g_frontTexture with its own content.
    SDL_Texture* gameplaySnap = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888,
                                                   SDL_TEXTUREACCESS_TARGET, 1920, 1080);
    if (gameplaySnap)
    {
        SDL_SetTextureBlendMode(gameplaySnap, SDL_BLENDMODE_BLEND);
        SDL_SetRenderTarget(g_renderer, gameplaySnap);
        SDL_RenderCopy(g_renderer, g_frontTexture, nullptr, nullptr);
        SDL_SetRenderTarget(g_renderer, g_frontTexture);
    }

    // Main puppet show loop.
    {
        Uint32 now = SDL_GetTicks();
        s_scriptNextMs = now;

        FreezeGameTickCount();

        Uint32 nextFrameMs = now;
        Uint32 prevNow     = now;
        bool   done_loop   = false;
        finished = false;   // reset round-over flag so the puppet show loop can run

        while (!done_loop && !finished)
        {
            now = SDL_GetTicks();
            float dtMs = (float)(now - prevNow);
            if (dtMs > 100.0f) dtMs = 100.0f;
            prevNow = now;

            // Input: skip to end on Escape
            if (SDLU_EscapeKeyIsPressed())
            {
                s_scriptPtr = s_scriptEnd;
                done_loop   = true;
            }

            // Advance script (multiple 0-delay commands per frame)
            while (s_scriptPtr != s_scriptEnd && now >= s_scriptNextMs)
            {
                Uint32 delayMs = ProcessCommand(*s_scriptPtr, now);
                ++s_scriptPtr;
                s_scriptNextMs = now + delayMs;
                if (delayMs > 0) break;
            }
            if (s_scriptPtr == s_scriptEnd && now >= s_scriptNextMs &&
                !s_curtainRaising && !s_talkActive && s_balloonAlpha < 0.01f)
            {
                done_loop = true;
            }

            // Update
            UpdatePuppets(now, dtMs);
            UpdateCurtain(now);
            UpdateBalloon(now);
            UpdateTalkText(now);

            // Render
            RenderFrame(now);
            SDLU_Present();

            // Cap at 60fps
            nextFrameMs += 16;
            Uint32 curMs = SDL_GetTicks();
            if (nextFrameMs > curMs && (nextFrameMs - curMs) < 200)
                SDL_Delay(nextFrameMs - curMs);
            else
                nextFrameMs = SDL_GetTicks();
        }

        UnfreezeGameTickCount();
    }

    TransitionToGameplay(behavior, gameplaySnap);
}

void UI_ShowPuppetShowForSolitaireUnlock() {}
void UI_ShowPuppetShowForOnePlayerUnlock() {}
void UI_ShowPuppetShowForTwoPlayerUnlock() {}
