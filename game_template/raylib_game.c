/*******************************************************************************************
*
*   raylib game template
*
*   <Game title>
*   <Game description>
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2021 Ramon Santamaria (@raysan5)
*
*   Modified by Jett ( MonstersGoBoom ) to remove switch statements and provide a more flexible approach
* 
********************************************************************************************/

#include "raylib.h"
#include "screens.h"    // NOTE: Declares global (extern) variables and screens functions

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Shared Variables Definition (global)
// NOTE: Those variables are shared between modules through screens.h
//----------------------------------------------------------------------------------
Font font = { 0 };
Music music = { 0 };
Sound fxCoin = { 0 };

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

// Required variables to manage screen transitions (fade-in, fade-out)
static float transAlpha = 0.0f;
static bool onTransition = false;
static bool transFadeOut = false;
static Screen *transFromScreen = NULL;
static Screen *transToScreen = NULL;

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void ChangeToScreen(Screen *screen);     // Change to screen, no transition effect

static void TransitionToScreen(Screen *screen); // Request transition to next screen
static void UpdateTransition(void);         // Update transition effect
static void DrawTransition(void);           // Draw transition effect (full-screen rectangle)

static void UpdateDrawFrame(void);          // Update and draw one frame

Screen* currentScreen;
//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib game template");

    InitAudioDevice();      // Initialize audio device

    // Load global data (assets that must be available in all screens, i.e. font)
    font = LoadFont("resources/mecha.png");
    music = LoadMusicStream("resources/ambient.ogg");
    fxCoin = LoadSound("resources/coin.wav");

    SetMusicVolume(music, 1.0f);
    PlayMusicStream(music);

    // Setup and init first screen
    currentScreen = &LogoScreen;
//    currentScreen = &TitleScreen;
    currentScreen->Init();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    // Unload current screen data before closing
    currentScreen->Unload();
    // Unload global data loaded
    UnloadFont(font);
    UnloadMusicStream(music);
    UnloadSound(fxCoin);

    CloseAudioDevice();     // Close audio context

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
// Change to next screen, no transition
static void ChangeToScreen(Screen *screen)
{
    // Unload current screen
    currentScreen->Unload();
    currentScreen = screen;
    currentScreen->Init();
}

// Request transition to next screen
static void TransitionToScreen(Screen *screen)
{
    onTransition = true;
    transFadeOut = false;
    transFromScreen = currentScreen;
    transToScreen = screen;
    transAlpha = 0.0f;
}

// Update transition effect (fade-in, fade-out)
static void UpdateTransition(void)
{
    if (!transFadeOut)
    {
        transAlpha += 0.05f;

        // NOTE: Due to float internal representation, condition jumps on 1.0f instead of 1.05f
        // For that reason we compare against 1.01f, to avoid last frame loading stop
        if (transAlpha > 1.01f)
        {
            transAlpha = 1.0f;
            transFromScreen->Unload();
            transToScreen->Init();
            currentScreen = transToScreen;
            transFadeOut = true;
        }
    }
    else  // Transition fade out logic
    {
        transAlpha -= 0.02f;

        if (transAlpha < -0.01f)
        {
            transAlpha = 0.0f;
            transFadeOut = false;
            onTransition = false;
            transFromScreen = NULL;
            transToScreen = NULL;
        }
    }
}

// Draw transition effect (full-screen rectangle)
static void DrawTransition(void)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, transAlpha));
}

// Update and draw game frame
static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    UpdateMusicStream(music);       // NOTE: Music keeps playing between screens

    currentScreen->Update();
    if (!onTransition)
    {
        Screen* tscreen = currentScreen->Finish();
        if (tscreen!=NULL)
        {
            TransitionToScreen(tscreen);
        }
    }
    else UpdateTransition();    // Update transition (fade-in, fade-out)
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        currentScreen->Draw();
        // Draw full screen rectangle in front of everything
        if (onTransition) DrawTransition();

        //DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
