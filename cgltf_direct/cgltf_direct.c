#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define CGLTF_MALLOC RL_MALLOC
#define CGLTF_FREE RL_FREE

Camera camera = { 0 };

//  assuming     #define CGLTF_IMPLEMENTATION is enabled 
//  make sure include path is set to allow access to this
#include "external/cgltf.h"        
// Load file data callback for cgltf
static cgltf_result LoadFileGLTFCallback(const struct cgltf_memory_options *memoryOptions, const struct cgltf_file_options *fileOptions, const char *path, cgltf_size *size, void **data)
{
    int filesize;
    unsigned char *filedata = LoadFileData(path, &filesize);

    if (filedata == NULL) return cgltf_result_io_error;

    *size = filesize;
    *data = filedata;

    return cgltf_result_success;
}

// Release file data callback for cgltf
static void ReleaseFileGLTFCallback(const struct cgltf_memory_options *memoryOptions, const struct cgltf_file_options *fileOptions, void *data)
{
    UnloadFileData(data);
}

//  draw an arrow
void DrawArrow(Color color)
{
    DrawLine3D((Vector3){0,0,0},(Vector3){0.5,0,0},  color);
    DrawCylinderEx((Vector3){0.5,0,0},
                   (Vector3){0.7,0,0}, .1, 0, 12, color);
}
//  draw 3d axis
void DrawAxis()
{   
    rlSetLineWidth(2);
    rlPushMatrix();
    DrawArrow(RED);
    rlPopMatrix();
    rlPushMatrix();
    rlRotatef(90,0,1,0);
    DrawArrow(GREEN);
    rlPopMatrix();
    rlPushMatrix();
    rlRotatef(90,0,0,1);
    DrawArrow(BLUE);
    rlPopMatrix();
    rlSetLineWidth(1);
}
//  draw the name on screen 
void DrawNode2D(cgltf_node *node)
{
    Vector2 spos = GetWorldToScreen((Vector3){node->translation[0],node->translation[1],node->translation[2]},camera);
    int w = GetTextWidth(node->name);
    DrawText(TextFormat("%s",node->name),(int)spos.x-(w/2),(int)spos.y-16,16,WHITE);
    for (int c=0;c<node->children_count;c++)
    {
        DrawNode2D(node->children[c]);
    }
}
//  draw something
void DrawNode(cgltf_node *node)
{
    rlPushMatrix();
    if (node->has_translation)
        rlTranslatef(node->translation[0],node->translation[1],node->translation[2]);
    if (node->has_rotation)
        rlMultMatrixf(MatrixToFloat(QuaternionToMatrix((Quaternion){node->rotation[0],node->rotation[1],node->rotation[2],node->rotation[3]})));

    DrawAxis();
    DrawCube((Vector3){0,0,0},0.1,0.1,0.1,ORANGE);
    //  then draw my children
    for (int c=0;c<node->children_count;c++)
        DrawNode(node->children[c]);
    //  all done
    rlPopMatrix();
}

int main(int argc,char **argv)
{
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "cglf");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    camera.position = (Vector3){ 10.0f, 4.0f, 10.0f }; 
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;                                
    camera.projection = CAMERA_PERSPECTIVE;             

    char *fileName = argv[1];   //  just use 1st arg 
    int dataSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &dataSize);
    //  set default options 
    cgltf_options options = { 0 };
    options.file.read = LoadFileGLTFCallback;
    options.file.release = ReleaseFileGLTFCallback;
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse(&options, fileData, dataSize, &data);

    if (result == cgltf_result_success)
    {
        if (data->file_type == cgltf_file_type_glb) TraceLog(LOG_INFO, "MODEL: [%s] Model basic data (glb) loaded successfully", fileName);
        else if (data->file_type == cgltf_file_type_gltf) TraceLog(LOG_INFO, "MODEL: [%s] Model basic data (glTF) loaded successfully", fileName);
        else TraceLog(LOG_WARNING, "MODEL: [%s] Model format not recognized", fileName);
        result = cgltf_load_buffers(&options, data, fileName);
        if (result != cgltf_result_success) TraceLog(LOG_INFO, "MODEL: [%s] Failed to load mesh/material buffers", fileName);
    }

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            UpdateCamera(&camera,CAMERA_FREE);
            SetMousePosition(GetScreenWidth()/2,GetScreenHeight()/2);
            HideCursor();
        }
        else 
            ShowCursor();


        BeginDrawing();
            ClearBackground(DARKGRAY);

            BeginMode3D(camera);
            //  draw all nodes in the scene
            for (int q=0;q<data->scenes[0].nodes_count;q++)
            {
                DrawNode(data->scenes[0].nodes[q]);
            }
            //  show a grid
            DrawGrid(100,20);
            EndMode3D();
            //  draw the scene again. this time just 2D text
            for (int q=0;q<data->scenes[0].nodes_count;q++)
            {
                DrawNode2D(data->scenes[0].nodes[q]);
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    CloseWindow();        // Close window and OpenGL context
    return 0;
}
