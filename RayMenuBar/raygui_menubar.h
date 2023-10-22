//  note W is width of button, not bar 
//  note H is height of button AND bar

typedef struct 
{
    const char *string;
    int         uID;
    void        *data;
} MenuBarEntry;

#ifdef RAYGUI_MENUBAR_IMPLEMENTATION
// Button control, returns true when clicked
static int FocusedGuiRect(Rectangle bounds)
{
    int result = 0;
    GuiState state = GuiGetState();
    if ((state != STATE_DISABLED) && !guiLocked && !guiSliderDragging)
    {
        Vector2 mousePoint = GetMousePosition();
        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                state = STATE_PRESSED;
            else
                state = STATE_FOCUSED;
        }
    }
    return (state); 
}

int GuiShowMenu(MenuBarEntry **menu,int w,int h)
{
    int Align = GuiGetStyle(BUTTON,TEXT_ALIGNMENT);
    GuiStatusBar((Rectangle){0,0,GetScreenWidth(),h},"");
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT,TEXT_ALIGN_LEFT);

    int ret = 0;
    bool isClosed = false;
    bool isFocused = false;
    int isOpen = -1;
    // find out who's open
    for (int m=0;menu[m]!=NULL;m++)
    {
        MenuBarEntry *item = menu[m];
        if (item->uID!=0)
            isOpen = m;
    }
    //  for all menus
    for (int m=0;menu[m]!=NULL;m++)
    {
        Rectangle rect=(Rectangle){m*w,0,w,h};
        MenuBarEntry *item = menu[m];
        //  first item in menu is the top level button string
        //  tell the system we're focused on something
        if (FocusedGuiRect(rect))   isFocused=true;
        //  put the Menu title button up 
        if (GuiButton(rect,TextFormat(" %s",item->string)))
        {
            //  if we hit the button, we toggle the menu on/off
            item->uID=!item->uID;
            //  set is closed if the top menu is switched off.
            if (item->uID==0)   isClosed=true;
        }
        //  if it's open
        if (item->uID!=0)
        {
            //  check if it's the one we had last time 
            if ((m!=isOpen) && (isOpen!=-1))
                menu[isOpen]->uID=0;

            //  add the other entries
            for (int q=1;item[q].string!=NULL;q++)
            {
                //  step down 1 slot
                rect.y+=rect.height;
                //  set if we're focused
                if (FocusedGuiRect(rect))
                    isFocused=true;
                //  handle button press
                if (GuiButton(rect,item[q].string))
                {
                    item->uID=false;
                    isClosed=true;
                    ret = item[q].uID;
                }
            }
        }
    }
    //  if we're not focused and left click, close the menu
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && (isFocused==false))
        isClosed = true;

    //  turn everything off
    if (isClosed==true)
    {
        for (int m=0;menu[m]!=NULL;m++)
        {
            menu[m]->uID=false;
        }
    }
    //  restore alignment
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT,Align);
    return ret;
}

#else 

//  note W is width of button, not bar 
//  note H is height of button AND bar

int GuiShowMenu(MenuBarEntry **menu,int w,int h);
#endif
