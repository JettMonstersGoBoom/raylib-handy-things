
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "config.h"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define PAK_FS_IMPL
#include "pakfs.h"

//  we use a single lightmap image
//  we can tune with these defines


#define _Q2_IMPL_
#include "quake2.h"

int screenWidth = 1280;
int screenHeight = 720;

int main(void)
{
    Model World;
    Camera camera;
    Camera lerpCamera;

    // Initialization
    //--------------------------------------------------------------------------------------
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "raylib [models] example - quake 2 header");

    ChangeDirectory("resources");

    PAKFS_Mount(".");
    World = Q2BSP_Load("maps/base1.bsp");
    PAKFS_Unmount();
    
    //  just print the entity string from the map
    //  it's up to you to parse it
    char *entString = Q2BSP_GetEntityString(&World);
    printf("%s", entString);

    // Define the camera to look into our 3d world
    camera.position = (Vector3){ 0.0f, 0.75f, 1 }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };  // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera up vector (rotation towards target)
    camera.fovy = 70.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type

    lerpCamera.position = (Vector3){ 0.0f, 0.5f, 2 }; // Camera position
    lerpCamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };  // Camera looking at point
    lerpCamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera up vector (rotation towards target)
    lerpCamera.fovy = 70.0f;                            // Camera field-of-view Y
    lerpCamera.projection = CAMERA_PERSPECTIVE;         // Camera projection type

//    SetTargetFPS(60);     // Set our game frames-per-second
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button
    {
        float dt = GetFrameTime();
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            UpdateCamera(&camera, CAMERA_FREE);
        }
        lerpCamera.position = Vector3Lerp(lerpCamera.position, camera.position, dt * 4.0f);
        lerpCamera.target = Vector3Lerp(lerpCamera.target, camera.target, dt * 4.0f);

        BeginDrawing();
        ClearBackground(RED);
        BeginMode3D(lerpCamera);
        Q2BSP_Tick(&World, &lerpCamera);
        DrawModel(World, Vector3Zero(), 1, WHITE);
        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
    }

    //  YOU MUST USE THIS
    Q2BSP_Unload(World);
    //  PLEASE 
    //  I BEG

    CloseWindow();              // Close window and OpenGL context
    return 0;
}
