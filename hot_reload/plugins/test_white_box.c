
#include <stdio.h>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else 
#define EXPORT 
#endif

Vector3 sPos;
Vector3 sDir;
BoundingBox bBox;

EXPORT void OnUnload()
{
    TraceLog(LOG_INFO,"we're un-loaded %s",__FILE__);
}

EXPORT void OnLoad()
{
    TraceLog(LOG_INFO,"we're loaded %s",__FILE__);
    bBox.min = (Vector3){-2,-1,-1};  
    bBox.max = (Vector3){2,1,1};  
    sPos = (Vector3){-5,0,0};
    sDir = (Vector3){0.01,0,0};
}

EXPORT void OnUpdate3D(Camera3D *camera)
{
    Color hitter = RED;
    sPos = Vector3Add(sPos,sDir);

    if (CheckCollisionBoxSphere(bBox,sPos,1)==true)
    {
        hitter = YELLOW;
    }

    DrawSphere(sPos,1,WHITE);
    DrawBoundingBox(bBox,hitter);

    DrawGrid(10, 25.0f);         // Draw a grid
}

EXPORT void OnUpdate2D(Camera2D *camera)
{
}

