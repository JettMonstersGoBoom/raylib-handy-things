#include <stdio.h>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#define RAYGUI_MENUBAR_IMPLEMENTATION
#include "raygui_menubar.h"

enum 
{
    MENU_OPEN = 0x1000,
    MENU_SAVE,
    MENU_OPEN_PROJECT,
    MENU_SAVE_PROJECT,
    MENU_UNDO,
    MENU_REDO,
    MENU_CUT,
    MENU_COPY,
    MENU_PASTE,
    MENU_EXIT
};
//  first item is the menu header, and it's uID should be 0 ( closed )
MenuBarEntry file_menu[] = 
{
    {"File",0},
    {"#05# Open",MENU_OPEN},
    {"#06# Save",MENU_SAVE},
    {"#01# Open Project",MENU_OPEN_PROJECT},
    {"#02# Save Project",MENU_SAVE_PROJECT},
    {"#113# Exit",MENU_EXIT},
    {NULL},
};

MenuBarEntry edit_menu[] = 
{
    {"Edit",0},
    {"#56#Undo",MENU_UNDO},
    {"#57#Redo",MENU_REDO},
    {"#17#Cut",MENU_CUT},
    {"#16#Copy",MENU_COPY},
    {"#18#Paste",MENU_PASTE},
    {NULL},
};

MenuBarEntry demo_menu[] = 
{
    {"Demo",0},
    {"#56#Stuff",MENU_UNDO},
    {"#57#Things",MENU_REDO},
    {NULL},
};

MenuBarEntry *MainMenu[]=
{
    file_menu,
    edit_menu,
    demo_menu,
    NULL
};

bool exitWindow = false;
void HandleMenuBar();
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1280;
    int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "raylib menu template");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    exitWindow = false;
    //  load font
    const int textSize = 24;
    GuiSetStyle(DEFAULT, TEXT_SIZE, textSize);

    // Main game loop
    while (!exitWindow)    // Detect window close button or ESC key
    {
        ClearBackground(BLACK);
        exitWindow = WindowShouldClose();
        BeginDrawing();
        HandleMenuBar();
        EndDrawing();
    }
 //   UnloadFont(fontTtf);
    CloseWindow();        // Close window and OpenGL context
    return 0;
}

void HandleMenuBar()
{
    int bwidth =  (GuiGetStyle(DEFAULT, TEXT_SIZE) * 8) + 4;
    int bheight = GuiGetStyle(DEFAULT, TEXT_SIZE) + 4;

    int ret = GuiShowMenu(MainMenu,bwidth,bheight);
    if (ret!=0)
    {
        switch(ret)
        {
            case MENU_EXIT:
            {
                exitWindow=true;
                break;
            }
            case MENU_OPEN:
            {
                printf("Open File");
                break;
            }
        }
    }
}