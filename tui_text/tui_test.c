#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "raylib.h"
#define TUI_IMPLEMENTATION
#include "tui.h"

const char *menu_bar[]=
{
    "File:Open:Save:Open PRJ:Save PRJ:Exit",
    "Edit:Undo:Redo:Cut:Copy:Paste",
    "Demo:Stuff:Things",
    NULL
};

void tui_test()
{
    float t = fmod(GetTime()*5,128);
//    clear text screen
    tui_cls();
//    set the print attribute 
    tui_set_attribute(0x03);
//    print X,Y, variadic text    
    tui_print(0,2,"FPS :%d",GetFPS());
    tui_set_attribute(0x13);
    tui_print(0,3,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    tui_set_attribute(0x23);
    tui_print(0,4,"abcdefghijklmnopqrstuvwxyz");

    tui_set_attribute(0x06);
//    draw a plain box
    tui_box(12,12,12,12);
//    
    tui_print(13,13,"Hello There");
//    draw the color box at 32 X 12 Y
    tui_colorbox(32,12);
    
    tui_set_attribute(0x73);
//    scale mouse to fit display ( we actually scale the 8x8 display UP to 16 ) so divide by 16 here
    int x = GetMouseX()/16;
    int y = GetMouseY()/16;
    tui_box_decorated(x,y,16,16);
//    draw bar
    tui_set_attribute(0x67);
//    X,Y,value,MAX_VALUE    
    tui_bar(0,24,(int)t&127,128);
//    
    static int choice = 0;
    tui_selection_group(16,12,"Hello:This:Is:Options\0",&choice);
    int menu_choice = tui_menu(menu_bar,9,1);
    if (menu_choice!=0)
        printf("%x\n",menu_choice);
    tui_present(16,WHITE);
}

int main(void)
{
    //--------------------------------------------------------------------------------------
    int screenWidth = 640;
    int screenHeight = 480;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "raylib tui template");

    int width = GetMonitorWidth(0)/16;
    int height = GetMonitorHeight(0)/16;


    Camera3D    camera;
    camera.position = (Vector3){ 100.0f, 40.0f, 100.0f }; 
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         
    camera.fovy = 45.0f;                                
    camera.projection = CAMERA_PERSPECTIVE;             

    tui_create(width,height);
    tui_set_font(LoadTexture("tui_font.png"));
    tui_set_palette(LoadTexture("color-graphics-adapter-1x.png"));
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
        ClearBackground(BLACK);

        UpdateCamera(&camera,CAMERA_ORBITAL);
        BeginMode3D(camera);
        DrawGrid(10,100);
        EndMode3D();

        tui_test();
        EndDrawing();
    }

    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}
