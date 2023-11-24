#include <stdint.h>

#ifndef RAYGUI_ICON_SIZE
#define RAYGUI_ICON_SIZE               16          // Size of icons in pixels (squared)
#endif

typedef struct TreeView_Item_T
{
    char *                  Text;
    uint8_t                 isOpen:1; 
    struct TreeView_Item_T* Children;
    struct TreeView_Item_T* Next;
    void*                   Data;
} TreeView_Item_T;

typedef struct 
{
    //  this could be part of the item struct if you wanted to show different types of data per item
    void (*ShowItemCallback)(TreeView_Item_T *node,Rectangle *rect,int indent,TreeView_Item_T **selected);

    Rectangle       panel;
    Rectangle       content;
    Rectangle       view;
    Vector2         scroll;
    TreeView_Item_T *Root;
    int             ypadding;
    int             renderedItems;
    int             consideredItems;
} TreeView;

TreeView_Item_T*    AddTreeItem(const char *Text,struct TreeView_Item_T *parent);
void                FreeTreeView(TreeView_Item_T *node);
void                GuiTreeViewItem(TreeView *tree_view,TreeView_Item_T *node, Rectangle *rect,int indent,TreeView_Item_T **selected);
TreeView_Item_T*    GuiTreeView(TreeView *tree_view);

void _GuiDrawText(const char *text, Rectangle textBounds, int alignment, Color tint);


