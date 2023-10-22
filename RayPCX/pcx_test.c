#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "raylib.h"
#include "rlgl.h"

#define RAYLIB_PCX_IMPLEMENTATION
#include "ray_pcx.h"

//  NOTE 
//  we use fragColor.red ( Tint ) to set which palette {1,0,0,0} selects the 2nd palette 

const char indexed_shader[]={"#version 330\n"\
                            "in vec2 fragTexCoord;\n"\
                            "in vec4 fragColor;\n"\
                            "uniform sampler2D texture0;\n"\
                            "uniform sampler2D clut0;\n"\
                            "out vec4 finalColor;\n"\
                            "void main(){\n"\
                            "float findex = texture2D(texture0,fragTexCoord).r;\n"\
                            "int index = int(findex * 256.0);\n"\
                            "int palette = int(fragColor.r * 256.0);\n"\
                            "if (index==0) { discard; }\n"\
                            "finalColor = texture2D(clut0 , vec2((index/256.0),float(palette)/16.0));\n"
                            "finalColor.a = 1;\n}"};

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
Color colr;
    //  room for 16 full Palettes 
    unsigned char Palettes[16][768];

    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 640;
    int screenHeight = 480;
    
    InitWindow(screenWidth, screenHeight, "raylib editor template");

    memset(Palettes,0,sizeof(Palettes));
    Image LoadedPCXImage = LoadImagePCX("test.pcx",&Palettes[0][0]);
    Image ImagePalette = (Image)
    {
        .width = 256,
        .height = 16,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8,
        .data = Palettes,
        .mipmaps = 1
    };

    Shader ShaderIndexed = LoadShaderFromMemory(NULL,indexed_shader);
    int ShaderUniformClut = GetShaderLocation(ShaderIndexed,"clut0");
    Texture2D TexturePalette = LoadTextureFromImage(ImagePalette);
    Texture2D TextureImage = LoadTextureFromImage(LoadedPCXImage);

    UnloadImage(LoadedPCXImage);
    UnloadImage(ImagePalette);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
        ClearBackground(BLACK);

        BeginShaderMode(ShaderIndexed);
        SetShaderValueTexture(ShaderIndexed,ShaderUniformClut,TexturePalette);
        DrawTexture(TextureImage,0,0,(Color){0,0,0,0});
        EndShaderMode();

        DrawTextureEx(TexturePalette,(Vector2){0,GetScreenHeight()-(16*8)},0,3,WHITE);
        DrawFPS(10,10);
        EndDrawing();

        //----------------------------------------------------------------------------------
    }

    UnloadTexture(TextureImage);
    UnloadTexture(TexturePalette);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}
