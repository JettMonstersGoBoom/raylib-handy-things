#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

typedef struct 
{
    unsigned int key;
    bool (*action)(void *idk);
} _input_action_t_;

bool Action_A(void *idk)
{
    printf("action A\n");
    return false;
}

bool Action_B(void *idk)
{
    printf(".");
    return false;
}

bool Action_A_pressed(void *idk)
{
    printf("action A pressed\n");
    return false;
}

bool Action_B_pressed(void *idk)
{
    printf("action B pressed\t");
    return false;
}

bool Action_B_released(void *idk)
{
    printf("\taction B released\n");
    return false;
}

bool Action_C(void *idk)
{
    printf("action C\n");
    return true;
}

#define GAMEPAD0            0b00100000000000000000000000000000
#define GAMEPAD1            0b01000000000000000000000000000000
#define GAMEPAD2            0b01100000000000000000000000000000
#define GAMEPAD3            0b10000000000000000000000000000000
#define GAMEPAD_ANY         0b11100000000000000000000000000000  //  any of the 4 gamepads 

#define GAMEPAD_PRESSED     0b00010000000000000000000000000000
#define GAMEPAD_RELEASED    0b00001000000000000000000000000000

#define KEY_PRESSED         0b00010000000000000000000000000000
#define KEY_RELEASED        0b00001000000000000000000000000000
#define KEY_SHIFT           0b00000100000000000000000000000000
#define KEY_CONTROL         0b00000010000000000000000000000000
#define KEY_ALT             0b00000001000000000000000000000000


//  define some example actions
_input_action_t_ input_actions[]=
{
    //  hold CTRL + SHIFT + ALT + C for "Action_C"
    { KEY_C | KEY_CONTROL | KEY_SHIFT | KEY_ALT | KEY_PRESSED , Action_C},

    //  hold A
    { KEY_A , Action_A },
    //  pressed A
    { KEY_A | KEY_PRESSED , Action_A_pressed},
    //  B handler
    { KEY_B | KEY_PRESSED , Action_B_pressed},
    { KEY_B | KEY_RELEASED , Action_B_released},
    { KEY_B , Action_B },

    { GAMEPAD0      | GAMEPAD_BUTTON_LEFT_FACE_UP | GAMEPAD_PRESSED         , Action_A_pressed},
    { GAMEPAD0      | GAMEPAD_BUTTON_LEFT_FACE_DOWN                         , Action_B},
    { GAMEPAD_ANY   | GAMEPAD_BUTTON_RIGHT_FACE_LEFT | GAMEPAD_RELEASED     , Action_B_released},

    { 0 , NULL}
};
//  we pass a pointer to inputs, so we could change them for different parts of the game. 
//  or redefine them 

void CheckKeys(_input_action_t_ *inputs)
{
    for (_input_action_t_ *action=inputs;action->action!=NULL;action++)
    {
        //  assume we're not taking an action 
        bool takeAction = false;
        if ((action->key&GAMEPAD_ANY)!=0)
        {
            int gid = (action->key>>29);
            for (int q=0;q<4;q++)
            {
                //  check for bits to see if we want this pad 
                if ((gid&(1<<q))!=0)
                {
                    if (IsGamepadAvailable(q))
                    {
                        if ((action->key&GAMEPAD_PRESSED)!=0)                  //  check if we've asked for HELD state
                        {
                            if (IsGamepadButtonPressed(q,action->key & 0xffff))
                            {
                                printf("GID %d,%x PRESSED?\n",gid,action->key&0xffff);
                                takeAction = true;
                            }
                        }
                        else if ((action->key&GAMEPAD_RELEASED)!=0)                  //  check if we've asked for RELEASE state
                        {
                            if (IsGamepadButtonReleased(q,action->key & 0xffff))
                            {
                                printf("GID %d,%x RELEASED?\n",gid,action->key&0xffff);
                                takeAction = true;
                            }
                        }
                        else 
                        {
                            if (IsGamepadButtonDown(q,action->key & 0xffff))
                            {
                                printf("GID %d,%x DOWN?\n",gid,action->key&0xffff);
                                takeAction = true;
                            }
                        }
                    }
                }
            }
            //  gamepad
        }
        else  
        {
            //  keyboard

            if ((action->key&KEY_PRESSED)!=0)                  //  check if we've asked for HELD state
            {
                if (IsKeyPressed(action->key&0x7ff))
                {
                    takeAction = true;
                }
            }
            else if ((action->key&KEY_RELEASED)!=0)         //  check if we've asked for RELEASED state
            {
                if (IsKeyReleased(action->key&0x7ff))
                {
                    takeAction = true;
                }
            }
            else                                            //  we just want the key 
            {
                if (IsKeyDown(action->key&0x7ff))
                {
                    takeAction = true;
                }
            }

            //  if we wanted SHIFT + key 
            if ((action->key&KEY_SHIFT)!=0)
            {
                if ((IsKeyDown(KEY_LEFT_SHIFT)==0) && (IsKeyDown(KEY_RIGHT_SHIFT)==0))  
                    takeAction = false;                     //  no shift was pressed so cancel any action
            }
            //  if we wanted CONTROL + key 
            if ((action->key&KEY_CONTROL)!=0)
            {
                if ((IsKeyDown(KEY_LEFT_CONTROL)==0) && (IsKeyDown(KEY_RIGHT_CONTROL)==0))
                    takeAction = false;                     //  no CONTROL was pressed so cancel any action
            }
            //  if we wanted ALT + key 
            if ((action->key&KEY_ALT)!=0)
            {
                if ((IsKeyDown(KEY_LEFT_ALT)==0) && (IsKeyDown(KEY_RIGHT_ALT)==0))
                    takeAction = false;                     //  no ALT was pressed so cancel any action
            }
        }
        //  ok we can take this action 
        //  doesn't pass anything yet but should
        if (takeAction==true)
        {
            if (action->action!=NULL)
                action->action(NULL);
        }
    }
}

int main(int argc,char **argv)
{
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "inputs");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
            ClearBackground(DARKGRAY);
            CheckKeys(input_actions);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    CloseWindow();        // Close window and OpenGL context
    return 0;
}
