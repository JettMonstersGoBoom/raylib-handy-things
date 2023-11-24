#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"
#include "raymath.h"

#include "raygui.h"

#include "treeview.h"

void GuiTreeShowItemDefault(TreeView_Item_T *node,Rectangle *rect,int indent,TreeView_Item_T **selected)
{
    if (GuiButton(*rect,""))
    {
        *selected = node;
    }
    _GuiDrawText(TextFormat("%*s%s",indent,"",node->Text),*rect,TEXT_ALIGN_LEFT,WHITE);
    rect->y+=rect->height;
}

//  define the startup panel
//  they're adjusted by the main loop in this example 
TreeView tView = 
{
    .panel ={ 0, 24, 256, 1024 },
    .content =  { 0, 0, 240, 32768 },
    .scroll =  {0,0},
    .ypadding = 0,
    .view = { 0, 0, 256,1024},
    .ShowItemCallback = GuiTreeShowItemDefault,
    .Root = NULL
};

void AddFolder(const char *filepath,TreeView_Item_T *parent)
{
    FilePathList flist = LoadDirectoryFiles(filepath);
    for (int q=0;q<flist.count;q++)
	{
        const char *path = GetFileName(flist.paths[q]);
        //  ignore . at the start of path *ignored* 
        if (path[0]!='.')
        {
            TreeView_Item_T *item = NULL;
            if (DirectoryExists(flist.paths[q]))
            {
                item = AddTreeItem(path,parent);
                AddFolder(flist.paths[q],item);
            }
            else
            {
                item = AddTreeItem(path,parent);
            }
            if (item!=NULL)
            {
                item->Data = strdup(flist.paths[q]);
            }
        }
    }
}


int main(int argc,char **argv)
{
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "TreeView");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    tView.Root = AddTreeItem("Root",NULL);
    AddFolder(".",tView.Root);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        //  to account for resize
        tView.panel.width = (float)(GetScreenWidth()/3);
        tView.panel.height = (float)(GetScreenHeight() - 48);

        tView.view.width = tView.panel.width;
        tView.view.height = tView.panel.height;
        tView.content.width = tView.view.width;

        TreeView_Item_T *selected = GuiTreeView(&tView);
        if (selected!=NULL)
        {
            printf("this was selected %s %s\n",selected->Text,selected->Data);
        }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    FreeTreeView(tView.Root);
    CloseWindow();        // Close window and OpenGL context
    return 0;
}
