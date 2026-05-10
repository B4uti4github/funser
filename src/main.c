#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/mujs/mujs.h"

// --- CONFIGURACIÓN Y ESTADOS ---
char base_path[512] = "./";
#define MAX_CHANNELS 64 // Subimos a 64 porque Sound es muy ligero

typedef struct { 
    int id; 
    Sound sound; 
    char path[256]; 
    bool active; 
} AudioChannel;

AudioChannel channels[MAX_CHANNELS];
int nextChannelId = 1;

typedef struct { char path[256]; Texture2D texture; } CachedTexture;
CachedTexture texCache[100];
int texCount = 0;

// --- UTILERÍAS ---
void set_base_path(const char *path) {
    const char *last = strrchr(path, '/');
    if (!last) last = strrchr(path, '\\');
    if (last) { 
        int len = last - path + 1;
        strncpy(base_path, path, len); 
        base_path[len] = '\0'; 
    }
}

Texture2D GetCachedTexture(const char *path) {
    for (int i = 0; i < texCount; i++) if (strcmp(texCache[i].path, path) == 0) return texCache[i].texture;
    Texture2D tex = LoadTexture(path);
    if (tex.id > 0 && texCount < 100) { 
        strncpy(texCache[texCount].path, path, 255); 
        texCache[texCount].texture = tex; 
        texCount++; 
    }
    return tex;
}

int FindChannelIdx(int id) {
    for (int i = 0; i < MAX_CHANNELS; i++) if (channels[i].active && channels[i].id == id) return i;
    return -1;
}

// --- API JS: INPUT ---
void js_isKeyDown(js_State *J) { js_pushboolean(J, IsKeyDown((int)js_tonumber(J, 1))); }
void js_isKeyPressed(js_State *J) { js_pushboolean(J, IsKeyPressed((int)js_tonumber(J, 1))); }
void js_getMousePos(js_State *J) {
    js_newobject(J);
    js_pushnumber(J, GetMouseX()); js_setproperty(J, -2, "x");
    js_pushnumber(J, GetMouseY()); js_setproperty(J, -2, "y");
}
void js_isMousePressed(js_State *J) {
    int btn = js_isdefined(J, 1) ? (int)js_tonumber(J, 1) : 0;
    js_pushboolean(J, IsMouseButtonPressed(btn));
}
void js_isMouseDown(js_State *J) {
    int btn = js_isdefined(J, 1) ? (int)js_tonumber(J, 1) : 0;
    js_pushboolean(J, IsMouseButtonDown(btn));
}
void js_getMouseWheel(js_State *J) { js_pushnumber(J, GetMouseWheelMove()); }

// --- API JS: AUDIO (Ahora usando Sound para evitar loops y leaks) ---

void js_playAudio(js_State *J) {
    const char *path = js_tostring(J, 1);
    char full[1024]; snprintf(full, 1024, "%s%s", base_path, path);

    // 1. Intentar buscar si el sonido ya está cargado para REUTILIZARLO
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (channels[i].active && strcmp(channels[i].path, path) == 0) {
            PlaySound(channels[i].sound);
            js_pushnumber(J, channels[i].id);
            return;
        }
    }

    // 2. Si no está cargado, buscar un slot libre
    int slot = -1;
    for (int i = 0; i < MAX_CHANNELS; i++) { if (!channels[i].active) { slot = i; break; } }

    if (slot != -1) {
        channels[slot].sound = LoadSound(full);
        if (channels[slot].sound.frameCount > 0) {
            PlaySound(channels[slot].sound);
            channels[slot].active = true;
            channels[slot].id = nextChannelId++;
            strncpy(channels[slot].path, path, 255);
            js_pushnumber(J, channels[slot].id);
        } else js_pushnumber(J, -1);
    } else js_pushnumber(J, -1);
}

void js_playAudioById(js_State *J) {
    int idx = FindChannelIdx((int)js_tonumber(J, 1));
    if (idx != -1) PlaySound(channels[idx].sound);
    js_pushundefined(J);
}

void js_setAudioPitch(js_State *J) {
    int idx = FindChannelIdx((int)js_tonumber(J, 1));
    if (idx != -1) SetSoundPitch(channels[idx].sound, (float)js_tonumber(J, 2));
    js_pushundefined(J);
}

void js_setAudioVolume(js_State *J) {
    int idx = FindChannelIdx((int)js_tonumber(J, 1));
    if (idx != -1) SetSoundVolume(channels[idx].sound, (float)js_tonumber(J, 2));
    js_pushundefined(J);
}

void js_stopAudio(js_State *J) {
    int idx = FindChannelIdx((int)js_tonumber(J, 1));
    if (idx != -1) StopSound(channels[idx].sound);
    js_pushundefined(J);
}

// Para compatibilidad con tu JoyDO.js antiguo
void js_seekAudio(js_State *J) { js_pushundefined(J); } 
void js_resumeAudio(js_State *J) { js_playAudioById(J); }

// --- API JS: RENDER & WINDOW ---
void render(js_State *J) {
    const char *type = js_tostring(J, 1);
    Color c = WHITE;
    if (js_isobject(J, 6)) {
        js_getproperty(J, 6, "r"); c.r = (unsigned char)js_tonumber(J, -1); js_pop(J, 1);
        js_getproperty(J, 6, "g"); c.g = (unsigned char)js_tonumber(J, -1); js_pop(J, 1);
        js_getproperty(J, 6, "b"); c.b = (unsigned char)js_tonumber(J, -1); js_pop(J, 1);
        js_getproperty(J, 6, "a"); c.a = js_isdefined(J, -1) ? (unsigned char)js_tonumber(J, -1) : 255; js_pop(J, 1);
    }
    if (strcmp(type, "rect") == 0) DrawRectangle((int)js_tonumber(J, 3), (int)js_tonumber(J, 4), (int)js_tonumber(J, 2), (int)js_tonumber(J, 5), c);
    else if (strcmp(type, "text") == 0) DrawText(js_tostring(J, 2), (int)js_tonumber(J, 3), (int)js_tonumber(J, 4), (int)js_tonumber(J, 5), c);
    else if (strcmp(type, "image") == 0) {
        char full[1024]; snprintf(full, 1024, "%s%s", base_path, js_tostring(J, 2));
        Texture2D t = GetCachedTexture(full);
        if (t.id > 0) DrawTexture(t, (int)js_tonumber(J, 3), (int)js_tonumber(J, 4), WHITE);
    }
    js_pushundefined(J);
}

void js_initWindow(js_State *J) { InitWindow((int)js_tonumber(J, 1), (int)js_tonumber(J, 2), "Funser"); SetTargetFPS(60); js_pushundefined(J); }
void js_initAudio(js_State *J) { if (!IsAudioDeviceReady()) InitAudioDevice(); js_pushundefined(J); }
void js_configureStage(js_State *J) { SetWindowTitle(js_tostring(J, 1)); js_pushundefined(J); }
void requirejs(js_State *J) {
    char full[1024]; snprintf(full, 1024, "%s%s", base_path, js_tostring(J, 1));
    if (js_dofile(J, full)) { fprintf(stderr, "Require Error: %s\n", js_tostring(J, -1)); js_pop(J, 1); }
    js_pushundefined(J);
}

// --- MAIN LOOP ---
int main(int argc, char **argv) {
    if (argc < 2) return 1;
    set_base_path(argv[1]);
    js_State *J = js_newstate(NULL, NULL, JS_STRICT);

    // Registro de funciones
    js_newcfunction(J, js_initWindow, "initWindow", 2); js_setglobal(J, "initWindow");
    js_newcfunction(J, js_configureStage, "configureStage", 1); js_setglobal(J, "configureStage");
    js_newcfunction(J, js_initAudio, "initAudioSupport", 0); js_setglobal(J, "initAudioSupport");
    js_newcfunction(J, render, "render", 6); js_setglobal(J, "render");
    js_newcfunction(J, js_isKeyDown, "isKeyDown", 1); js_setglobal(J, "isKeyDown");
    js_newcfunction(J, js_isKeyPressed, "isKeyPressed", 1); js_setglobal(J, "isKeyPressed");
    js_newcfunction(J, js_getMousePos, "getMousePos", 0); js_setglobal(J, "getMousePos");
    js_newcfunction(J, js_isMousePressed, "isMousePressed", 1); js_setglobal(J, "isMousePressed");
    js_newcfunction(J, js_isMouseDown, "isMouseDown", 1); js_setglobal(J, "isMouseDown");
    js_newcfunction(J, js_getMouseWheel, "getMouseWheel", 0); js_setglobal(J, "getMouseWheel");
    js_newcfunction(J, js_playAudio, "playAudio", 1); js_setglobal(J, "playAudio");
    js_newcfunction(J, js_playAudioById, "playAudioById", 1); js_setglobal(J, "playAudioById");
    js_newcfunction(J, js_seekAudio, "seekAudio", 2); js_setglobal(J, "seekAudio");
    js_newcfunction(J, js_stopAudio, "stopAudio", 1); js_setglobal(J, "stopAudio");
    js_newcfunction(J, js_setAudioPitch, "setAudioPitch", 2); js_setglobal(J, "setAudioPitch");
    js_newcfunction(J, js_setAudioVolume, "setAudioVolume", 2); js_setglobal(J, "setAudioVolume");
    js_newcfunction(J, js_resumeAudio, "resumeAudio", 1); js_setglobal(J, "resumeAudio");
    js_newcfunction(J, requirejs, "require", 1); js_setglobal(J, "require");

    if (js_dofile(J, argv[1])) { fprintf(stderr, "JS Error: %s\n", js_tostring(J, -1)); return 1; }

    while (!WindowShouldClose()) {
        // NOTA: Ya NO necesitamos UpdateMusicStream en el loop.
        // Raylib gestiona los 'Sound' automáticamente en su propio hilo.
        
        BeginDrawing();
        ClearBackground(BLACK);
        js_getglobal(J, "onUpdate");
        if (js_iscallable(J, -1)) { 
            js_pushundefined(J); 
            js_pushnumber(J, GetFrameTime()); 
            js_pcall(J, 1); 
            js_pop(J, 1); 
        }
        EndDrawing();
    }

    // Limpieza
    for (int i = 0; i < texCount; i++) UnloadTexture(texCache[i].texture);
    for (int i = 0; i < MAX_CHANNELS; i++) if (channels[i].active) UnloadSound(channels[i].sound);
    
    CloseAudioDevice();
    CloseWindow();
    js_freestate(J);
    return 0;
}