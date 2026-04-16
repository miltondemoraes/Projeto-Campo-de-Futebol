#include "audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include "third_party/miniaudio.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

static ma_engine g_engine;
static int g_engine_ok;
static ma_sound g_crowd_sound;
static int g_crowd_playing = 0;

static int file_exists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

static void resolve_sound_path(char *out, size_t outSize, const char *filename) {
    static char buf[6][768];
    int n = 0;

    snprintf(buf[n], sizeof(buf[n]), "assets/sounds/%s", filename);
    n++;
    snprintf(buf[n], sizeof(buf[n]), "../assets/sounds/%s", filename);
    n++;

#ifdef _WIN32
    {
        char exeDir[MAX_PATH];
        if (GetModuleFileNameA(NULL, exeDir, MAX_PATH) > 0) {
            char *slash = strrchr(exeDir, '\\');
            if (slash) {
                slash[1] = '\0';
                snprintf(buf[n], sizeof(buf[n]), "%s..\\assets\\sounds\\%s", exeDir, filename);
                n++;
                snprintf(buf[n], sizeof(buf[n]), "%sassets\\sounds\\%s", exeDir, filename);
                n++;
            }
        }
    }
#endif

    for (int i = 0; i < n; i++) {
        if (file_exists(buf[i])) {
            strncpy(out, buf[i], outSize - 1);
            out[outSize - 1] = '\0';
            return;
        }
    }
    out[0] = '\0';
}

void audioInit(void) {
    ma_result r = ma_engine_init(NULL, &g_engine);
    g_engine_ok = (r == MA_SUCCESS);
    if (!g_engine_ok) {
        fprintf(stderr, "audio: ma_engine_init falhou (%d)\n", (int)r);
    }
}

void audioShutdown(void) {
    if (g_crowd_playing) {
        ma_sound_uninit(&g_crowd_sound);
        g_crowd_playing = 0;
    }
    if (g_engine_ok) {
        ma_engine_uninit(&g_engine);
        g_engine_ok = 0;
    }
}

void audioPlayKick(void) {
    char path[768];

    if (!g_engine_ok) {
        return;
    }
    resolve_sound_path(path, sizeof(path), "kick.wav");
    if (path[0] == '\0') {
        return;
    }
    ma_engine_play_sound(&g_engine, path, NULL);
}

void audioPlayGoal(void) {
    char path[768];

    if (!g_engine_ok) {
        return;
    }
    resolve_sound_path(path, sizeof(path), "goal.wav");
    if (path[0] == '\0') {
        return;
    }
    ma_engine_play_sound(&g_engine, path, NULL);
}

void audioPlayCrowd(void) {
    char path[768];
    ma_result r;

    if (!g_engine_ok || g_crowd_playing) {
        return;
    }
    resolve_sound_path(path, sizeof(path), "crowd.wav");
    if (path[0] == '\0') {
        fprintf(stderr, "audio: crowd.wav nao encontrado\n");
        return;
    }
    r = ma_sound_init_from_file(&g_engine, path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &g_crowd_sound);
    if (r != MA_SUCCESS) {
        fprintf(stderr, "audio: ma_sound_init_from_file falhou para crowd.wav (%d)\n", (int)r);
        return;
    }
    ma_sound_set_looping(&g_crowd_sound, 1);
    ma_sound_set_volume(&g_crowd_sound, 0.15f);
    ma_sound_start(&g_crowd_sound);
    g_crowd_playing = 1;
}

void audioStopCrowd(void) {
    if (g_crowd_playing) {
        ma_sound_stop(&g_crowd_sound);
        ma_sound_uninit(&g_crowd_sound);
        g_crowd_playing = 0;
    }
}
