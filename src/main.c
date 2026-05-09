#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/mujs/mujs.h"

// --- CACHE DE TEXTURAS ---
typedef struct {
    char path[256];
    Texture2D texture;
} CachedTexture;

CachedTexture cache[100];
int cacheCount = 0;

Texture2D GetCachedTexture(const char *path) {
    for (int i = 0; i < cacheCount; i++) {
        if (strcmp(cache[i].path, path) == 0) return cache[i].texture;
    }
    if (cacheCount < 100) {
        Texture2D tex = LoadTexture(path);
        if (tex.id > 0) {
            strcpy(cache[cacheCount].path, path);
            cache[cacheCount].texture = tex;
            cacheCount++;
            return tex;
        }
    }
    return (Texture2D){0};
}

// --- HELPERS ---
Color GetColorFromJS(js_State *J, int idx) {
    Color color = WHITE; 
    if (js_isobject(J, idx)) {
        js_getproperty(J, idx, "r"); color.r = (unsigned char)js_tonumber(J, -1); js_pop(J, 1);
        js_getproperty(J, idx, "g"); color.g = (unsigned char)js_tonumber(J, -1); js_pop(J, 1);
        js_getproperty(J, idx, "b"); color.b = (unsigned char)js_tonumber(J, -1); js_pop(J, 1);
        js_getproperty(J, idx, "a"); 
        color.a = js_isdefined(J, -1) ? (unsigned char)js_tonumber(J, -1) : 255;
        js_pop(J, 1);
    }
    return color;
}

// --- API DE CONSOLA ---
void js_print(js_State *J) {
    int n = js_gettop(J);
    for (int i = 1; i < n; i++) {
        printf("%s%s", js_tostring(J, i), i == n - 1 ? "" : " ");
    }
    printf("\n");
    js_pushundefined(J);
}

// --- API DE WINDOW / STAGE ---

// initWindow(width, height)
void js_initWindow(js_State *J) {
    int w = (int)js_tonumber(J, 1);
    int h = (int)js_tonumber(J, 2);
    
    if (!IsWindowReady()) {
        InitWindow(w, h, "Funser Runtime");
        SetTargetFPS(60);
    }
    js_pushundefined(J);
}

// configureStage(title, iconPath [opcional])
void js_configureStage(js_State *J) {
    // Verificamos si la ventana existe ANTES de hacer nada
    if (!IsWindowReady()) {
        // Si no hay ventana, ignoramos la configuración silenciosamente
        // o podrías imprimir un warning para el desarrollador:
        // printf("Warning: configureStage ignorado. Llama primero a initWindow().\n");
        js_pushundefined(J);
        return;
    }

    if (js_gettop(J) < 1) return;

    const char *title = js_tostring(J, 1);
    const char *iconPath = NULL;
    
    if (js_gettop(J) >= 2 && js_isstring(J, 2)) {
        iconPath = js_tostring(J, 2);
    }

    SetWindowTitle(title);
    
    if (iconPath && strlen(iconPath) > 0) {
        Image icon = LoadImage(iconPath);
        if (icon.data != NULL) {
            SetWindowIcon(icon);
            UnloadImage(icon);
        }
    }
    
    js_pushundefined(J);
}

// --- MOTOR DE RENDER ---
void render(js_State *J) {
    if (!IsWindowReady() || js_gettop(J) < 2) return; 
    const char *type = js_tostring(J, 1);

    if (strcmp(type, "text") == 0) {
        DrawText(js_tostring(J, 2), (int)js_tonumber(J, 3), (int)js_tonumber(J, 4), (int)js_tonumber(J, 5), GetColorFromJS(J, 6));
    } 
    else if (strcmp(type, "rect") == 0) {
        DrawRectangleV((Vector2){(float)js_tonumber(J, 3), (float)js_tonumber(J, 4)}, 
                       (Vector2){(float)js_tonumber(J, 2), (float)js_tonumber(J, 5)}, 
                       GetColorFromJS(J, 6));
    }
    else if (strcmp(type, "image") == 0) {
        Texture2D tex = GetCachedTexture(js_tostring(J, 2));
        if (tex.id > 0) {
            DrawTexture(tex, (int)js_tonumber(J, 3), (int)js_tonumber(J, 4), WHITE);
        }
    }
    js_pushundefined(J);
}

// --- MAIN ---
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: %s <script.js>\n", argv[0]);
        return 1;
    }

    js_State *J = js_newstate(NULL, NULL, JS_STRICT);

    // Registro de funciones
    js_newcfunction(J, render, "render", 6); js_setglobal(J, "render");
    js_newcfunction(J, js_initWindow, "initWindow", 2); js_setglobal(J, "initWindow");
    js_newcfunction(J, js_configureStage, "configureStage", 2); js_setglobal(J, "configureStage");
    
    // Console API
    js_newobject(J);
    js_newcfunction(J, js_print, "log", 1); js_setproperty(J, -2, "log");
    js_newcfunction(J, js_print, "error", 1); js_setproperty(J, -2, "error");
    js_setglobal(J, "console");
    js_newcfunction(J, js_print, "trace", 1); js_setglobal(J, "trace");

    // Ejecutar script
    if (js_dofile(J, argv[1])) {
        fprintf(stderr, "JS Error: %s\n", js_tostring(J, -1));
        js_freestate(J);
        return 1;
    }

    // Bucle principal
    while (IsWindowReady() && !WindowShouldClose()) {
        float dt = GetFrameTime();
        BeginDrawing();
        ClearBackground(RAYWHITE);

        js_getglobal(J, "onUpdate");
        if (js_iscallable(J, -1)) {
            js_pushundefined(J);
            js_pushnumber(J, dt);
            if (js_pcall(J, 1)) {
                fprintf(stderr, "Runtime Error: %s\n", js_tostring(J, -1));
                js_pop(J, 1);
            } else {
                js_pop(J, 1);
            }
        } else {
            js_pop(J, 1);
        }

        EndDrawing();
    }

    // Limpieza final
    for (int i = 0; i < cacheCount; i++) UnloadTexture(cache[i].texture);
    js_freestate(J);
    if (IsWindowReady()) CloseWindow();
    
    return 0;
}