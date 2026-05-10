#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/mujs/mujs.h"

// --- CONFIGURACIÓN DE RUTAS ---
char base_path[512] = "./";

void set_base_path(const char *main_script_path) {
    const char *last_slash = strrchr(main_script_path, '/');
    if (!last_slash) last_slash = strrchr(main_script_path, '\\');
    if (last_slash) {
        int length = last_slash - main_script_path + 1;
        strncpy(base_path, main_script_path, (length < 512) ? length : 511);
        base_path[length] = '\0';
    }
}

// --- CACHÉ DE TEXTURAS (Para que el dibujo sea rápido) ---
typedef struct { char path[256]; Texture2D texture; } CachedTexture;
CachedTexture texCache[100];
int texCount = 0;

Texture2D GetCachedTexture(const char *path) {
    for (int i = 0; i < texCount; i++) {
        if (strcmp(texCache[i].path, path) == 0) return texCache[i].texture;
    }
    if (texCount < 100) {
        Texture2D tex = LoadTexture(path);
        if (tex.id > 0) {
            strncpy(texCache[texCount].path, path, 255);
            texCache[texCount].texture = tex;
            texCount++;
            return tex;
        }
    }
    return (Texture2D){0};
}

// --- GESTOR DE AUDIO (Tipo FMOD con Streams) ---
#define MAX_CHANNELS 32
typedef struct {
    int id;
    Music music;
    bool active;
} AudioChannel;

AudioChannel channels[MAX_CHANNELS];
int nextChannelId = 1;

int FindChannelIdx(int id) {
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (channels[i].active && channels[i].id == id) return i;
    }
    return -1;
}

// --- HELPERS PARA JS ---
Color GetColorFromJS(js_State *J, int idx) {
    Color c = WHITE;
    if (js_isobject(J, idx)) {
        js_getproperty(J, idx, "r"); c.r = (unsigned char)js_tonumber(J, -1); js_pop(J, 1);
        js_getproperty(J, idx, "g"); c.g = (unsigned char)js_tonumber(J, -1); js_pop(J, 1);
        js_getproperty(J, idx, "b"); c.b = (unsigned char)js_tonumber(J, -1); js_pop(J, 1);
        js_getproperty(J, idx, "a"); 
        c.a = js_isdefined(J, -1) ? (unsigned char)js_tonumber(J, -1) : 255;
        js_pop(J, 1);
    }
    return c;
}

// --- FUNCIONES API (C <-> JS) ---

void js_initWindow(js_State *J) {
    InitWindow((int)js_tonumber(J, 1), (int)js_tonumber(J, 2), "Funser Engine");
    SetTargetFPS(60);
    js_pushundefined(J);
}

void js_configureStage(js_State *J) {
    SetWindowTitle(js_tostring(J, 1));
    js_pushundefined(J);
}

void js_initAudio(js_State *J) {
    if (!IsAudioDeviceReady()) InitAudioDevice();
    js_pushundefined(J);
}

void js_playAudio(js_State *J) {
    const char *path = js_tostring(J, 1);
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", base_path, path);
    
    int slot = -1;
    for (int i = 0; i < MAX_CHANNELS; i++) {
        // Si el canal está inactivo o la música ya terminó de sonar
        if (!channels[i].active || (channels[i].active && !IsMusicStreamPlaying(channels[i].music))) {
            slot = i;
            break;
        }
    }

    if (slot != -1) {
        // --- PREVENCIÓN DE LEAK ---
        // Si el slot estaba activo pero el audio terminó, liberamos la memoria vieja antes de cargar la nueva
        if (channels[slot].active) {
            UnloadMusicStream(channels[slot].music);
        }

        channels[slot].music = LoadMusicStream(full_path);
        if (channels[slot].music.ctxData != NULL) {
            PlayMusicStream(channels[slot].music);
            channels[slot].active = true;
            channels[slot].id = nextChannelId++;
            js_pushnumber(J, channels[slot].id);
        } else {
            channels[slot].active = false;
            js_pushnumber(J, -1);
        }
    } else {
        js_pushnumber(J, -1);
    }
}
void js_seekAudio(js_State *J) {
    int idx = FindChannelIdx((int)js_tonumber(J, 1));
    if (idx != -1) SeekMusicStream(channels[idx].music, (float)js_tonumber(J, 2));
    js_pushundefined(J);
}

void js_stopAudio(js_State *J) {
    int idx = FindChannelIdx((int)js_tonumber(J, 1));
    if (idx != -1) {
        StopMusicStream(channels[idx].music);
        UnloadMusicStream(channels[idx].music);
        channels[idx].active = false;
    }
    js_pushundefined(J);
}

void render(js_State *J) {
    const char *type = js_tostring(J, 1);
    if (strcmp(type, "rect") == 0) {
        DrawRectangle((int)js_tonumber(J, 3), (int)js_tonumber(J, 4), (int)js_tonumber(J, 2), (int)js_tonumber(J, 5), GetColorFromJS(J, 6));
    } 
    else if (strcmp(type, "text") == 0) {
        DrawText(js_tostring(J, 2), (int)js_tonumber(J, 3), (int)js_tonumber(J, 4), (int)js_tonumber(J, 5), GetColorFromJS(J, 6));
    }
    else if (strcmp(type, "image") == 0) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s%s", base_path, js_tostring(J, 2));
        Texture2D tex = GetCachedTexture(full_path);
        if (tex.id > 0) DrawTexture(tex, (int)js_tonumber(J, 3), (int)js_tonumber(J, 4), WHITE);
    }
    js_pushundefined(J);
}

void requirejs(js_State *J) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", base_path, js_tostring(J, 1));
    if (js_dofile(J, full_path)) {
        fprintf(stderr, "Error en require: %s\n", js_tostring(J, -1));
        js_pop(J, 1);
    }
    js_pushundefined(J);
}

// --- MAIN LOOP ---
int main(int argc, char **argv) {
    if (argc < 2) { printf("Uso: engine <script.js>\n"); return 1; }
    set_base_path(argv[1]);

    js_State *J = js_newstate(NULL, NULL, JS_STRICT);

    // Registro de funciones globales
    js_newcfunction(J, js_initWindow, "initWindow", 2); js_setglobal(J, "initWindow");
    js_newcfunction(J, js_configureStage, "configureStage", 1); js_setglobal(J, "configureStage");
    js_newcfunction(J, js_initAudio, "initAudioSupport", 0); js_setglobal(J, "initAudioSupport");
    js_newcfunction(J, js_playAudio, "playAudio", 1); js_setglobal(J, "playAudio");
    js_newcfunction(J, js_stopAudio, "stopAudio", 1); js_setglobal(J, "stopAudio");
    js_newcfunction(J, js_seekAudio, "seekAudio", 2); js_setglobal(J, "seekAudio");
    js_newcfunction(J, render, "render", 6); js_setglobal(J, "render");
    js_newcfunction(J, requirejs, "require", 1); js_setglobal(J, "require");

    // Ejecutar script inicial
    if (js_dofile(J, argv[1])) {
        fprintf(stderr, "Error JS: %s\n", js_tostring(J, -1));
        return 1;
    }

    while (!WindowShouldClose()) {
        // Actualizar Audio Streams
        for (int i = 0; i < MAX_CHANNELS; i++) {
            if (channels[i].active) UpdateMusicStream(channels[i].music);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        js_getglobal(J, "onUpdate");
        if (js_iscallable(J, -1)) {
            js_pushundefined(J);
            js_pushnumber(J, GetFrameTime());
            if (js_pcall(J, 1)) {
                fprintf(stderr, "Runtime Error: %s\n", js_tostring(J, -1));
                js_pop(J, 1);
            } else js_pop(J, 1);
        } else js_pop(J, 1);

        EndDrawing();
    }

    // --- LIMPIEZA ---
    for (int i = 0; i < texCount; i++) UnloadTexture(texCache[i].texture);
    // Al salir del while(!WindowShouldClose())
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (channels[i].active) {
            StopMusicStream(channels[i].music);
            UnloadMusicStream(channels[i].music); // <--- Esto libera la RAM del buffer
        }
    }
    if (IsAudioDeviceReady()) CloseAudioDevice();
    CloseWindow();
    js_freestate(J);

    return 0;
}