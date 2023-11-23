#include <stdio.h>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define RCAMERA_IMPLEMENTATION
#include "rcamera.h"

#include "plugins.h"

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1280;
    int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "raylib editor template");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    
    PluginsInit("plugins");

    Camera camera = { 0 };
    camera.position = (Vector3){ 10.0f, -5.0f, 10.0f }; 
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     
    camera.up = (Vector3){ 0.0f, -1.0f, 0.0f };         
    camera.fovy = 45.0f;                                
    camera.projection = CAMERA_PERSPECTIVE;             

    Camera2D cam2d = {0};
    cam2d.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    cam2d.rotation = 0.0f;
    cam2d.zoom = 1.0f;

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        PluginsCheckForUpdates();

        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode3D(camera);
            PluginsUpdate3D(&camera);
            EndMode3D();

            BeginMode2D(cam2d);
            PluginsUpdate2D(&cam2d);
            EndMode2D();

        DrawFPS(10,10);
        EndDrawing();

        if (IsKeyPressed(KEY_R))
            PluginsForceReLoad();
    }
    PluginsUnLoad();
    CloseWindow();        // Close window and OpenGL context
    return 0;
}

