
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

#include "config.h"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define MAX_INSTANCES  4000

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame
Camera camera;
Shader skinningShader;

typedef struct 
{
    float CurrentFrameTime;
    int CurrentAnimation;
    int nAnimations;
    Texture2D *pAnimationTextures;
    Model model; 
} AnimModel;

AnimModel animtest;
Model animtest_sharedanim;

Matrix *transforms;
void CreateMatrixTextures(int nAnimations,Model model,ModelAnimation *Animations,Texture2D *AnimationTextures);
void SetModelMapTexture(Model m,int map,Texture2D tex);
void SetModelMaterialsShader(Model m,Shader shader);

#define SHADER_LOC_MAP_ANIMATION        SHADER_LOC_MAP_BRDF
#define MATERIAL_MAP_ANIMATION          MATERIAL_MAP_BRDF 

enum 
{
    SHADER_LOC_FRAME_A = SHADER_LOC_BONE_MATRICES+1,
    SHADER_LOC_FRAME_B,
    SHADER_LOC_TEXTURE_SIZE,
};

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1024;
    int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - Everything on GPU skinning");
    ChangeDirectory("resources");

    // Define transforms to be uploaded to GPU for instances
    transforms = (Matrix *)RL_CALLOC(MAX_INSTANCES, sizeof(Matrix));   // Pre-multiplied transformations passed to rlgl

    // Translate and rotate cubes randomly
    for (int i = 0; i < MAX_INSTANCES; i++)
    {
        Matrix translation = MatrixTranslate((float)GetRandomValue(-20, 20), (float)GetRandomValue(-20, 20), (float)GetRandomValue(-20, 20));
        Matrix rotation = MatrixRotateY(GetRandomValue(0,360)*DEG2RAD);
        transforms[i] = MatrixMultiply(rotation, translation);
    }

    // Define the camera to look into our 3d world
    camera.position = (Vector3){ 5.0f, 5.0f, 5.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };  // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type

    //--------------------------------------------------------------------------------------
    // Shader
    //--------------------------------------------------------------------------------------

    skinningShader = LoadShader(TextFormat("shaders/glsl%i/skinning.vs", GLSL_VERSION),
                                TextFormat("shaders/glsl%i/skinning.fs", GLSL_VERSION));
    //  get location of AtlasTexture 

    skinningShader.locs[SHADER_LOC_MAP_ANIMATION] = GetShaderLocation(skinningShader,"AnimationTexture");
    skinningShader.locs[SHADER_LOC_FRAME_A] = GetShaderLocation(skinningShader,"fframeA");
    skinningShader.locs[SHADER_LOC_FRAME_B] = GetShaderLocation(skinningShader,"fframeB");
#if defined(PLATFORM_WEB)
    skinningShader.locs[SHADER_LOC_TEXTURE_SIZE] = GetShaderLocation(skinningShader,"texSize");
#endif

    //--------------------------------------------------------------------------------------
    // Load models and create animation textures
    //--------------------------------------------------------------------------------------


    const char *modelfile = "models/gltf/character-female-b.glb";
    animtest.model = LoadModel(modelfile);
    ModelAnimation *pAnimations = LoadModelAnimations(modelfile,&animtest.nAnimations);

    //  alloc space for textures
    animtest.pAnimationTextures = (Texture2D*)RL_CALLOC(animtest.nAnimations,sizeof(Texture2D));

    CreateMatrixTextures(animtest.nAnimations,animtest.model,pAnimations,animtest.pAnimationTextures);

    //  load another model mapped to the same skeleton
    animtest_sharedanim = LoadModel("models/gltf/character-male-c.glb");

    SetModelMaterialsShader(animtest.model,skinningShader);
    SetModelMaterialsShader(animtest_sharedanim,skinningShader);

    // it's fine to get rid of the animation data now
    UnloadModelAnimations(pAnimations,animtest.nAnimations);

    //--------------------------------------------------------------------------------------
    // demo
    //--------------------------------------------------------------------------------------

    DisableCursor();                    // Limit cursor to relative movement inside the window

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);     // Set our game frames-per-second
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif
    //--------------------------------------------------------------------------------------
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(skinningShader);
    CloseWindow();              // Close window and OpenGL context
    return 0;
}


//--------------------------------------------------------------------------------------
//  utils
//--------------------------------------------------------------------------------------
void SetModelMapTexture(Model m,int map,Texture2D tex)
{
    for (int q=0;q<m.materialCount;q++)
        m.materials[q].maps[map].texture = tex;
}

void SetModelMaterialsShader(Model m,Shader shader)
{
    for (int q=0;q<m.materialCount;q++)
    {
        m.materials[q].shader = shader;
    }    
}

//  dupe of rmodel DrawModel but using a raw matrix
static void DrawModelMtx(Model model, Matrix matTransform, Color tint)
{
    matTransform = MatrixMultiply(matTransform,model.transform);
    for (int i = 0; i < model.meshCount; i++)
    {
        Color color = model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;

        Color colorTint = WHITE;
        colorTint.r = (unsigned char)(((int)color.r*(int)tint.r)/255);
        colorTint.g = (unsigned char)(((int)color.g*(int)tint.g)/255);
        colorTint.b = (unsigned char)(((int)color.b*(int)tint.b)/255);
        colorTint.a = (unsigned char)(((int)color.a*(int)tint.a)/255);

        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = colorTint;
        DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], matTransform);
        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = color;
    }
}


//----------------------------------------------------------------------------------
// demo main loop 
//----------------------------------------------------------------------------------

void UpdateDrawFrame()
{
    float dt = GetFrameTime();

    //  just tick the frames
    animtest.CurrentFrameTime+=dt * 30.0f; // 30fps animations

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        UpdateCamera(&camera, CAMERA_FREE);
    

//    if (IsKeyPressed(KEY_T)) animtest.CurrentAnimation++;
//    else if (IsKeyPressed(KEY_G)) animtest.CurrentAnimation--;

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(BLACK);
        DrawRectangleGradientV(0,0,GetScreenWidth(),GetScreenHeight(),(Color){64,64,64,255},(Color){4,4,4,255});
        BeginMode3D(camera);

        float start = GetTime();

        for (int q=0;q<MAX_INSTANCES;q++)
        {
            //  change animation based on q
            animtest.CurrentAnimation=q%animtest.nAnimations;
            //  texture height IS number of frames for this animation
            //  fmod is fine for this test, though forreal BE smarter
            float frameA = fmod(animtest.CurrentFrameTime , animtest.pAnimationTextures[animtest.CurrentAnimation].height);
            float frameB = fmod(animtest.CurrentFrameTime+1.0f,animtest.pAnimationTextures[animtest.CurrentAnimation].height);

            //  update the shader with frame A ( start frame ) and frame B ( end frame )
            //  the fraction of frame A is used to smooth between A & B 
            SetShaderValue(skinningShader,skinningShader.locs[SHADER_LOC_FRAME_A],&frameA,SHADER_UNIFORM_FLOAT);
            SetShaderValue(skinningShader,skinningShader.locs[SHADER_LOC_FRAME_B],&frameB,SHADER_UNIFORM_FLOAT);

#if defined(PLATFORM_WEB)
            Vector2 texSize = (Vector2){animtest.pAnimationTextures[animtest.CurrentAnimation].width,animtest.pAnimationTextures[animtest.CurrentAnimation].height};
            SetShaderValue(skinningShader,skinningShader.locs[SHADER_LOC_TEXTURE_SIZE],&texSize,SHADER_UNIFORM_VEC2);            
#endif
            //  test shared skeleton 
            if (q&1)
            {
                //  set the animation for this model 
                SetModelMapTexture(animtest.model,MATERIAL_MAP_ANIMATION,animtest.pAnimationTextures[animtest.CurrentAnimation]);
                //  draw 
                DrawModelMtx(animtest.model,transforms[q],WHITE);
            }
            else                
            {
                //  set the animation for this model 
                SetModelMapTexture(animtest_sharedanim,MATERIAL_MAP_ANIMATION,animtest.pAnimationTextures[animtest.CurrentAnimation]);
                //  draw 
                DrawModelMtx(animtest_sharedanim,transforms[q],WHITE);
            }
        }
        //  just for timing
        float end = GetTime();
        DrawGrid(10, 1.0f);
        EndMode3D();

    DrawFPS(10,10);
    DrawText(TextFormat("ms %f anim %d",end-start,animtest.CurrentAnimation),10,30,18,WHITE);
    EndDrawing();
    //----------------------------------------------------------------------------------
}

// Convert float to half-float (stored as unsigned short)
static unsigned short FloatToHalf(float x)
{
    unsigned short result = 0;

    const unsigned int b = (*(unsigned int*) & x) + 0x00001000; // Round-to-nearest-even: add last bit after truncated mantissa
    const unsigned int e = (b & 0x7F800000) >> 23; // Exponent
    const unsigned int m = b & 0x007FFFFF; // Mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
    result = (b & 0x80000000) >> 16 | (e > 112)*((((e - 112) << 10) & 0x7C00) | m >> 13) | ((e < 113) & (e > 101))*((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) | (e > 143)*0x7FFF; // sign : normalized : denormalized : saturate
    return result;
}
//  convert animations to a texture
//      number of bones * 4 
//  each animation is a unique texture
//  each frame pose is a horizontal line of the texture
//  frame pose is number of bones * 16 floats

void CreateMatrixTextures(int nAnimations,Model model,ModelAnimation *Animations,Texture2D *AnimationTextures)
{
    for (int a=0;a<nAnimations;a++)
    {
        ModelAnimation anim = Animations[a];

        Image bat;
        bat.width = anim.boneCount*4;   //  4 elements of a matrix * number of bones
        bat.height = anim.frameCount;
        bat.mipmaps = 1;
        uint8_t *ptr = RL_CALLOC(bat.width * bat.height,(sizeof(float)*4));
        unsigned short *pp = (unsigned short*)ptr;

        //  we only need 3 half floats for each matrix row ( we ignore the last row )
        //  always 0,0,0,1
        #define STRIDE 3
        bat.format = RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16;

        bat.data = ptr;
        int z=0;
        for (int frame=0;frame<anim.frameCount;frame++)
        {
            int y =frame;
            for (int boneId = 0;boneId<bat.width/4;boneId++)
            {

                int x = boneId*4; // NOTE 4 elements here for the matrix

                Vector3 inTranslation = model.bindPose[boneId].translation;
                Quaternion inRotation = model.bindPose[boneId].rotation;
                Vector3 inScale = model.bindPose[boneId].scale;

                Vector3 outTranslation =    anim.framePoses[frame][boneId].translation;
                Quaternion outRotation =    anim.framePoses[frame][boneId].rotation;
                Vector3 outScale =          anim.framePoses[frame][boneId].scale;

                Vector3 invTranslation = Vector3RotateByQuaternion(Vector3Negate(inTranslation), QuaternionInvert(inRotation));
                Quaternion invRotation = QuaternionInvert(inRotation);
                Vector3 invScale = Vector3Divide((Vector3){ 1.0f, 1.0f, 1.0f }, inScale);

                Vector3 boneTranslation = Vector3Add(Vector3RotateByQuaternion(Vector3Multiply(outScale, invTranslation),outRotation), outTranslation);
                Quaternion boneRotation = QuaternionMultiply(outRotation, invRotation);
                Vector3 boneScale = Vector3Multiply(outScale, invScale);

                Matrix boneMatrix = MatrixMultiply(
                                        MatrixMultiply(
                                                    QuaternionToMatrix(boneRotation),
                                                    MatrixTranslate(boneTranslation.x, boneTranslation.y, boneTranslation.z)
                                                    ),
                                                    MatrixScale(boneScale.x, boneScale.y, boneScale.z));
                
                //  we don't need to send the last ROW of 0,0,0,1
                ((unsigned short *)bat.data)[(y*bat.width + x)*STRIDE] =             FloatToHalf(boneMatrix.m0);
                ((unsigned short *)bat.data)[(y*bat.width + x)*STRIDE + 1] =         FloatToHalf(boneMatrix.m1);
                ((unsigned short *)bat.data)[(y*bat.width + x)*STRIDE + 2] =         FloatToHalf(boneMatrix.m2);

                ((unsigned short *)bat.data)[(y*bat.width + (x+1))*STRIDE] =         FloatToHalf(boneMatrix.m4);
                ((unsigned short *)bat.data)[(y*bat.width + (x+1))*STRIDE + 1] =     FloatToHalf(boneMatrix.m5);
                ((unsigned short *)bat.data)[(y*bat.width + (x+1))*STRIDE + 2] =     FloatToHalf(boneMatrix.m6);

                ((unsigned short *)bat.data)[(y*bat.width + (x+2))*STRIDE] =         FloatToHalf(boneMatrix.m8);
                ((unsigned short *)bat.data)[(y*bat.width + (x+2))*STRIDE + 1] =     FloatToHalf(boneMatrix.m9);
                ((unsigned short *)bat.data)[(y*bat.width + (x+2))*STRIDE + 2] =     FloatToHalf(boneMatrix.m10);

                ((unsigned short *)bat.data)[(y*bat.width + (x+3))*STRIDE] =         FloatToHalf(boneMatrix.m12);
                ((unsigned short *)bat.data)[(y*bat.width + (x+3))*STRIDE + 1] =     FloatToHalf(boneMatrix.m13);
                ((unsigned short *)bat.data)[(y*bat.width + (x+3))*STRIDE + 2] =     FloatToHalf(boneMatrix.m14);
            }
        }
        AnimationTextures[a] = LoadTextureFromImage(bat);
        //  for wasm
        SetTextureFilter(AnimationTextures[a],TEXTURE_FILTER_POINT);
        SetTextureWrap(AnimationTextures[a],TEXTURE_WRAP_REPEAT);
    }
}

