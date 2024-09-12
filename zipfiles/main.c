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

typedef struct ZFS_zip
{
    struct zip_t *zip;
    struct ZFS_zip *next;
} ZFS_zip;

ZFS_zip *archives=NULL;
unsigned char* ZFS_LoadFileData(const char* path, int* len)
{
    ZFS_zip* archive = archives;
    char* rp = path;

    if ((path[0] == '.') && (path[1]=='/'))
        rp = path + 3;

    while (archive != NULL)
    {
        int zf = zip_entry_open(archive->zip, rp);
        if (zf == 0)
        {
            void* buffer = NULL;
            int length;
            zip_entry_read(archive->zip, &buffer, &length);
            *len = length;
            TraceLog(LOG_INFO, "ZFS:Loaded %s %x %d", path, buffer, length);
            return buffer;
        }
        archive = archive->next;
    }
    TraceLog(LOG_ERROR, "ZFS:%s not found\n", path);
    return NULL;
}

void ZFS_UnMountArchives()
{
    ZFS_zip* archive = archives;

    while (archive != NULL)
    {
        free(archive->zip);
        archive = archive->next;
    }
}

void ZFS_MountArchives(const char *path,const char *ext)
{
    int errnum = 0;
    FilePathList flist = LoadDirectoryFilesEx(path,ext,true);
    for (unsigned int q=0;q<flist.count;q++)
    {
        struct zip_t *z= zip_openwitherror(flist.paths[q], 0,'r',&errnum);
        if (errnum==0)
        {
            ZFS_zip *archive = (ZFS_zip*)RL_CALLOC(1,sizeof(ZFS_zip));
            archive->zip = z;
            archive->next = archives;
            TraceLog(LOG_INFO,"ZFS:Mounted %s",flist.paths[q]);
            archives = archive;
        }
    }
    SetLoadFileDataCallback(ZFS_LoadFileData);
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
    ZFS_MountArchives("archives path",".zip");

    Texture2D texture = LoadTexture("file inside zip");
    InitWindow(1280, 720, "ted");

    //  LoadModel, LoadTexture, LoadImage will now all load from the archives 
    //  assuming the zip contains the file

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
        DrawTexture(texture,0,0,WHITE);
        DrawFPS(10,10);
        EndDrawing();
    }
    UnloadTexture(texture);
    ZFS_UnMountArchives();
}
#endif
