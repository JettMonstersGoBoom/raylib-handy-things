// NOTE NOTE 
// I think the rotation may be incorrect. the only A-B testing I've done is with the Camera object
// END NOTE

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

// JUST FOR DISPLAY, ignore
//  draw an arrow
void DrawArrow(Color color)
{
    DrawCylinderEx((Vector3) { 0, 0, 0 },
        (Vector3) {0.5, 0, 0}, .025, 0.025, 3, color);
    DrawCylinderEx((Vector3){0.5,0,0},
                   (Vector3){0.7,0,0}, .1, 0, 12, color);
}

// JUST FOR DISPLAY, ignore
//  draw 3d axis
void DrawAxis()
{   
    rlPushMatrix();
    DrawArrow(RED);
    rlPopMatrix();

    rlPushMatrix();
    rlRotatef(90, 0, 0, 1);
    DrawArrow(BLUE);
    rlPopMatrix();

    rlPushMatrix();
    rlRotatef(90,0,1,0);
    DrawArrow(GREEN);
    rlPopMatrix();
    rlSetLineWidth(1);
}

// JUST FOR DISPLAY, ignore
// Get size position for a 3d world space position (useful for texture drawing)
Vector2 GetWorldToScreenExClip(Vector3 position, Camera camera, int width, int height)
{
    // Calculate projection matrix (from perspective instead of frustum
    Matrix matProj = MatrixIdentity();

    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Calculate projection matrix from perspective
        matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)width/(double)height), rlGetCullDistanceNear(), rlGetCullDistanceFar());
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        double aspect = (double)width/(double)height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = MatrixOrtho(-right, right, -top, top, rlGetCullDistanceNear(), rlGetCullDistanceFar());
    }

    // Calculate view matrix from camera look at (and transpose it)
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
    // Convert world position vector to quaternion
    Quaternion worldPos = { position.x, position.y, position.z, 1.0f };

    // Transform world position to view
    worldPos = QuaternionTransform(worldPos, matView);

    // Transform result to projection (clip space position)
    worldPos = QuaternionTransform(worldPos, matProj);

    // Calculate normalized device coordinates (inverted y)
    Vector3 ndcPos = { worldPos.x/worldPos.w, -worldPos.y/worldPos.w, worldPos.z/worldPos.w };

    // Calculate 2d screen position vector
    Vector2 screenPosition = { (ndcPos.x + 1.0f)/2.0f*(float)width, (ndcPos.y + 1.0f)/2.0f*(float)height };
    if (worldPos.z<0)
    {
        screenPosition.x = -1;
        screenPosition.y = -1;
    }
    return screenPosition;
}

// JUST FOR DISPLAY, ignore
// draw the name on screen 
// NOTE will draw even behind the camera. which isn't correct :P 
void DrawNode2D(cgltf_node *node,Vector2 *screenpos)
{
    const char* upper = TextToUpper(node->name);
//    if ((node->camera) || (node->mesh) || (node->light))
    {
        Vector2 spos = GetWorldToScreenExClip((Vector3){node->translation[0],node->translation[1],node->translation[2]},camera,GetScreenWidth(),GetScreenHeight());
        if ((spos.x!=-1) && (spos.y!=-1))
        {
            int w = GetTextWidth(upper);
            DrawText(TextFormat("%s", upper),(int)spos.x-(w/2),(int)spos.y-16,16,WHITE);
        }
        DrawText(TextFormat("%s", upper), (int)screenpos->x, (int)screenpos->y, 16, WHITE);
        screenpos->y += 14;
    }
    screenpos->x += 16;

    for (int c=0;c<node->children_count;c++)
    {
        DrawNode2D(node->children[c],screenpos);
    }
    screenpos->x -= 16;
}

void DrawLight(cgltf_light *light)
{
    float r = light->range;
    if (r == 0) r = 1;
    DrawSphereWires((Vector3){0,0,0},r,7,7,(Color){light->color[0]*255.0f,light->color[1] * 255.0f,light->color[2] * 255.0f,255});
}

void DrawCGLTFMesh(cgltf_mesh* mesh)
{
    DrawCube((Vector3) { 0, 0, 0 }, 1, 1, 1, ORANGE);
}

//  draw something
void DrawNode(cgltf_node *node)
{
    rlPushMatrix();
    if (node->has_translation)
        rlTranslatef(node->translation[0],node->translation[1],node->translation[2]);
    if (node->has_rotation)
        rlMultMatrixf(MatrixToFloat(QuaternionToMatrix((Quaternion){node->rotation[0],node->rotation[1],node->rotation[2],node->rotation[3]})));

    //  we don't want to scale the axis 
    DrawAxis();

    if (node->has_scale)
        rlScalef(node->scale[0], node->scale[1], node->scale[2]);

    if (node->mesh)
    {
        DrawCGLTFMesh(node->mesh);
    }
    else if (node->camera)
    {
        rlPushMatrix();
        rlRotatef(45, 0, 0, 1);
        DrawCylinderWiresEx((Vector3) { 0, 0, 0 },
            (Vector3) {0, 0, -1}, .1, 1, 4, WHITE);
        rlPopMatrix();
    }
    else if (node->light)
    {
        DrawLight(node->light);
    }
    else
    {
        DrawCubeWires((Vector3) { 0, 0, 0 }, 1, 1, 1, RED);
    }
    for (int c=0;c<node->children_count;c++)
    {
        DrawNode(node->children[c]);
    }
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

    cgltf_options options = { 0 };
    options.file.read = LoadFileGLTFCallback;
    options.file.release = ReleaseFileGLTFCallback;
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse(&options, fileData, dataSize, &data);

    if (result == cgltf_result_success)
    {
        result = cgltf_load_buffers(&options, data, fileName);
        if (result != cgltf_result_success) TraceLog(LOG_INFO, "MODEL: [%s] Failed to load gltf", fileName);
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

                for (int q=0;q<data->scenes[0].nodes_count;q++)
                    DrawNode(data->scenes[0].nodes[q]);

                DrawGrid(100,20);
            EndMode3D();

            Vector2 spos;
            spos.x = 10;
            spos.y = 10;

            for (int q=0;q<data->scenes[0].nodes_count;q++)
                DrawNode2D(data->scenes[0].nodes[q],&spos);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    CloseWindow();        // Close window and OpenGL context
    return 0;
}
