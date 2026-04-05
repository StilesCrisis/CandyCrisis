// prefs.c

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iomanip>

#include "main.h"
#include "prefs.h"
#include "music.h"
#include "soundfx.h"
#include "hiscore.h"
#include "keyselect.h"
#include "Globals.h"

#define PREF_NAME(x)   x
typedef const char*    PrefKeyName;

struct Preference
{
    PrefKeyName   keyName;
    void*         valuePtr;
    unsigned int  valueLength;
};

Preference prefList[] =
{
    { PREF_NAME("MusicOn"),                 &musicOn,               sizeof(musicOn) },
    { PREF_NAME("SoundOn"),                 &soundOn,               sizeof(soundOn) },
    { PREF_NAME("KeyBindings"),             playerKeys,             sizeof(playerKeys) },
    { PREF_NAME("HighScores"),              scores,                 sizeof(scores) },
    { PREF_NAME("BestCombo"),               &best,                  sizeof(best) },
    { PREF_NAME("LevelBeaten"),             &g_levelBeaten,         sizeof(g_levelBeaten) },
    { PREF_NAME("RecentHighScoreNames"),    g_recentHighScoreNames, sizeof(g_recentHighScoreNames) },
};


#ifdef __EMSCRIPTEN__

void LoadPrefs()
{
    for (Preference& pref : prefList)
    {
        int hexLen = EM_ASM_INT({
            var val = localStorage.getItem('CandyCrisis.' + UTF8ToString($0));
            return val ? val.length : 0;
        }, pref.keyName);

        if (hexLen > 0 && (unsigned)hexLen / 2 == pref.valueLength)
        {
            char* hexStr = (char*)malloc(hexLen + 1);
            EM_ASM({
                var val = localStorage.getItem('CandyCrisis.' + UTF8ToString($0));
                stringToUTF8(val, $1, $2);
            }, pref.keyName, hexStr, hexLen + 1);

            uint8_t* dst = (uint8_t*)pref.valuePtr;
            for (size_t i = 0; i < (size_t)hexLen / 2; i++)
            {
                unsigned int byte;
                sscanf(hexStr + i * 2, "%02x", &byte);
                dst[i] = (uint8_t)byte;
            }
            free(hexStr);
        }
    }
}

void SavePrefs()
{
    for (Preference& pref : prefList)
    {
        const uint8_t* src = (const uint8_t*)pref.valuePtr;
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (size_t i = 0; i < pref.valueLength; i++)
            oss << std::setw(2) << (unsigned)src[i];

        EM_ASM({
            var key = 'CandyCrisis.' + UTF8ToString($0);
            localStorage.setItem(key, UTF8ToString($1));
        }, pref.keyName, oss.str().c_str());
    }
}

#elif _WIN32

void LoadPrefs()
{
    for (Preference& pref: prefList)
    {
        HKEY hKey;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\CandyCrisis"), 0, KEY_QUERY_VALUE, &hKey))
        {
            DWORD dwType = REG_BINARY;
            DWORD dwLength = pref.valueLength;

            if ((ERROR_SUCCESS == RegQueryValueEx(hKey, pref.keyName, 0, &dwType, NULL, &dwLength))
                && dwType == REG_BINARY
                && dwLength == pref.valueLength)
            {
                RegQueryValueEx(hKey, pref.keyName, 0, &dwType, reinterpret_cast<BYTE*>(pref.valuePtr), &dwLength);
            }

            RegCloseKey(hKey);
        }
    }
}

void SavePrefs()
{
    for (Preference& pref: prefList)
    {
        HKEY hKey;
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\CandyCrisis"), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL))
        {
            RegSetValueEx(hKey, pref.keyName, 0, REG_BINARY, reinterpret_cast<BYTE*>(pref.valuePtr), pref.valueLength);
            RegCloseKey(hKey);
        }
    }
}

#else // macOS (SDL) and any other platform

// Preferences are stored as per-key binary files under SDL_GetPrefPath().
// On macOS this lands in ~/Library/Application Support/CandyCrisis/CandyCrisis/.

static void GetPrefFilePath(char* out, size_t outSize, const char* keyName)
{
    char* prefPath = SDL_GetPrefPath("CandyCrisis", "CandyCrisis");
    snprintf(out, outSize, "%s%s.bin", prefPath ? prefPath : "", keyName);
    SDL_free(prefPath);
}

void LoadPrefs()
{
    for (Preference& pref : prefList)
    {
        char path[1024];
        GetPrefFilePath(path, sizeof(path), pref.keyName);
        FILE* f = fopen(path, "rb");
        if (f)
        {
            fread(pref.valuePtr, 1, pref.valueLength, f);
            fclose(f);
        }
    }
}

void SavePrefs()
{
    for (Preference& pref : prefList)
    {
        char path[1024];
        GetPrefFilePath(path, sizeof(path), pref.keyName);
        FILE* f = fopen(path, "wb");
        if (f)
        {
            fwrite(pref.valuePtr, 1, pref.valueLength, f);
            fclose(f);
        }
    }
}

#endif
