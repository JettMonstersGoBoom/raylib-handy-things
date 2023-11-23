
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
    sPos = (Vector3){0,-4,0};
    sDir = (Vector3){0.0,0.01,0};
}

EXPORT void OnUpdate3D(Camera3D *camera)
{
    sPos = Vector3Add(sPos,sDir);
    sDir = Vector3Add(sDir,(Vector3){0,0.001f,0});
    if (sPos.y>0.1f)
    {
        sDir.y = -sDir.y;
    }
    DrawSphere(sPos,1,GREEN);
}

EXPORT void OnUpdate2D(Camera2D *camera)
{
}

