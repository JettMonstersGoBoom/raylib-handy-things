//
// using the zip/src library from here https://github.com/kuba--/zip
// the other systems i found had a LOT of external dependancies, this is 1 .C file and 2 headers
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include "zip.h"
//    container with the zip pointers and a count 
typedef struct _archive_t_
{
    int count;
    struct zip_t **zips;
} _archive_t_; 

_archive_t_ archives;
//    mount all zips/pk3's in the path ( use extension ) 
void zipsMount(const char *path,const char *ext)
{
    FilePathList list = LoadDirectoryFilesEx(path,ext,false);
    if (list.count==0)
        return;

    archives.count = list.count;
    archives.zips = (struct zip_t*)RL_CALLOC(list.count,sizeof(void*));
    for (int q=0;q<list.count;q++)
        archives.zips[q] = zip_open(list.paths[q], 0, 'r');
}
//    checks for file availability in any archives
//    returns -1 if not found
int _zipFindFile(const char *fname)
{
    for (int q=0;q<archives.count;q++)
    {
        int ret = zip_entry_open(archives.zips[q],fname);
        if (ret==0)
        {
            printf("found %d for %s\n",q,fname);
            return q;
        }
    }
    printf("file %s not found\n",fname);
    return -1;
}

//    load data returning a buffer ( or NULL ) and filling size 
//
void *zipLoadFileData(const char *fname,int *size)
{
    for (int q=0;q<archives.count;q++)
    {
        int ret = zip_entry_open(archives.zips[q],fname);
        if (ret==0)
        {
            size_t bufsize;
            char *buf = NULL;
            size_t buftmp;
            bufsize = zip_entry_read(archives.zips[q], (void **)&buf, &buftmp);
            if (size!=NULL)
                *size = bufsize;
            return buf;
        }
    }
    TraceLog("ZIP: file not found in archives %s",fname);
    return NULL;
}
//    obvious 
Image zipLoadImage(const char *fname)
{
    int buffsize;
    void *buffer = zipLoadFileData(fname,&buffsize);
    Image im = {0};
    if (buffer!=NULL)
    {
        int nFrames = 0;
        TraceLog(LOG_INFO,"ZIP: %s from archive",fname);
        im = LoadImageAnimFromMemory(GetFileExtension(fname),buffer,buffsize,&nFrames);
        printf("LoadImage %s %d %d\n",fname,im.width,im.height);
        free(buffer);
    }
    else 
    {
        TraceLog(LOG_ERROR,"ZIP: LoadImage %s not found in archives",fname);
    }
    return im;
}
//    obviouser
Texture2D zipLoadTexture(const char *fname)
{
    return LoadTextureFromImage(zipLoadImage(fname));
}

//    simple test 
#if 1
void main(int argc,char **argv)
{
    Camera camera = { 0 };
    camera.position = (Vector3){ 10,10,10 }; 
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;                                
    camera.projection = CAMERA_PERSPECTIVE;             

    //    EDIT THIS
    //  change this to where your archives are stored 
    //    EDIT THIS
    zipsMount("quake path",".pk3");

    InitWindow(1280, 720, "ted");

    //    EDIT THIS
    Texture2D test = zipLoadTexture("textures/sfx/sewerwater_base.dds");
    //    EDIT THIS

    while (!WindowShouldClose())
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
        ClearBackground(BLACK);
        BeginMode3D(camera);        
        DrawGrid(20,20);
        EndMode3D();
        DrawTexture(test,0,0,WHITE);
        DrawFPS(10,10);
        EndDrawing();
    }
}
#endif
