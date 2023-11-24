#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"
#include "raymath.h"

#define ICON_TEXT_PADDING 0
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "treeview.h"

void _GuiDrawText(const char *text, Rectangle textBounds, int alignment, Color tint)
{
    GuiDrawText(text,textBounds,alignment,tint);
}

//  call this to add items to the tree
TreeView_Item_T* AddTreeItem(const char *Text,struct TreeView_Item_T *parent)
{
TreeView_Item_T*  item;

    item = ( TreeView_Item_T* )RL_CALLOC( 1,sizeof( TreeView_Item_T ) );
    if ( !item )
    {
        /* Memory allocation failed! */
        printf( "AddTreeItem(): Failed to allocate memory\n" );
        return( NULL );
    }
    item->Text = strdup(Text);
    item->isOpen = false;
    if (parent!=NULL)
    {
        TreeView_Item_T*  last_item = parent->Children;
        //  if we don't currently have children. make this item the first child
        if (last_item==NULL)
        {
            parent->Children = item;
        }
        else
        {
            //  otherwise we find the last item in the list
            while(last_item->Next!=NULL)
                last_item=last_item->Next;
            last_item->Next = item;
        }
    }
    return( item );
}

//  release memory 
void FreeTreeView(TreeView_Item_T *node)
{
    while(node!=NULL)
    {
        if (node->Children==NULL)
        {
            FreeTreeView(node->Children);
        }
        TreeView_Item_T *next = node->Next;
        if (node->Data!=NULL)
            RL_FREE(node->Data);
        RL_FREE(node->Text);
        RL_FREE(node);
        node = next;
    }
}

void GuiTreeViewItem(TreeView *tree_view,TreeView_Item_T *node, Rectangle *rect,int indent,TreeView_Item_T **selected)
{
    int current_border_width = GuiGetStyle(BUTTON, BORDER_WIDTH);
    Vector2 textSize = MeasureTextEx(GuiGetFont(), TextFormat("%*s",indent,""), (float)GuiGetStyle(DEFAULT, TEXT_SIZE), (float)GuiGetStyle(DEFAULT, TEXT_SPACING));
    textSize.x -= RAYGUI_ICON_SIZE;

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiSetStyle(BUTTON, BORDER_WIDTH,0);

    while(node!=NULL)
    {
        bool shouldDraw = (rect->y>tree_view->panel.y-rect->height) && (rect->y<tree_view->panel.y+tree_view->view.height);
        tree_view->consideredItems++;
        if (shouldDraw)
        {
            rect->width = tree_view->content.width;
        }
        if (node->Children==NULL)
        {
            if (shouldDraw)
            {
                tree_view->renderedItems++;
                if (tree_view->ShowItemCallback!=NULL)
                    tree_view->ShowItemCallback(node,rect,indent,selected);
            }
        }
        else 
        {
            if (shouldDraw)
            {
                tree_view->renderedItems++;
                if (GuiButton(*rect,""))
                {
                    node->isOpen=!node->isOpen;
                }
                GuiDrawText(TextFormat("%*s%s",indent,"",node->Text),*rect,TEXT_ALIGN_LEFT,WHITE);
                GuiDrawIcon(node->isOpen ? 116 : 115,rect->x+textSize.x,rect->y,1,WHITE);
            }
            if (node->isOpen)
            {
                rect->y+=rect->height;
                if (node->Children!=NULL)
                {
                    int pad = GuiGetStyle(DEFAULT,TEXT_PADDING);
                    indent+=2;
                    GuiTreeViewItem(tree_view,node->Children,rect,indent,selected);
                    indent-=2;
                }
            }
            else 
            {
                rect->y+=rect->height;
            }
        }
        node = node->Next;
    }
    GuiSetStyle(BUTTON, BORDER_WIDTH,current_border_width);
}

TreeView_Item_T *GuiTreeView(TreeView *tree_view)
{
TreeView_Item_T *selected = NULL;

    tree_view->content.height = ((tree_view->consideredItems) * (GuiGetStyle(DEFAULT,TEXT_SIZE) + tree_view->ypadding));
    GuiScrollPanel(tree_view->panel, NULL, tree_view->content, &tree_view->scroll, &tree_view->view);

    tree_view->consideredItems = 0;
    BeginScissorMode((int)tree_view->view.x, (int)tree_view->view.y, (int)tree_view->view.width, (int)tree_view->view.height);
    Rectangle startRect = (Rectangle){  tree_view->view.x,
                                        tree_view->view.y + tree_view->scroll.y,
                                        tree_view->view.width,
                                        GuiGetStyle(DEFAULT, TEXT_SIZE)+tree_view->ypadding };

        GuiTreeViewItem(tree_view,tree_view->Root,&startRect,2,&selected);

    EndScissorMode();
    return (selected);
}

