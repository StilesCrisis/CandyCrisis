//
//  UI_VictoryScreen.cpp
//  Candy Crisis
//
//  World 1/2/3 victory screens — SDL port of Candy Crisis TV iOS implementation.
//

#include "stdafx.h"
#include <iterator>
#include <vector>
#include "UI_VictoryScreen.h"
#include "pause.h"
#include "font.h"
#include "MTypes.h"
#include "music.h"
#include "SDLU.h"
#include "gworld.h"
#include "blitter.h"
#include "main.h"
#include "gameticks.h"
#include "zap.h"
#include "soundfx.h"
#include "Platform.h"

// ---------------------------------------------------------------------------
// Shared helpers
// ---------------------------------------------------------------------------

// Render a blob/splat at an arbitrary scaled size centered on (cx, cy).
static void RenderScaled(CC_RGBSurface* surf, float cx, float cy, float size)
{
    if (!surf || !surf->texture || size < 1.f) return;
    int isize = (int)size;
    SDL_Rect dst = { (int)(cx - size * 0.5f), (int)(cy - size * 0.5f), isize, isize };
    SDL_RenderCopy(g_renderer, surf->texture, nullptr, &dst);
}


static inline float easeInOut(float t) { return t * t * (3.f - 2.f * t); }
static inline float easeOut(float t)   { return 1.f - (1.f - t) * (1.f - t); }

// Draw one line of text with a white (+2,+2) drop shadow and black foreground,
// centered horizontally around centerX. Matches the pattern used in all three
// World victory text overlays. Skips empty strings.
static void BlitLineShadowed(SkittlesFont* font, const char* text, int centerX, int y,
                             int alpha, float scale = 1.f)
{
    if (!*text) return;
    float x  = centerX - GetTextWidth(font, text) * scale * 0.5f;
    float sx = x + 2, sy = y + 2;
    BlitString(font, text, &sx, sy, 255, 255, 255, alpha, scale);
    BlitString(font, text, &x,  y,  0,   0,   0,   alpha, scale);
}


// Draw the "World N - Clear" banner at the top of each victory screen.
// Storyboard: UILabel at {0,67,1920,66}, Skia Bold 50pt, white text, black shadow, bg black 25%.
static void DrawWorldClearBanner(SkittlesFont* font, const char* text)
{
    if (!font) return;
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 63);  // ~25% black
    SDL_Rect bg = { 0, 67, 1920, 66 };
    SDL_RenderFillRect(g_renderer, &bg);
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_NONE);
    MRect textRect = { 67, 0, 133, 1920 };
    DrawScaledCenteredText(font, text, textRect, 0.65f, 255, 255, 255, 4);
}


// ---------------------------------------------------------------------------
// World 1 Victory Screen
// ---------------------------------------------------------------------------
//
// 10 parallax layers pan upward over 60 s (each layer taller than 1080 px).
// Text halo (937×483 storyboard size) fades in to 90% over 5 s.
// Blob rain starts at 7 s, spawning every 100 ms, using GetZapStyle for splats.
// ---------------------------------------------------------------------------

struct FallingBlob
{
    uint32_t spawn_ms;
    float    x;
    float    targetY;
    float    scale;
    int      color;      // 0..kBlobTypes-1
    int      splatFrame; // -1 = falling, 0..kZapFrames = splatting
};

void UI_ShowWorld1VictoryScreen(TransitionAfterUIScreen behavior)
{
    SDLU_ResetEscapeState();
    SDLU_LoadWorldPackage(1);

    static const int kNumLayers = 10;

    CC_RGBSurface* layers[kNumLayers] = {};
    for (int i = 0; i < kNumLayers; ++i)
        layers[i] = LoadPICTAsRGBSurface(picWorld1Layer1 + i);

    CC_RGBSurface* blobSurf[kBlobTypes]  = {};
    CC_RGBSurface* splatSurf[kBlobTypes] = {};
    for (int c = 0; c < kBlobTypes; ++c)
    {
        blobSurf[c]  = LoadPICTAsRGBSurface(picBlob0  + c);
        splatSurf[c] = LoadPICTAsRGBSurface(picSplat0 + c);
    }
    SDL_Texture* haloTex = IMG_LoadTexture(g_renderer, QuickResourceName("PICT", picTextHalo, "", "", ".png"));
    if (haloTex) SDL_SetTextureBlendMode(haloTex, SDL_BLENDMODE_BLEND);
    CC_RGBSurface* drawSurface = Gfx_InitRGBSurface(1920, 1080);

    SkittlesFont* boldFont = GetFont(picSkiaBoldFont);
    SkittlesFont* textFont = GetFont(picSkiaFont);

    // Storyboard text (World1Victory.storyboard, textHaloLabel):
    // rect (624,545,672,415), center-aligned, black text, white shadow +2+2, fade 5.5 s.
    static const char* const kTextLines[] = {
        "Endless candies continue to",
        "rain down. They could only be",
        "coming from Candy Mountain, but",
        "reaching the source requires",
        "a journey past the river and",
        "through a treacherous forest.",
        "",
        "World 2 unlocked!"
    };
    static const int kNumTextLines = (int)std::size(kTextLines);

    // Render the first frame into g_frontSurface so Platform_FadeIn reveals it.
    Gfx_AcquireSurface(drawSurface);
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);
    for (int i = 0; i < kNumLayers; ++i)
    {
        if (!layers[i]) continue;
        SDL_Rect layerDst = { 0, 0, layers[i]->w, layers[i]->h };
        SDL_RenderCopy(g_renderer, layers[i]->texture, nullptr, &layerDst);
    }
    Gfx_ReleaseSurface(drawSurface);
    CC_Rect full0 = { 0, 0, 1920, 1080 };
    Gfx_BlitFrontSurface(drawSurface, &full0, &full0);

    ChooseMusic(12);
    Platform_FadeIn();
    SDLU_WaitForRelease();
    DoFullRepaint = nullptr;

    const uint32_t kDuration     = 60000;
    const uint32_t kBlobStart    = 7000;
    const uint32_t kBlobInterval = 100;

    uint32_t startTime     = Platform_GetTicks();
    uint32_t lastBlobSpawn = 0;
    int      nextBlobColor = 0;

    std::vector<FallingBlob> blobs;
    blobs.reserve(64);

    bool done = false;
    while (!done && !finished)
    {
        uint32_t t_ms    = Platform_GetTicks() - startTime;
        float    progress = std::clamp((float)t_ms / (float)kDuration, 0.f, 1.f);

        UpdateSound();
        Gfx_AcquireSurface(drawSurface);

        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
        SDL_RenderClear(g_renderer);

        // 1. Parallax layers, back to front.
        for (int i = 0; i < kNumLayers; ++i)
        {
            if (!layers[i]) continue;
            int draw_y = -(int)(progress * (layers[i]->h - 1080));
            SDL_Rect layerDst = { 0, draw_y, layers[i]->w, layers[i]->h };
            SDL_RenderCopy(g_renderer, layers[i]->texture, nullptr, &layerDst);
        }

        // 2. Text halo: 937×483 storyboard rect, fade to 90% over 5 s.
        //    Premultiplied blend: pixel data is premultiplied by LoadPICTRGBPixels.
        if (haloTex && t_ms > 0)
        {
            float a = std::clamp((float)t_ms / 5000.f, 0.f, 1.f) * 0.9f;
            SDL_Rect haloR = { 444, 437, 1031, 631 };
            SDL_SetTextureAlphaMod(haloTex, (Uint8)(a * 255.f));
            SDL_RenderCopy(g_renderer, haloTex, nullptr, &haloR);
        }

        // 3. Storyboard text label: fades in over 5.5 s, centered in (624,545,672,415).
        if (textFont && t_ms > 0)
        {
            const float kScale = 0.75f;
            int alpha = (int)(std::clamp((float)t_ms / 5500.f, 0.f, 1.f) * 255.f);
            int lineH = (int)((textFont->h + 4) * kScale);
            for (int li = 0; li < kNumTextLines; ++li)
            {
                if (!*kTextLines[li]) continue;
                BlitLineShadowed(textFont, kTextLines[li], 624 + 672/2, 545 + li * lineH, alpha, kScale);
            }
        }

        // 3. Blob rain.
        if (t_ms >= kBlobStart)
        {
            // Spawn.
            if (t_ms - lastBlobSpawn >= kBlobInterval)
            {
                lastBlobSpawn = t_ms;
                FallingBlob b;
                b.spawn_ms   = t_ms;
                float dist   = 0.5f + 0.5f * ((float)rand() / (float)RAND_MAX);
                b.targetY    = 700.f + 400.f * dist;
                b.scale      = 0.75f * ((float)t_ms / (float)kDuration) * dist;
                b.x          = 1920.f * ((float)rand() / (float)RAND_MAX);
                b.color      = nextBlobColor++ % kBlobTypes;
                b.splatFrame = -1;
                blobs.push_back(b);
            }

            // Update & draw.
            static const int kZapIndices[6] = { -3, -2, -1, 1, 2, 3 };

            for (auto it = blobs.begin(); it != blobs.end(); )
            {
                FallingBlob& b = *it;
                float blobSize = (float)kBlobHorizSize * b.scale;

                float t_frames = (float)(t_ms - b.spawn_ms) * 60.f / 1000.f;

                if (b.splatFrame < 0)
                {
                    // iOS physics: y = y_start + scale*(6*t + t*(t-1))
                    float y = -100.f + b.scale * (6.f * t_frames + t_frames * (t_frames - 1.f));

                    if (y >= b.targetY)
                        b.splatFrame = 0;
                    else
                        RenderScaled(blobSurf[b.color], b.x, y, blobSize);
                }

                if (b.splatFrame >= 0)
                {
                    if (b.splatFrame >= kZapFrames) { it = blobs.erase(it); continue; }

                    // iOS: splatView size = raindropSize, transform.scale = raindropScale * (1 - 0.8*f/kZapFrames)
                    // → rendered chunk size = kBlobHorizSize * raindropScale² * (1 - 0.8*f/kZapFrames)
                    //   = blobSize * b.scale * (1 - 0.8*f/kZapFrames)
                    float zapScaleFactor = b.scale * (1.0f - 0.8f * b.splatFrame / (float)kZapFrames);
                    float chunkSize = blobSize * zapScaleFactor;

                    for (int si = 0; si < 6; ++si)
                    {
                        MRect chunkRect = { 0, 0, kBlobVertSize, kBlobHorizSize };
                        int   chunkType = 0;
                        GetZapStyle(0, &chunkRect, nullptr, &chunkType,
                                    kZapIndices[si], b.splatFrame, 4);

                        float cx = b.x       + b.scale * (float)chunkRect.left;
                        float cy = b.targetY + b.scale * (float)chunkRect.top;
                        RenderScaled(splatSurf[b.color], cx, cy, chunkSize);
                    }
                    ++b.splatFrame;
                }
                ++it;
            }
        }

        DrawWorldClearBanner(boldFont, "World 1 - Clear");

        Gfx_ReleaseSurface(drawSurface);

        CC_Rect full = { 0, 0, 1920, 1080 };
        Gfx_BlitFrontSurface(drawSurface, &full, &full);
        SDLU_Present();

        if (t_ms >= kDuration || SDLU_EscapeKeyIsPressed()) done = true;
        SDLU_Yield();
    }

    Platform_FadeOut();

    for (int i = 0; i < kNumLayers; ++i) if (layers[i]) Gfx_FreeSurface(layers[i]);
    for (int c = 0; c < kBlobTypes;  ++c) { if (blobSurf[c]) Gfx_FreeSurface(blobSurf[c]); if (splatSurf[c]) Gfx_FreeSurface(splatSurf[c]); }
    SDL_DestroyTexture(haloTex);
    Gfx_FreeSurface(drawSurface);

    UI_DoTransitionAfterUIScreen(behavior);
}


// ---------------------------------------------------------------------------
// World 2 / World 3 layer descriptors
// ---------------------------------------------------------------------------
//
// On Apple TV, all layers are UIImageViews in a 1920×6000 frame.  The content mode
// determines where the image sits within the frame (Top, Bottom, or ScaleToFill).
// The frame itself scrolls via additive animation from initial_y to 0 over 60 s.
//
// initial_y = 1080 - from_y_tv  (see World2VictoryViewController.mm slideImage:fromY:toY:)
// initial_x = x slide offset applied additively (negative = starts left of final pos)

// Content modes matching the UIImageView contentMode in the TV storyboard.
// All layers live in a virtual 1920×6000 frame that scrolls as a unit;
// the content mode determines where the actual image sits within that frame.
enum W2ContentMode { kAlignFill, kAlignTop, kAlignBottom };

static const int kFrameHeight = 6000;

struct W2LayerDesc
{
    int pictID;
    int initial_y;      // additive Y offset at progress=0; animated to 0
    int initial_x;      // additive X offset at progress=0; animated to 0
    int storyboard_x;   // final left-edge x of the layer on screen
    W2ContentMode mode;  // where image sits in the 6000-tall frame
};

static const W2LayerDesc kWorld2Layers[] =
{
    { picWorld2Layer1,  -1920,   0,    0,    kAlignFill   },  // 1920×2000, stretched to 6000
    { picWorld2Layer2L, -1720, -800,   0,    kAlignTop    },  // 684px wide, TopLeft
    { picWorld2Layer2R, -1720,  800, 1410,   kAlignTop    },  // 510px wide, TopRight → x=1920-510
    { picWorld2Layer3L, -2620, -400,   0,    kAlignTop    },  // 1547px wide, TopLeft
    { picWorld2Layer3R, -2620,  400,  446,   kAlignTop    },  // 1474px wide, TopRight → x=1920-1474
    { picWorld2Layer4,  -2920,   0,    0,    kAlignBottom },  // 3835px tall
    { picWorld2Layer5,  -4020,   0,    0,    kAlignTop    },  // 6000px tall (fills frame)
    { picWorld2Layer6,  -4220,   0,    0,    kAlignBottom },  // 2063px tall
    { picWorld2Layer7,  -4320,   0,    0,    kAlignBottom },  // 1420px tall
    { picWorld2Layer8,  -4520,   0,    0,    kAlignBottom },  // 1100px tall
    { picWorld2Layer9,  -4820,   0,    0,    kAlignBottom },  // 545px tall
    { picWorld2Layer10, -4920,   0,    0,    kAlignBottom },  // 535px tall
};
static const int kNumW2Layers = (int)std::size(kWorld2Layers);

static const W2LayerDesc kWorld3Layers[] =
{
    { picWorld3Layer1,  -1920,   0,    0,    kAlignFill   },
    { picWorld3Layer2L, -1720, -800,   0,    kAlignTop    },
    { picWorld3Layer2R, -1720,  800, 1410,   kAlignTop    },
    { picWorld3Layer3L, -2620, -400,   0,    kAlignTop    },
    { picWorld3Layer3R, -2620,  400,  446,   kAlignTop    },
    { picWorld3Layer4,  -2920,   0,    0,    kAlignBottom },
    { picWorld3Layer5,  -4020,   0,    0,    kAlignTop    },
    { picWorld2Layer6,  -4220,   0,    0,    kAlignBottom },  // layers 6-10 shared with World 2
    { picWorld2Layer7,  -4320,   0,    0,    kAlignBottom },
    { picWorld2Layer8,  -4520,   0,    0,    kAlignBottom },
    { picWorld2Layer9,  -4820,   0,    0,    kAlignBottom },
    { picWorld2Layer10, -4920,   0,    0,    kAlignBottom },
};
static const int kNumW3Layers = (int)std::size(kWorld3Layers);


// ---------------------------------------------------------------------------
// World 2 Victory Screen
// ---------------------------------------------------------------------------

void UI_ShowWorld2VictoryScreen(TransitionAfterUIScreen behavior)
{
    SDLU_ResetEscapeState();
    SDLU_LoadWorldPackage(2);

    CC_RGBSurface* layers[kNumW2Layers] = {};
    for (int i = 0; i < kNumW2Layers; ++i)
        layers[i] = LoadPICTAsRGBSurface(kWorld2Layers[i].pictID);

    SDL_Texture* haloTex = IMG_LoadTexture(g_renderer, QuickResourceName("PICT", picTextHalo, "", "", ".png"));
    if (haloTex) SDL_SetTextureBlendMode(haloTex, SDL_BLENDMODE_BLEND);
    CC_RGBSurface* drawSurface = Gfx_InitRGBSurface(1920, 1080);

    SkittlesFont* boldFont = GetFont(picSkiaBoldFont);
    SkittlesFont* textFont = GetFont(picSkiaFont);

    static const char* const kTextLines[] = {
        "The perils of the forest are",
        "defeated at last. Candy Mountain",
        "looms overhead. Spooky monsters",
        "block the path. Can you outwit the",
        "dragon at its peak and stop the",
        "rain of candy?",
        "",
        "World 3 unlocked!"
    };
    static const int kNumTextLines = (int)std::size(kTextLines);

    const uint32_t kDuration = 60000;
    const CC_Rect  full      = { 0, 0, 1920, 1080 };

    auto renderFrame = [&](uint32_t t_ms)
    {
        float progress = std::clamp((float)t_ms / (float)kDuration, 0.f, 1.f);

        Gfx_AcquireSurface(drawSurface);
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
        SDL_RenderClear(g_renderer);

        for (int i = 0; i < kNumW2Layers; ++i)
        {
            if (!layers[i]) continue;
            const W2LayerDesc& d = kWorld2Layers[i];
            int frame_y = (int)(d.initial_y * (1.f - progress));
            int draw_x  = d.storyboard_x + (int)(d.initial_x * (1.f - progress));
            int draw_y, draw_h, draw_w;
            switch (d.mode) {
                case kAlignFill:   draw_y = frame_y; draw_h = kFrameHeight; draw_w = 1920; break;
                case kAlignBottom: draw_y = frame_y + kFrameHeight - layers[i]->h; draw_h = layers[i]->h; draw_w = layers[i]->w; break;
                default:           draw_y = frame_y; draw_h = layers[i]->h; draw_w = layers[i]->w; break;
            }
            SDL_Rect layerDst = { draw_x, draw_y, draw_w, draw_h };
            SDL_RenderCopy(g_renderer, layers[i]->texture, nullptr, &layerDst);
        }

        // Text halo: fades in at 10 s over 10 s, max 75% alpha.
        if (haloTex && t_ms > 10000)
        {
            float a = std::clamp((float)(t_ms - 10000) / 10000.f, 0.f, 1.f) * 0.75f;
            SDL_Rect haloDst = { 444, 437, 1031, 631 };
            SDL_SetTextureAlphaMod(haloTex, (Uint8)(a * 255.f));
            SDL_RenderCopy(g_renderer, haloTex, nullptr, &haloDst);
        }

        // Text label: fades in at 10 s over 10 s, centered in (624,545,672,415).
        if (textFont && t_ms > 10000)
        {
            const float kScale = 0.75f;
            int alpha = (int)(std::clamp((float)(t_ms - 10000) / 10000.f, 0.f, 1.f) * 255.f);
            int lineH = (int)((textFont->h + 4) * kScale);
            for (int li = 0; li < kNumTextLines; ++li)
            {
                if (!*kTextLines[li]) continue;
                BlitLineShadowed(textFont, kTextLines[li], 624 + 672/2, 545 + li * lineH, alpha, kScale);
            }
        }

        DrawWorldClearBanner(boldFont, "World 2 - Clear");

        Gfx_ReleaseSurface(drawSurface);
        Gfx_BlitFrontSurface(drawSurface, &full, &full);
    };

    // Pre-render first frame so Platform_FadeIn reveals victory screen, not stale game graphics.
    renderFrame(0);
    ChooseMusic(12);
    Platform_FadeIn();
    SDLU_WaitForRelease();
    DoFullRepaint = nullptr;

    uint32_t startTime = Platform_GetTicks();
    bool done = false;

    while (!done && !finished)
    {
        uint32_t t_ms = Platform_GetTicks() - startTime;
        UpdateSound();
        renderFrame(t_ms);
        SDLU_Present();
        if (t_ms >= kDuration || SDLU_EscapeKeyIsPressed()) done = true;
        SDLU_Yield();
    }

    Platform_FadeOut();

    for (int i = 0; i < kNumW2Layers; ++i) if (layers[i]) Gfx_FreeSurface(layers[i]);
    SDL_DestroyTexture(haloTex);
    Gfx_FreeSurface(drawSurface);

    UI_DoTransitionAfterUIScreen(behavior);
}


// ---------------------------------------------------------------------------
// World 3 Victory Screen
// ---------------------------------------------------------------------------
//
// Phase 1 (0–11 s):  Layers static at offset (0,0).
// Phase 2 (11–56 s): Layers slide FROM (0,0) TO (initial_x, initial_y) — upward/outward
//                    — easeInOut over 45 s.  (initial_y is negative → layers rise up.)
// Composite overlay: World-2-Composite.png fades out 1→0 over 10 s.
// Marshmallow rain:  22 instances across 4 depth layers with exact storyboard positions.
// Text halo+label:   Fades in at 20 s over 10 s.  Storyboard rect: 937×483 at (492,370).
// ---------------------------------------------------------------------------

struct MarshmallowInstance
{
    CC_RGBSurface*   surf;
    int              sx;       // storyboard x (left edge; often negative for wide strips)
    int              sy;       // storyboard y (negative = starts above screen)
    float            fall_speed; // px per ms
    float            amplitude;  // x sine wave amplitude (px)
    float            freq;       // x sine wave frequency (rad/ms)
    uint32_t         start_ms;
    SDL_RendererFlip flip;
};

void UI_ShowWorld3VictoryScreen(TransitionAfterUIScreen behavior)
{
    SDLU_ResetEscapeState();
    SDLU_LoadWorldPackage(3);

    CC_RGBSurface* layers[kNumW3Layers] = {};
    for (int i = 0; i < kNumW3Layers; ++i)
        layers[i] = LoadPICTAsRGBSurface(kWorld3Layers[i].pictID);

    CC_RGBSurface* compositeSurf = LoadPICTAsRGBSurface(picWorld3Composite);
    SDL_Texture* haloTex = IMG_LoadTexture(g_renderer, QuickResourceName("PICT", picTextHalo, "", "", ".png"));
    if (haloTex) SDL_SetTextureBlendMode(haloTex, SDL_BLENDMODE_BLEND);

    CC_RGBSurface* mFartherSurf  = LoadPICTAsRGBSurface(picMarshmallowsFarther);
    CC_RGBSurface* mFarLightSurf = LoadPICTAsRGBSurface(picMarshmallowsFarLight);
    CC_RGBSurface* mFarSurf      = LoadPICTAsRGBSurface(picMarshmallowsFar);
    CC_RGBSurface* mNearSurf     = LoadPICTAsRGBSurface(picMarshmallowsNear);

    CC_RGBSurface* drawSurface   = Gfx_InitRGBSurface(1920, 1080);

    SkittlesFont* boldFont = GetFont(picSkiaBoldFont);
    SkittlesFont* textFont = GetFont(picSkiaFont);

    static const char* const kTextLines[] = {
        "The dragon has been defeated!",
        "Marshmallows twinkle in the sky",
        "one last time. At long last,",
        "life in Candy Village can",
        "return to normal.",
        "",
        "Thank you for playing!",
    };
    static const int kNumTextLines = (int)std::size(kTextLines);

    // Wave frequencies derived from TV code frame-rate math:
    //   Farther: waveCycle += π/125 per frame @60fps → period = 250/60 s = 4167 ms
    //   Far:     waveCycle += π/96  per frame @60fps → period = 192/60 s = 3200 ms
    //   Near:    waveCycle += π/75  per frame @60fps → period = 150/60 s = 2500 ms
    const float k2Pi = 2.f * (float)M_PI;
    const float kFartherFreq = k2Pi / 4167.f;   // rad/ms
    const float kFarFreq     = k2Pi / 3200.f;
    const float kNearFreq    = k2Pi / 2500.f;

    // Fall speeds: distance / duration  (px/ms)
    //   Farther/FarLight: 2600px / 60000ms
    //   Far:              4000px / 60000ms
    //   Near:             7000px / 60000ms
    const float kFartherSpd = 2600.f / 60000.f;
    const float kFarSpd     = 4000.f / 60000.f;
    const float kNearSpd    = 7000.f / 60000.f;

    // Flip macro
    const SDL_RendererFlip kHV = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);

    // 22 marshmallow instances with exact storyboard positions and flip transforms.
    // Positions from World3Victory.storyboard.  X is negative because the 2739-px-wide
    // strip extends well to the right; the actual visible portion is [abs(sx), abs(sx)+1920].
    // Y positions are exact multiples of image height, staggered so they tile seamlessly.
    const MarshmallowInstance kMarshmallows[] =
    {
        // --- Farther (5 instances, PICT_638, 2739×621, delay 7 s) ---
        { mFartherSurf, -410, -621,   kFartherSpd,  6.f, kFartherFreq, 7000, SDL_FLIP_NONE       },
        { mFartherSurf, -200,-1242,   kFartherSpd,  6.f, kFartherFreq, 7000, SDL_FLIP_HORIZONTAL },
        { mFartherSurf, -500,-1863,   kFartherSpd,  6.f, kFartherFreq, 7000, SDL_FLIP_VERTICAL   },
        { mFartherSurf, -100,-2484,   kFartherSpd,  6.f, kFartherFreq, 7000, kHV                 },
        { mFartherSurf, -300,-3105,   kFartherSpd,  6.f, kFartherFreq, 7000, SDL_FLIP_NONE       },

        // --- Far group (9 instances, delay 5 s; instance 0 uses FarLight image) ---
        { mFarLightSurf,-400, -621,   kFarSpd,     12.f, kFarFreq,     5000, SDL_FLIP_NONE       },
        { mFarSurf,     -200,-1242,   kFarSpd,     12.f, kFarFreq,     5000, SDL_FLIP_NONE       },
        { mFarSurf,     -600,-1863,   kFarSpd,     12.f, kFarFreq,     5000, SDL_FLIP_HORIZONTAL },
        { mFarSurf,     -275,-2484,   kFarSpd,     12.f, kFarFreq,     5000, SDL_FLIP_VERTICAL   },
        { mFarSurf,     -500,-3105,   kFarSpd,     12.f, kFarFreq,     5000, kHV                 },
        { mFarSurf,        0,-3726,   kFarSpd,     12.f, kFarFreq,     5000, SDL_FLIP_NONE       },
        { mFarSurf,     -100,-4347,   kFarSpd,     12.f, kFarFreq,     5000, SDL_FLIP_VERTICAL   },
        { mFarSurf,     -135,-4968,   kFarSpd,     12.f, kFarFreq,     5000, SDL_FLIP_HORIZONTAL },
        { mFarSurf,     -440,-5589,   kFarSpd,     12.f, kFarFreq,     5000, SDL_FLIP_NONE       },

        // --- Near (7 instances, PICT_641, 2800×991, delay 9 s) ---
        { mNearSurf,    -440,  -991,  kNearSpd,    18.f, kNearFreq,    9000, SDL_FLIP_NONE       },
        { mNearSurf,    -240, -1982,  kNearSpd,    18.f, kNearFreq,    9000, SDL_FLIP_HORIZONTAL },
        { mNearSurf,    -100, -2973,  kNearSpd,    18.f, kNearFreq,    9000, SDL_FLIP_NONE       },
        { mNearSurf,       0, -3964,  kNearSpd,    18.f, kNearFreq,    9000, SDL_FLIP_VERTICAL   },
        { mNearSurf,    -500, -4955,  kNearSpd,    18.f, kNearFreq,    9000, kHV                 },
        { mNearSurf,    -150, -5946,  kNearSpd,    18.f, kNearFreq,    9000, SDL_FLIP_NONE       },
        { mNearSurf,    -440, -6937,  kNearSpd,    18.f, kNearFreq,    9000, SDL_FLIP_HORIZONTAL },
    };
    const int kNumMarshmallows = (int)std::size(kMarshmallows);

    const uint32_t kDuration = 40000;
    const uint32_t kPhase1   = 11000;
    const uint32_t kPhase2   = 45000;
    const CC_Rect  full      = { 0, 0, 1920, 1080 };

    auto renderFrame = [&](uint32_t t_ms)
    {
        Gfx_AcquireSurface(drawSurface);
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
        SDL_RenderClear(g_renderer);

        // Phase 2 (starts at kPhase1): slide layers from (0,0) toward (initial_x, initial_y).
        float p2_linear = std::clamp((float)(t_ms > kPhase1 ? t_ms - kPhase1 : 0u) / (float)kPhase2, 0.f, 1.f);
        float p2 = easeInOut(p2_linear);

        // TV Z-order (bottom→top): Layer1, Layer3L/3R, Layer4-10, marshmallows, Layer2L/2R, Composite.
        // Layer2L (index 1) and Layer2R (index 2) must draw above marshmallows but below the composite.
        auto renderLayer = [&](int i)
        {
            if (!layers[i]) return;
            const W2LayerDesc& d = kWorld3Layers[i];
            float off_y = d.initial_y * p2;
            float off_x = d.initial_x * p2;
            int frame_y = (int)off_y;
            int draw_x  = d.storyboard_x + (int)off_x;
            int draw_y, draw_h;
            switch (d.mode) {
                case kAlignFill:   draw_y = frame_y; draw_h = kFrameHeight; break;
                case kAlignBottom: draw_y = frame_y + kFrameHeight - layers[i]->h; draw_h = layers[i]->h; break;
                default:           draw_y = frame_y; draw_h = layers[i]->h; break;
            }
            // kAlignFill layers (e.g. Layer1) may be narrower than 1920 and must be stretched.
            int draw_w = (d.mode == kAlignFill) ? 1920 : layers[i]->w;
            SDL_Rect layerDst = { draw_x, draw_y, draw_w, draw_h };
            SDL_RenderCopy(g_renderer, layers[i]->texture, nullptr, &layerDst);
        };

        // Pass 1: Layer1 and Layer3L/3R through Layer10 (skip Layer2L/2R).
        renderLayer(0);
        for (int i = 3; i < kNumW3Layers; ++i) renderLayer(i);

        // Pass 2: Marshmallow rain (between background layers and Layer2L/2R in Z-order).
        for (int m = 0; m < kNumMarshmallows; ++m)
        {
            const MarshmallowInstance& mi = kMarshmallows[m];
            if (!mi.surf || t_ms < mi.start_ms) continue;

            float elapsed = (float)(t_ms - mi.start_ms);
            float draw_y  = (float)mi.sy + elapsed * mi.fall_speed;
            float wave_x  = mi.amplitude * sinf(elapsed * mi.freq);
            int   draw_x  = mi.sx + (int)wave_x;

            SDL_Rect marshmallowDst = { draw_x, (int)draw_y, mi.surf->w, mi.surf->h };
            SDL_RenderCopyEx(g_renderer, mi.surf->texture, nullptr, &marshmallowDst, 0.0, nullptr, mi.flip);
        }

        // Pass 3: Layer2L and Layer2R (above marshmallows, below composite).
        renderLayer(1);
        renderLayer(2);

        // Pass 4: Composite overlay fades out 1→0 over 10 s with easeOut (fast start, slow finish).
        // The texture is premultiplied, so color mod and alpha mod must be set to the same value;
        // otherwise at a=0 the blend degenerates to additive (srcRGB_premult + dstRGB), leaving
        // the brown mountain as a static ghost over the scrolling purple layers.
        if (compositeSurf)
        {
            float t_norm = std::clamp((float)t_ms / 10000.f, 0.f, 1.f);
            float a = 1.f - easeOut(t_norm);
            if (a > 0.f)
            {
                Uint8 ab = (Uint8)(a * 255.f);
                SDL_Rect compDst = { 0, 0, compositeSurf->w, compositeSurf->h };
                SDL_SetTextureColorMod(compositeSurf->texture, ab, ab, ab);
                SDL_SetTextureAlphaMod(compositeSurf->texture, ab);
                SDL_RenderCopy(g_renderer, compositeSurf->texture, nullptr, &compDst);
                SDL_SetTextureColorMod(compositeSurf->texture, 255, 255, 255);
                SDL_SetTextureAlphaMod(compositeSurf->texture, 255);
            }
        }

        // Text halo + label: fade in at 20 s over 10 s.
        if (t_ms > 20000)
        {
            float fadeT = std::clamp((float)(t_ms - 20000) / 10000.f, 0.f, 1.f);

            if (haloTex)
            {
                SDL_Rect haloDst = { 445, 446, 1031, 556 };
                SDL_SetTextureAlphaMod(haloTex, (Uint8)(fadeT * 0.75f * 255.f));
                SDL_RenderCopy(g_renderer, haloTex, nullptr, &haloDst);
            }

            if (textFont)
            {
                const float kScale = 0.75f;
                int alpha = (int)(fadeT * 255.f);
                int lineH = (int)((textFont->h + 4) * kScale);
                for (int li = 0; li < kNumTextLines; ++li)
                {
                    BlitLineShadowed(textFont, kTextLines[li], 624 + 672/2, 545 + li * lineH, alpha, kScale);
                }
            }
        }

        DrawWorldClearBanner(boldFont, "World 3 - Clear");

        Gfx_ReleaseSurface(drawSurface);
        Gfx_BlitFrontSurface(drawSurface, &full, &full);
    };

    // Pre-render first frame so Platform_FadeIn reveals the victory screen, not stale game graphics.
    renderFrame(0);
    ChooseMusic(14);
    Platform_FadeIn();
    SDLU_WaitForRelease();
    DoFullRepaint = nullptr;

    uint32_t startTime = Platform_GetTicks();
    bool done = false;

    while (!done && !finished)
    {
        uint32_t t_ms = Platform_GetTicks() - startTime;
        UpdateSound();
        renderFrame(t_ms);
        SDLU_Present();
        if (t_ms >= kDuration || SDLU_EscapeKeyIsPressed()) done = true;
        SDLU_Yield();
    }

    Platform_FadeOut();

    for (int i = 0; i < kNumW3Layers; ++i) if (layers[i]) Gfx_FreeSurface(layers[i]);
    if (compositeSurf)  Gfx_FreeSurface(compositeSurf);
    SDL_DestroyTexture(haloTex);
    if (mFartherSurf)   Gfx_FreeSurface(mFartherSurf);
    if (mFarLightSurf)  Gfx_FreeSurface(mFarLightSurf);
    if (mFarSurf)       Gfx_FreeSurface(mFarSurf);
    if (mNearSurf)      Gfx_FreeSurface(mNearSurf);
    Gfx_FreeSurface(drawSurface);

    UI_ShowCreditsScreen(behavior);
}


// ---------------------------------------------------------------------------
// Credits Screen
// ---------------------------------------------------------------------------
//
// 5 campfire background layers with additive parallax animation over 30 s.
// Credits panel (605×1080) fades in at t=3 s over 9 s with easeOut.
// At t=30 s or Escape: advances to PostCredits screen.
//
// Campfire layer animations (CoreAnimation additive, from TV CreditsViewController.mm):
//   layer1 — static (1920×1080)
//   layer2 — Y: +400→-250 (easeInOut, 30 s)
//   layer3 — X: -500→+500 (easeInOut, 30 s)
//   layer4 — Y: -500→-200 (easeInOut, 30 s)
//   layer5 — Y: +400→+300 (easeInOut, 30 s)
// ---------------------------------------------------------------------------

void UI_ShowCreditsScreen(TransitionAfterUIScreen behavior)
{
    static const int kNumCampfire = 5;
    CC_RGBSurface* campfire[kNumCampfire] = {};
    for (int i = 0; i < kNumCampfire; ++i)
        campfire[i] = LoadPICTAsRGBSurface(picCreditsLayer1 + i);

    CC_RGBSurface* logoSurf    = LoadPICTAsRGBSurface(picLogo);
    CC_RGBSurface* drawSurface = Gfx_InitRGBSurface(1920, 1080);
    SkittlesFont*  titleFont    = GetFont(picSkiaBoldFont);   // Skia-Regular_Bold
    SkittlesFont*  hiScoreFont  = GetFont(picSkiaFont);     // Skia-Regular

    auto renderFrame = [&](uint32_t t_ms) {
        // Layer slide progress is linear (CoreAnimation default timing, no timingFunction set).
        float    progress  = std::clamp((float)t_ms / (float)30000, 0.f, 1.f);

        Gfx_AcquireSurface(drawSurface);
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
        SDL_RenderClear(g_renderer);

        // Campfire Layer 1: static full-screen (1920×1080)
        if (campfire[0]) {
            SDL_Rect dst = { 0, 0, 1920, 1080 };
            SDL_RenderCopy(g_renderer, campfire[0]->texture, nullptr, &dst);
        }
        // Layer 2: Y additive +400→-250 (slides up, 1920×1599)
        if (campfire[1]) {
            SDL_Rect dst = { 0, (int)(400.f - 650.f * progress), campfire[1]->w, campfire[1]->h };
            SDL_RenderCopy(g_renderer, campfire[1]->texture, nullptr, &dst);
        }
        // Layer 3: X additive -500→+500 (slides right, 1920×1599)
        if (campfire[2]) {
            SDL_Rect dst = { (int)(-500.f + 1000.f * progress), 0, campfire[2]->w, campfire[2]->h };
            SDL_RenderCopy(g_renderer, campfire[2]->texture, nullptr, &dst);
        }
        // Layer 4: Y additive -500→-200 (slides down from above, 1920×1599)
        if (campfire[3]) {
            SDL_Rect dst = { 0, (int)(-500.f + 300.f * progress), campfire[3]->w, campfire[3]->h };
            SDL_RenderCopy(g_renderer, campfire[3]->texture, nullptr, &dst);
        }
        // Layer 5: Y additive +400→+300 (slides slightly up, 1920×1599)
        if (campfire[4]) {
            SDL_Rect dst = { 0, (int)(400.f - 100.f * progress), campfire[4]->w, campfire[4]->h };
            SDL_RenderCopy(g_renderer, campfire[4]->texture, nullptr, &dst);
        }

        // Credits panel: fades in at t=3 s over 9 s (easeOut)
        float panel_alpha = 0.f;
        if (t_ms >= 3000) {
            float ft = std::clamp((float)(t_ms - 3000) / 9000.f, 0.f, 1.f);
            panel_alpha = easeOut(ft);
        }

        if (panel_alpha > 0.f)
        {
            const int kPanelW = 605;
            const int ialpha  = (int)(panel_alpha * 255.f);

            // Semi-transparent black panel (storyboard: inner label alpha=0.8, creditsView fades)
            SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, (Uint8)(panel_alpha * 229.f)); // ~90% black
            SDL_Rect panelRect = { 0, 0, kPanelW, 1080 };
            SDL_RenderFillRect(g_renderer, &panelRect);
            SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_NONE);

            // Candy Crisis logo at storyboard position (25, 41, 554×250)
            if (logoSurf && logoSurf->texture) {
                Uint8 ab = (Uint8)ialpha;
                SDL_SetTextureColorMod(logoSurf->texture, ab, ab, ab);
                SDL_SetTextureAlphaMod(logoSurf->texture, ab);
                SDL_Rect logoDst = { 25, 41, 554, 250 };
                SDL_RenderCopy(g_renderer, logoSurf->texture, nullptr, &logoDst);
                SDL_SetTextureColorMod(logoSurf->texture, 255, 255, 255);
                SDL_SetTextureAlphaMod(logoSurf->texture, 255);
            }

            // Credit text labels
            if (titleFont && hiScoreFont)
            {
                // Scales: picSkiaBoldFont 43pt, picSkiaFont 41pt/24pt/17pt.
                const float kHdrScale  = 0.65f;
                const float kNameScale = 0.65f;
                const float kSmall     = 0.36f;  // slightly under 24pt to match TV visual appearance
                // Line advance: storyboard music box is 203px / 7 lines = exactly 29px.
                const int   kSmallLineH = 29;
                // All single-line storyboard labels are 66px tall; use that for vertical centering.
                // The two-line "Project Lead"/"Software Development" block occupies 99px total;
                // split it as top 49px + bottom 50px (one line each, centered within its half).
                auto lineRect = [kPanelW](int y, int h) -> MRect { return {(short)y, 0, (short)(y+h), (short)kPanelW}; };

                DrawScaledCenteredText(titleFont,   "Project Lead",         lineRect(299, 49), kHdrScale,  253, 209, 146, 0, ialpha); // warm peach
                DrawScaledCenteredText(titleFont,   "Software Development", lineRect(348, 50), kHdrScale,  249, 168,  55, 0, ialpha); // golden yellow
                DrawScaledCenteredText(hiScoreFont, "John Stiles",          lineRect(390, 66), kNameScale, 255, 255, 255, 0, ialpha); // white
                DrawScaledCenteredText(titleFont,   "Creative Director",    lineRect(479, 66), kHdrScale,  250, 196, 198, 0, ialpha); // soft pink
                DrawScaledCenteredText(hiScoreFont, "Kat Bentley",          lineRect(532, 66), kNameScale, 255, 255, 255, 0, ialpha); // white
                DrawScaledCenteredText(titleFont,   "Music",                lineRect(616, 66), kHdrScale,  246, 167, 186, 0, ialpha); // rose pink

                // y=671: Music credits — two 276-wide columns, one name per line.
                // Left column: right-aligned within x=0..276.  Right column: left-aligned at x=321.
                static const char* kMusicLeft[] = {
                    "fmod", "Lizardking", "Armadon, Explizit",
                    "Leviathan, Nemesis", "Jester, Pygmy", "Siren", "Sirrus",
                };
                static const char* kMusicRight[] = {
                    "Scaven, FC", "Spring", "Timewalker",
                    "Jason, Silents", "Chromatic Dragon", "Ng Pei Sin",
                };
                // UIKit vertically centers text within the label frame.
                // Left (7 lines × 29px = 203px): offset = 0. Right (6 lines × 29px = 174px): offset = (203-174)/2 = 14px.
                const int kMusicBoxH   = 203;
                const int kLeftStartY  = 671 + (kMusicBoxH - (int)std::size(kMusicLeft)  * kSmallLineH) / 2;
                const int kRightStartY = 671 + (kMusicBoxH - (int)std::size(kMusicRight) * kSmallLineH) / 2;
                for (int li = 0; li < (int)std::size(kMusicLeft); ++li) {
                    float rx = 276.f - GetTextWidth(hiScoreFont, kMusicLeft[li]) * kSmall;
                    BlitString(hiScoreFont, kMusicLeft[li],  &rx, kLeftStartY  + li * kSmallLineH, 255, 255, 255, ialpha, kSmall);
                }
                for (int li = 0; li < (int)std::size(kMusicRight); ++li) {
                    float fx = 321.f;
                    BlitString(hiScoreFont, kMusicRight[li], &fx, kRightStartY + li * kSmallLineH, 255, 255, 255, ialpha, kSmall);
                }

                // Tiny label at (308, 1007): 17pt, line advance = 17pt × 1.2.
                const float kTiny      = kSmall * 17.f / 24.f;
                const int   kTinyLineH = (int)(hiScoreFont->h * kTiny * 1.2f);
                float tx = 308.f;
                BlitString(hiScoreFont, "8155",  &tx, 1007,              255, 255, 255, ialpha, kTiny);
                tx = 308.f;
                BlitString(hiScoreFont, "18455", &tx, 1007 + kTinyLineH, 255, 255, 255, ialpha, kTiny);

                DrawScaledCenteredText(titleFont,   "Special Thanks", lineRect(882,  66), kHdrScale,  240, 132, 171, 0, ialpha); // magenta pink
                DrawScaledCenteredText(hiScoreFont, "Mandi Stiles",   lineRect(944,  66), kNameScale, 255, 255, 255, 0, ialpha); // white
                // "Al" storyboard label is 525px wide (not full 605), so center within that.
                DrawScaledCenteredText(hiScoreFont, "Al", {1001, 0, 1067, 525}, kNameScale, 255, 255, 255, 0, ialpha); // white
            }
        }

        Gfx_ReleaseSurface(drawSurface);
        CC_Rect full = { 0, 0, 1920, 1080 };
        Gfx_BlitFrontSurface(drawSurface, &full, &full);
    };

    renderFrame(0);
    Platform_FadeIn();
    SDLU_WaitForRelease();
    DoFullRepaint = nullptr;

    const uint32_t kDuration = 30000;
    uint32_t startTime = Platform_GetTicks();
    bool done = false;

    while (!done && !finished)
    {
        uint32_t t_ms = Platform_GetTicks() - startTime;
        UpdateSound();
        renderFrame(t_ms);
        SDLU_Present();

        if (t_ms >= kDuration || SDLU_EscapeKeyIsPressed()) done = true;
        SDLU_Yield();
    }

    Platform_FadeOut();

    for (int i = 0; i < kNumCampfire; ++i) if (campfire[i]) Gfx_FreeSurface(campfire[i]);
    if (logoSurf) Gfx_FreeSurface(logoSurf);
    Gfx_FreeSurface(drawSurface);

    UI_ShowPostCreditsScreen(behavior);
}


// ---------------------------------------------------------------------------
// PostCredits Screen
// ---------------------------------------------------------------------------
//
// 3 Marshmallow-Roast layers (all 1920×1080).
// Layer 1: always visible.
// Layer 2: shows at t=1.5 s.
// Layer 3: blink animation — show@1.6s, hide@2.5s, show@2.6s, hide@3.0s, show@3.15s.
// At t=4.0 s or Escape: calls UI_DoTransitionAfterUIScreen.
// (From PostCreditsViewController.mm)
// ---------------------------------------------------------------------------

void UI_ShowPostCreditsScreen(TransitionAfterUIScreen behavior)
{
    CC_RGBSurface* roast[3] = {};
    for (int i = 0; i < 3; ++i)
        roast[i] = LoadPICTAsRGBSurface(picMarshmallowRoast1 + i);

    CC_RGBSurface* drawSurface = Gfx_InitRGBSurface(1920, 1080);

    auto renderFrame = [&](uint32_t t_ms) {
        Gfx_AcquireSurface(drawSurface);
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
        SDL_RenderClear(g_renderer);

        // Layer 1: always visible (static background)
        if (roast[0] && roast[0]->texture) {
            SDL_Rect dst = { 0, 0, 1920, 1080 };
            SDL_RenderCopy(g_renderer, roast[0]->texture, nullptr, &dst);
        }
        // Layer 2: appears at t=1.5 s
        if (roast[1] && roast[1]->texture && t_ms >= 1500) {
            SDL_Rect dst = { 0, 0, 1920, 1080 };
            SDL_RenderCopy(g_renderer, roast[1]->texture, nullptr, &dst);
        }
        // Layer 3: blink schedule (from PostCreditsViewController.mm)
        // show@1600, hide@2500, show@2600, hide@3000, show@3150
        bool layer3Visible = false;
        if      (t_ms >= 3150) layer3Visible = true;
        else if (t_ms >= 3000) layer3Visible = false;
        else if (t_ms >= 2600) layer3Visible = true;
        else if (t_ms >= 2500) layer3Visible = false;
        else if (t_ms >= 1600) layer3Visible = true;
        if (roast[2] && roast[2]->texture && layer3Visible) {
            SDL_Rect dst = { 0, 0, 1920, 1080 };
            SDL_RenderCopy(g_renderer, roast[2]->texture, nullptr, &dst);
        }

        Gfx_ReleaseSurface(drawSurface);
        CC_Rect full = { 0, 0, 1920, 1080 };
        Gfx_BlitFrontSurface(drawSurface, &full, &full);
    };

    renderFrame(0);
    Platform_FadeIn();
    SDLU_WaitForRelease();
    DoFullRepaint = nullptr;

    const uint32_t kDuration = 4000;
    uint32_t startTime = Platform_GetTicks();
    bool done = false;

    while (!done && !finished)
    {
        uint32_t t_ms = Platform_GetTicks() - startTime;
        UpdateSound();
        renderFrame(t_ms);
        SDLU_Present();

        if (t_ms >= kDuration || SDLU_EscapeKeyIsPressed()) done = true;
        SDLU_Yield();
    }

    Platform_FadeOut();

    for (int i = 0; i < 3; ++i) if (roast[i]) Gfx_FreeSurface(roast[i]);
    Gfx_FreeSurface(drawSurface);

    UI_DoTransitionAfterUIScreen(behavior);
}
