//  clear tui screen
void tui_cls();
//  just attributes
void tui_puta(int x, int y,uint8_t attr);
uint8_t tui_geta(int x, int y);
//  just cell
void tui_putc(int x, int y,uint8_t c);
uint8_t tui_getc(int x, int y);

//  both cell and attribute
void tui_put(int x, int y,uint16_t c);
uint16_t tui_get(int x, int y);

void tui_set_attribute(uint8_t attr);

//  does NOT draw box, just shadows for it
void tui_shadowbox(int x,int y,int w,int h);
//  shows color combos
void tui_colorbox(int x,int y);

//  bordered filled box
void tui_box(int x,int y,int w,int h);
void tui_box_decorated(int x,int y,int w,int h);

//  slider
void tui_bar(int x,int y,int val,int max);
//  print string 
void tui_print(int x, int y, const char *text, ...);

//  selection box
bool tui_selection_group(int x,int y,const char *group,int *index);
//  menu bar
uint16_t tui_menu(const char **menu,int w,int h);

//  setup  
//  font texture
void tui_set_font(Texture2D font);
//  palette texture
void tui_set_palette(Texture2D pal);

void tui_create(int width,int height);
void tui_present(int view_scale,Color tint);

#ifdef TUI_IMPLEMENTATION
#include "rlgl.h"

#define GLSL(X) \
  "#version 330\n" \
  #X

static const char* shaderVS = GLSL(
// Input vertex attributes
in vec3 vertexPosition;
in vec4 vertexColor;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
// Input uniform values
uniform mat4 mvp;

uniform vec2 viewportSize;
uniform vec2 viewportOffset;
uniform vec2 inverseMapTextureSize;
uniform float inverseTileSize;

out vec2 fragTexCoord;
out vec2 fragPixelCoord;
out vec4 fragColor;
out vec2 fragCoord;
void main()
{
    fragPixelCoord = (vertexTexCoord * viewportSize) + viewportOffset;
    fragTexCoord = (fragPixelCoord * inverseMapTextureSize) * inverseTileSize;
    gl_Position = mvp*vec4(vertexPosition, 1.0);
    fragCoord = vertexTexCoord;
    fragColor = vertexColor;
}
);

static const char *shaderFS = GLSL(
in vec2 fragPixelCoord;
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D tileMapTexture;
uniform sampler2D tileSourceTexture;
uniform sampler2D paletteTexture;
uniform vec2 inverseSourceTextureSize;
uniform float tileSize;
out vec4 finalColor;
void main()
{
    if ((fragTexCoord.x<0) || (fragTexCoord.x>1) || (fragTexCoord.y<0) || (fragTexCoord.y>1))
        discard;

    //  two channels of 8 bit data
    float tileIndex = texture2D(tileMapTexture, fragTexCoord).r;
    float attrIndex = texture2D(tileMapTexture, fragTexCoord).a;
    //  low byte
    int index = int(floor(tileIndex * 255.0));
    int attrByte = int(floor(attrIndex * 255.0));
    if (attrByte==0) discard;
    vec2 spriteOffset = vec2(index&0xf,index/16) * tileSize;
    vec2 spriteCoord = (mod(fragPixelCoord, tileSize));
    spriteCoord.x = floor(spriteCoord.x);
    spriteCoord.y = floor(spriteCoord.y);

    float fg = (attrByte&0xf)/15.9f;
    float bg = ((attrByte>>4)&0xf)/15.9f;

    float level = texture2D(tileSourceTexture , (spriteOffset + spriteCoord) * inverseSourceTextureSize).r;
    float neglevel = 1.0-level;
    finalColor = texture2D(paletteTexture , vec2(fg,0.0)) * vec4(level,level,level,1.0) + texture2D(paletteTexture , vec2(bg,0.0)) * vec4(neglevel,neglevel,neglevel,1.0);
    finalColor.a = 1;
    finalColor*=fragColor;
}
);

static struct 
{
    struct 
    {
        Texture2D tileMap;
        Texture2D tilePalette;
        Texture2D tileImage;
    } textures;
    Image text_buffer;
    int tileSize;
    int mx,my;
    bool Dirty;
    uint8_t attribute;
} rl_debug_t = {0};

static Shader rl_tilemap_shader = {0};
static struct 
{
    int viewportSize;
    int viewportOffset;
    int inverseSourceTextureSize;
    int inverseTileSize;
    int inverseMapSize;
    int tileSize;
    int sourceTexture;
    int mapTexture;
    int paletteTexture;
} rl_tilemap_locations;

bool tui_mouse_over(int x,int y,int w,int h)
{
    if ((rl_debug_t.mx<x) ||
        (rl_debug_t.my<y) ||
        (rl_debug_t.mx>=x+w) || 
        (rl_debug_t.my>=y+h))
        return false;   
    return true;
}

static const char **tui_textsplit(const char *text, char delimiter, int *count)
{
    #if !defined(RAYGUI_TEXTSPLIT_MAX_ITEMS)
        #define RAYGUI_TEXTSPLIT_MAX_ITEMS          128
    #endif
    #if !defined(RAYGUI_TEXTSPLIT_MAX_TEXT_SIZE)
        #define RAYGUI_TEXTSPLIT_MAX_TEXT_SIZE     1024
    #endif

    static const char *result[RAYGUI_TEXTSPLIT_MAX_ITEMS] = { NULL };   // String pointers array (points to buffer data)
    static char buffer[RAYGUI_TEXTSPLIT_MAX_TEXT_SIZE] = { 0 };         // Buffer data (text input copy with '\0' added)
    memset(buffer, 0, RAYGUI_TEXTSPLIT_MAX_TEXT_SIZE);

    result[0] = buffer;
    int counter = 1;

    // Count how many substrings we have on text and point to every one
    for (int i = 0; i < RAYGUI_TEXTSPLIT_MAX_TEXT_SIZE; i++)
    {
        buffer[i] = text[i];
        if (buffer[i] == '\0') break;
        else if ((buffer[i] == delimiter) || (buffer[i] == '\n'))
        {
            result[counter] = buffer + i + 1;
            buffer[i] = '\0';   // Set an end of string at this point
            counter++;
            if (counter > RAYGUI_TEXTSPLIT_MAX_ITEMS) break;
        }
    }

    *count = counter;

    return result;
}

void tui_shadow_vertical(int x,int y,int h)
{
    for (int _y=0;_y<=h-1;_y++)
    {
        uint16_t val = tui_get(x,y + _y);
        uint16_t atr = 0x0100;
        tui_put(x,y+_y,(val&0xff) | atr);
    }
}

void tui_shadow_horizontal(int x,int y,int w)
{
    for (int q=0;q<=w;q++)
    {
        uint16_t val = tui_get(x + q,y);
        uint16_t atr =  0x100;
        tui_put(x+q,y,(val&0xff) | atr);
    }
}

void tui_set_attribute(uint8_t attr)
{
    rl_debug_t.attribute = attr;
}
//  cell and attr
void tui_put(int x, int y,uint16_t c)
{
    uint16_t *scr = rl_debug_t.text_buffer.data;
    if ((x>=rl_debug_t.text_buffer.width) || (y>=rl_debug_t.text_buffer.height) || (x<0) || (y<0)) return;
    scr[x + (y*rl_debug_t.text_buffer.width)] = c;
    rl_debug_t.Dirty = true;
}


uint16_t tui_get(int x, int y)
{
    uint16_t *scr = rl_debug_t.text_buffer.data;
    if ((x>=rl_debug_t.text_buffer.width) || (y>=rl_debug_t.text_buffer.height) || (x<0) || (y<0)) return 0;
    return (scr[x + (y*rl_debug_t.text_buffer.width)]);
}

//  just cell

void tui_putc(int x, int y,uint8_t c)
{
    uint16_t *scr = rl_debug_t.text_buffer.data;
    if ((x>=rl_debug_t.text_buffer.width) || (y>=rl_debug_t.text_buffer.height) || (x<0) || (y<0)) return;
    scr[x + (y*rl_debug_t.text_buffer.width)] = (scr[x + (y*rl_debug_t.text_buffer.width)] & 0xff00) | c;
    rl_debug_t.Dirty = true;
}

uint8_t tui_getc(int x, int y)
{
    return (tui_get(x,y)&0xff);
}

void tui_puta(int x, int y,uint8_t attr)
{
    uint16_t *scr = rl_debug_t.text_buffer.data;
    if ((x>=rl_debug_t.text_buffer.width) || (y>=rl_debug_t.text_buffer.height) || (x<0) || (y<0)) return;
    scr[x + (y*rl_debug_t.text_buffer.width)] = (scr[x + (y*rl_debug_t.text_buffer.width)] & 0xff) | (attr<<8);
    rl_debug_t.Dirty = true;
}

//  just the attribute
uint8_t tui_geta(int x, int y)
{
    return (tui_get(x,y)>>8);
}

void tui_cls()
{
    memset(rl_debug_t.text_buffer.data,0,(rl_debug_t.text_buffer.width * 2) * rl_debug_t.text_buffer.height);
    rl_debug_t.Dirty = true;
}

bool tui_button(Rectangle rect,const char *str)
{
bool hover = tui_mouse_over(rect.x,rect.y,rect.width,rect.height);
bool ret = false;
uint16_t atr = rl_debug_t.attribute;

    if ((hover==true) && (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
        ret = true;

    rl_debug_t.attribute = hover==true ? 0x93 : 0x13;

    tui_print(rect.x,rect.y,"%*.*s", -(int)rect.width,(int)rect.width,str);

    rl_debug_t.attribute = atr;
    return ret;
}

//  returns 16 bit number 
//  11111111-------- menu index
//  --------11111111 submenu index 
//  0x0103 menu 1, subutem 3 
uint16_t tui_menu(const char **menu,int w,int h)
{
static int      GuiShowMenuAsciiOpen = -1;
uint16_t  ret = 0;
    tui_set_attribute(0x13);
    tui_box(0,0,rl_debug_t.textures.tileMap.width,1);
    //  full bar across
    for (int m=0;menu[m]!=NULL;m++)
    {
        Rectangle rect=(Rectangle){m*w,0,w,h};
        char *ent = strdup(menu[m]);
        char *tok = strtok(ent,":");

        if (tui_button(rect,TextFormat(" %s",tok)))  GuiShowMenuAsciiOpen = m;

        int subItem=0;
        if (GuiShowMenuAsciiOpen==m)
        {
            while(tok!=NULL)
            {
                rect.y+=h;
                tok = strtok(NULL,":");
                subItem++;
                if (tok!=NULL)
                {
                    if (tui_button(rect,tok))
                    {
                        ret = (m<<8) | subItem;
                        GuiShowMenuAsciiOpen = -1;
                    }
                }
            }
            tui_shadow_horizontal(rect.x+1,rect.y,w-2);
            tui_shadow_vertical((m*w)+w,1,rect.y);
        }
        free(ent);
    }
    return ret;
}


void tui_shadowbox(int x,int y,int w,int h)
{
    tui_shadow_vertical(x+w+1,y+1,h);
    tui_shadow_horizontal(x+1,y+h+1,w);
}

void tui_colorbox(int x,int y)
{
    uint16_t atr ;
    for (int q=0;q<16;q++)
    {
        for (int _y=0;_y<16;_y++)
        {
            atr = q<<12 | _y<<8;
            tui_put(x+q,y+_y,0xb1 | atr);
        }
    }
    tui_shadowbox(x,y,15,15);
}


#define BOX_UL 0x89  
#define BOX_UH 0x8a //196  
#define BOX_UR 0x8b 
#define BOX_VL 0x8c 
#define BOX_VR 0x8d 
#define BOX_BL 0x8e
#define BOX_BH 0x8f //196  
#define BOX_BR 0x90

void tui_box(int x,int y,int w,int h)
{
    uint16_t atr = rl_debug_t.attribute<<8;
    for (int q=0;q<w;q++)
    {
        for (int _y=0;_y<h;_y++)
        {
            tui_put(x+q,y+_y,0x20 | atr);
        }
    }
    tui_shadowbox(x,y,w-1,h-1);
}

void tui_box_decorated(int x,int y,int w,int h)
{
    uint16_t atr = rl_debug_t.attribute<<8;
    tui_put(x,y,BOX_UL | atr);
    tui_put(x+w,y,BOX_UR | atr);
    tui_put(x,y+h,BOX_BL | atr);
    tui_put(x+w,y+h,BOX_BR | atr);
    for (int q=1;q<=w-1;q++)
    {
        tui_put(x+q,y,BOX_UH | atr);
        tui_put(x+q,y+h,BOX_BH | atr);

        for (int _y=1;_y<=h-1;_y++)
        {
            tui_put(x+q,y+_y,0x20 | atr);
        }
    }
    for (int _y=1;_y<=h-1;_y++)
    {
        tui_put(x,y + _y,BOX_VL | atr);
        tui_put(x+w,y+_y,BOX_VR | atr);
    }
    tui_shadowbox(x,y,w,h);
}

void tui_bar(int x,int y,int val,int max)
{
    if (val>=max) val=max-1;
    int q=0;
    for (q=0;q<(val>>3);q++)
        tui_put(x+q,y,0x88 | (rl_debug_t.attribute<<8));
    q+=1;
    for (;q<(max>>3);q++)
        tui_put(x+q,y,0x00 | (rl_debug_t.attribute<<8));
    tui_put(x+(val>>3),y,(128+(val&7)) | (rl_debug_t.attribute<<8));
}

void tui_print(int x, int y, const char *text, ...)
{
	char tmp[1024];
	va_list args;
	const char *p;
	// Expand the formatting string.
	va_start(args, text);
	vsnprintf(tmp, sizeof(tmp), text, args);
	tmp[sizeof(tmp)-1] = 0;
	va_end(args);
    uint16_t atr = rl_debug_t.attribute<<8;
	// Print each glyph.
	p = tmp;
	while (*p)
	{
        unsigned char c = *p;
        tui_put(x,y,c | atr);
        x+=1;
        p++;
	}
}

bool tui_selection_group(int x,int y,const char *group,int *index)
{
int count;
const char **entries = tui_textsplit(group,':',&count);
int wide = 0;
int attr = rl_debug_t.attribute;
int hovered = -1;
bool ret = false;
    for (int q=0;q<count;q++)
    {
        if (strlen(entries[q])>wide)
            wide = strlen(entries[q]);
    }   
    tui_box(x,y,wide+2,count+1);
    for (int q=0;q<count;q++)
    {
        rl_debug_t.attribute=attr;
        bool mover = tui_mouse_over(x+1,y+1+q,wide,1);
        if (mover==true)
        {
            rl_debug_t.attribute=0x53;
            hovered=q;
        }
        tui_print(x+2,1+y+q,entries[q]);
        tui_put(x+1,1+y+q,((*index)==q ? 0x7 : 0x9) | (rl_debug_t.attribute<<8));
    }
    if (hovered!=-1)
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            *index = hovered;
            ret = true;
        }
    }
    return (ret);
}

void tui_set_font(Texture2D font)
{
    rl_debug_t.textures.tileImage = font;
}

void tui_set_palette(Texture2D pal)
{
    rl_debug_t.textures.tilePalette = pal;
}

void tui_create(int width,int height)
{
    if (rl_tilemap_shader.id==0)
    {
        rl_tilemap_shader=LoadShaderFromMemory(shaderVS,shaderFS);
    }
    rl_debug_t.text_buffer.width = width;
    rl_debug_t.text_buffer.height = height;
    rl_debug_t.text_buffer.mipmaps = 1;
    rl_debug_t.text_buffer.data = RL_CALLOC(2,rl_debug_t.text_buffer.width*rl_debug_t.text_buffer.height);
    rl_debug_t.text_buffer.format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;

    rl_debug_t.textures.tileMap = LoadTextureFromImage(rl_debug_t.text_buffer);    

    rl_tilemap_locations.viewportSize = GetShaderLocation(rl_tilemap_shader,"viewportSize");
    rl_tilemap_locations.viewportOffset = GetShaderLocation(rl_tilemap_shader,"viewportOffset");
    rl_tilemap_locations.inverseSourceTextureSize = GetShaderLocation(rl_tilemap_shader,"inverseSourceTextureSize");
    rl_tilemap_locations.inverseMapSize = GetShaderLocation(rl_tilemap_shader,"inverseMapTextureSize");
    rl_tilemap_locations.inverseTileSize = GetShaderLocation(rl_tilemap_shader,"inverseTileSize");
    rl_tilemap_locations.tileSize = GetShaderLocation(rl_tilemap_shader,"tileSize");
    rl_tilemap_locations.mapTexture = GetShaderLocation(rl_tilemap_shader,"tileMapTexture");
    rl_tilemap_locations.sourceTexture = GetShaderLocation(rl_tilemap_shader,"tileSourceTexture");
    rl_tilemap_locations.paletteTexture = GetShaderLocation(rl_tilemap_shader,"paletteTexture");
    rl_debug_t.tileSize = 8;
    rl_debug_t.Dirty = true;
}

void tui_present(int scale,Color tint)
{
    if (rl_debug_t.textures.tileImage.id==0)
    {
        TraceLog(LOG_ERROR,"TUI FONT needs to be set with tui_set_font ");
        return;
    }

    if (rl_debug_t.textures.tilePalette.id==0)
    {
        TraceLog(LOG_ERROR,"TUI COLORS need to be set with tui_set_palette ");
        return;
    }

    float view_scale = fminf(GetRenderWidth()/(rl_debug_t.text_buffer.width*8), GetScreenHeight()/(rl_debug_t.text_buffer.height*8));
    if (view_scale<1) view_scale=1.0f;
    view_scale = (int)view_scale*scale;

    rl_debug_t.mx = GetMouseX()/view_scale;
    rl_debug_t.my = GetMouseY()/view_scale;

    Rectangle outrect = (Rectangle){0,0,rl_debug_t.text_buffer.width * view_scale,rl_debug_t.text_buffer.height * view_scale};
    Rectangle inrect = (Rectangle){0,0,rl_debug_t.text_buffer.width*rl_debug_t.tileSize,rl_debug_t.text_buffer.height*rl_debug_t.tileSize};
    Vector2 inverseSourceTextureSize = (Vector2){1.0f/(float)rl_debug_t.textures.tileImage.width,1.0f/(float)rl_debug_t.textures.tileImage.height};
    Vector2 inverseTilemapTextureSize = (Vector2){1.0f/(float)rl_debug_t.textures.tileMap.width,1.0f/(float)rl_debug_t.textures.tileMap.height};
    float tileSize = (float)rl_debug_t.tileSize;
    float invTileSize = 1.0f/tileSize;

    if (rl_debug_t.Dirty==true)
    {
        UpdateTexture(rl_debug_t.textures.tileMap, rl_debug_t.text_buffer.data);
        rl_debug_t.Dirty = false;
    }

    SetTextureFilter(rl_debug_t.textures.tileImage, TEXTURE_FILTER_POINT);  
    SetTextureFilter(rl_debug_t.textures.tileMap, TEXTURE_FILTER_POINT);  
    SetTextureFilter(rl_debug_t.textures.tilePalette, TEXTURE_FILTER_POINT);  


    BeginShaderMode(rl_tilemap_shader);
    SetShaderValue(rl_tilemap_shader,rl_tilemap_locations.viewportSize,&inrect.width,SHADER_UNIFORM_VEC2);
    SetShaderValue(rl_tilemap_shader,rl_tilemap_locations.viewportOffset,&inrect.x,SHADER_UNIFORM_VEC2);
    SetShaderValue(rl_tilemap_shader,rl_tilemap_locations.inverseSourceTextureSize,&inverseSourceTextureSize,SHADER_UNIFORM_VEC2);
    SetShaderValue(rl_tilemap_shader,rl_tilemap_locations.inverseMapSize,&inverseTilemapTextureSize,SHADER_UNIFORM_VEC2);
    SetShaderValue(rl_tilemap_shader,rl_tilemap_locations.tileSize,&tileSize,SHADER_UNIFORM_FLOAT);
    SetShaderValue(rl_tilemap_shader,rl_tilemap_locations.inverseTileSize,&invTileSize,SHADER_UNIFORM_FLOAT);


    SetShaderValueTexture(rl_tilemap_shader,rl_tilemap_locations.sourceTexture,rl_debug_t.textures.tileImage);
    SetShaderValueTexture(rl_tilemap_shader,rl_tilemap_locations.mapTexture,rl_debug_t.textures.tileMap);
    SetShaderValueTexture(rl_tilemap_shader,rl_tilemap_locations.paletteTexture,rl_debug_t.textures.tilePalette);


    rlActiveTextureSlot(rl_tilemap_locations.sourceTexture);
    rlSetTexture(rl_debug_t.textures.tileImage.id);

    rlActiveTextureSlot(rl_tilemap_locations.mapTexture);
    rlSetTexture(rl_debug_t.textures.tileMap.id);

    rlActiveTextureSlot(rl_tilemap_locations.paletteTexture);
    rlSetTexture(rl_debug_t.textures.tilePalette.id);

    rlBegin(RL_QUADS);
    rlColor4ub(tint.r,tint.g,tint.b,tint.a);
//  topleft
    rlTexCoord2f(0.0f,0.0f);
    rlVertex2f(outrect.x,outrect.y);
//  bottom left
    rlTexCoord2f(0.0f,1.0f);
    rlVertex2f(outrect.x,outrect.y + outrect.height);
//  bottom right
    rlTexCoord2f(1.0f,1.0f);
    rlVertex2f(outrect.x + outrect.width,outrect.y + outrect.height);
//  top right
    rlTexCoord2f(1.0f,0.0f);
    rlVertex2f(outrect.x + outrect.width,outrect.y);
    rlEnd();

    rlActiveTextureSlot(rl_tilemap_locations.sourceTexture);
    rlDisableTexture();
    rlActiveTextureSlot(rl_tilemap_locations.mapTexture);
    rlDisableTexture();
    EndShaderMode();
}

#endif