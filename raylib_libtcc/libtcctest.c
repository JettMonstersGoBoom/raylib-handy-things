#include <stdio.h>

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "libtcc.h"

#define C_FILE "test.c"

bool TCC_error = false;

static void TCC_compile_err(void *d, const char *s)
{
    TCC_error = true;
	printf("error n%ld (%s)", (long)d, s);
}

TCCState *TCC_Prep()
{
    TCCState *state;
    TCC_error = false;
    state = tcc_new();
    tcc_set_output_type(state, TCC_OUTPUT_MEMORY);

    int n;
    tcc_set_error_func(state, &n, TCC_compile_err);

    tcc_add_include_path(state,"include");
    tcc_add_library_path(state,".");
    tcc_add_file(state,"raylib.dll");
    return state;
}

bool TCC_AddFile(TCCState *state,const char *filename)
{
    if (state==NULL) return;
    tcc_add_file(state,filename);
    if (tcc_relocate(state, TCC_RELOCATE_AUTO) < 0)
    {
        printf("failed to reallocate\n");
        return false;
    }
    return true;
}

void TCC_FuncFailed(void *idk){}

void *TCC_GetFunc(TCCState *state,char *funcname)
{
    if (state!=NULL)
    {
        void *func = tcc_get_symbol(state,funcname);
        if (func!=NULL)
        {
            return func;
        }
    }    
    printf("Function \"%s\" NOT FOUND\n",funcname);
    return TCC_FuncFailed;
}

void TCC_Unload(TCCState *state)
{
bool (*func)(void);
    func = NULL;
    if (state!=NULL)
        tcc_delete(state);
}
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    long        lastModTime = 0;   //  we force this to zero to trick the hot-reload to force reload
    TCCState    *state = NULL;

    const int screenWidth = 640;
    const int screenHeight = 480;
   
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        long curModTime = GetFileModTime(C_FILE);

        if (curModTime>lastModTime)
        {
            lastModTime = curModTime;
            //  you have to unload state 
            TCC_Unload(state);

            state = TCC_Prep();
            TCC_AddFile(state,C_FILE);
        }

        BeginDrawing();

            DrawText("Congrats! Main Application Text!", 190, 200, 20, LIGHTGRAY);  // Example

            int (*func)(void *) = TCC_GetFunc(state,"raytest");
            func(NULL);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    TCC_Unload(state);

    CloseWindow();        // Close window and OpenGL context
    return 0;
}
