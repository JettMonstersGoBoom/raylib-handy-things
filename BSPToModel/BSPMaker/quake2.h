
#ifdef _Q2_IMPL_

//  q2 levels are scaled really big, we can tweak that here 
#ifndef Q2_VERTEX_SCALE 
    #define Q2_VERTEX_SCALE 0.01f
#endif

//  control the final single lightmap image size
#ifndef LIGHTMAP_WIDTH
    #define LIGHTMAP_WIDTH 1024
#endif
#ifndef LIGHTMAP_HEIGHT
    #define LIGHTMAP_HEIGHT 1024 
#endif

//  will show the allocations in the log, and export "lightmap.png"
//  #define DEBUG_LIGHTMAP 

//  on disk location of shaders
#ifndef LIGHTMAP_VS
    #define LIGHTMAP_VS "shaders/lightmaped.vs"
#endif 
#ifndef LIGHTMAP_FS
    #define LIGHTMAP_FS "shaders/lightmaped.fs"
#endif 

#ifndef SKY_VS
    #define SKY_VS "shaders/sky.vs"
#endif 

#ifndef SKY_FS
    #define SKY_FS "shaders/sky.fs"
#endif 

#ifndef WAVE_FS
    #define WAVE_FS "shaders/wave.fs"
#endif

#include <float.h>

#include "builder.h"
#include "lightmap.h"
#include "qstrcmp.h"

//  surface flags
#define SURF_LIGHT 0x0001
#define SURF_SKY2D 0x0002
#define SURF_SKY3D 0x0004
#define SURF_WARP  0x0008	
#define SURF_TRANS33 0x0010
#define SURF_TRANS66 0x0020
#define SURF_TRANS (SURF_TRANS33 | SURF_TRANS66 | SURF_WARP)
#define SURF_NODRAW 0x0080
#define SURF_SKIP = 0x200
//  quake 2 lumps
enum 
{
    Lump_Entities,
    Lump_Planes,
    Lump_Vertices,
    Lump_Visibility,
    Lump_Nodes,
    Lump_Texture_Information,
    Lump_Faces,
    Lump_Lightmaps,
    Lump_Leaves,
    Lump_Leaf_Face_Table,
    Lump_Leaf_Brush_Table,
    Lump_Edges,
    Lump_Face_Edge_Table,
    Lump_Models,
    Lump_Brushes,
    Lump_Brush_Sides,
    Lump_Pop,
    Lump_Areas,
    Lump_Area_Portals
} Q2_LumpType;

typedef struct
{
    char *Name;
    Texture2D   Texture;
} CachedTexture2D;

#pragma pack(push,1)
typedef struct iVector4 {
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t w;
} iVector4;

typedef struct 
{
    uint32_t magic;
    uint32_t version;
} Q2BSP_header_t;

typedef struct 
{
    uint32_t offset,length;
} Q2BSP_lump_t;

typedef struct 
{
    Vector3     u_axis;
    float       u_offset;
    Vector3     v_axis;
    float       v_offset;
    uint32_t    flags;
    uint32_t    value;
    char        texture_name[32];
    uint32_t    next_texinfo;
} Q2BSP_texinfo_t;

typedef struct 
{
    Vector3 min,max;
    Vector3 origin;
    int32_t headnode;
    int32_t first_face,nFaces;
} Q2BSP_model_t;

typedef struct 
{
    uint16_t   plane;             // index of the plane the face is parallel to
    uint16_t   plane_side;        // set if the normal is parallel to the plane normal

    uint32_t   first_edge;        // index of the first edge (in the face edge array)
    uint16_t   num_edges;         // number of consecutive edges (in the face edge array)
	
    uint16_t   texture_info;      // index of the texture info structure	
   
    uint8_t    lightmap_styles[4]; // styles (bit flags) for the lightmaps
    uint32_t   lightmap_offset;   // offset of the lightmap (in bytes) in the lightmap lump
} Q2BSP_face_t;

typedef struct 
{
    uint16_t v[2];
} Q2BSP_edge_t;

#define	MIPLEVELS	4
typedef struct miptex_s
{
    char		name[32];
    unsigned	width, height;
    unsigned	offsets[MIPLEVELS];		// four mip maps stored
    char		animname[32];			// next frame in animation chain
    int			flags;
    int			contents;
    int			value;
} Q2BSP_wal_texture_t;

#pragma pack(pop)

typedef struct 
{
    //  we load all these
    int                 nVertices;
    Vector3             *Vertices;

    int                 nFaces;
    Q2BSP_face_t        *Faces;

    int                 nEdges;
    Q2BSP_edge_t        *Edges;

    uint32_t            nFaceEdges;
    uint32_t            *FaceEdges;

    int                 nModels;
    Q2BSP_model_t       *Models;

    int                 nTexInfo;
    Q2BSP_texinfo_t     *TexInfo;

    int                 nEntityString;
    uint8_t             *EntityString;

    int                 nlightMapRaw;
    uint8_t             *lightMapRaw;

    //                  we make these
    int                 nCachedTextures;
    CachedTexture2D     *CachedTextures;
    Model               *model;
    VertexContainer     *meshVertices;
    struct 
    {
        Shader          shader;
    } Sky;
    struct 
    {
        Shader          shader;
        int             secondsLoc;
        int             freqXLoc;
        int             freqYLoc;
        int             ampXLoc;
        int             ampYLoc;
        int             speedXLoc;
        int             speedYLoc;
        float           time;
    } Warp;
    struct 
    {
        Image           image;
        Texture2D       texture;
        Shader          shader;
    } lightMap;
} Q2BSP_info_t;

//-----------------------------------------------------------

void Q2BSP_PrepareLightmap(Q2BSP_info_t *bsp)
{
    TraceLog(LOG_INFO,"Lightmap %d x %d",LIGHTMAP_WIDTH,LIGHTMAP_HEIGHT);
    bsp->lightMap.image.height = LIGHTMAP_HEIGHT;
    bsp->lightMap.image.width = LIGHTMAP_WIDTH;
    bsp->lightMap.image.mipmaps = 1;
    bsp->lightMap.image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    bsp->lightMap.image.data = RL_CALLOC(LIGHTMAP_HEIGHT * LIGHTMAP_WIDTH, sizeof(uint8_t) * 3);
    bsp->lightMap.texture = LoadTextureFromImage(bsp->lightMap.image);
    memset(lightmap_allocation, 0, sizeof(lightmap_allocation));
    int lX,lY;
    LightmapAllocBlock(2,2,&lX,&lY);
    
    for (int32_t Y = 0; Y < 2; Y++)
    {
        for (int32_t X = 0; X < 2; X++)
        {
            ImageDrawPixel(&bsp->lightMap.image, X, Y, WHITE);
        }
    }
}

//-----------------------------------------------------------
//  Q2 color table
static unsigned char Q2BSP_ColorTable[768] = 
{
	0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x1F, 0x1F, 0x1F, 0x2F, 0x2F, 0x2F,
	0x3F, 0x3F, 0x3F, 0x4B, 0x4B, 0x4B, 0x5B, 0x5B, 0x5B, 0x6B, 0x6B, 0x6B,
	0x7B, 0x7B, 0x7B, 0x8B, 0x8B, 0x8B, 0x9B, 0x9B, 0x9B, 0xAB, 0xAB, 0xAB,
	0xBB, 0xBB, 0xBB, 0xCB, 0xCB, 0xCB, 0xDB, 0xDB, 0xDB, 0xEB, 0xEB, 0xEB,
	0x63, 0x4B, 0x23, 0x5B, 0x43, 0x1F, 0x53, 0x3F, 0x1F, 0x4F, 0x3B, 0x1B,
	0x47, 0x37, 0x1B, 0x3F, 0x2F, 0x17, 0x3B, 0x2B, 0x17, 0x33, 0x27, 0x13,
	0x2F, 0x23, 0x13, 0x2B, 0x1F, 0x13, 0x27, 0x1B, 0x0F, 0x23, 0x17, 0x0F,
	0x1B, 0x13, 0x0B, 0x17, 0x0F, 0x0B, 0x13, 0x0F, 0x07, 0x0F, 0x0B, 0x07,
	0x5F, 0x5F, 0x6F, 0x5B, 0x5B, 0x67, 0x5B, 0x53, 0x5F, 0x57, 0x4F, 0x5B,
	0x53, 0x4B, 0x53, 0x4F, 0x47, 0x4B, 0x47, 0x3F, 0x43, 0x3F, 0x3B, 0x3B,
	0x3B, 0x37, 0x37, 0x33, 0x2F, 0x2F, 0x2F, 0x2B, 0x2B, 0x27, 0x27, 0x27,
	0x23, 0x23, 0x23, 0x1B, 0x1B, 0x1B, 0x17, 0x17, 0x17, 0x13, 0x13, 0x13,
	0x8F, 0x77, 0x53, 0x7B, 0x63, 0x43, 0x73, 0x5B, 0x3B, 0x67, 0x4F, 0x2F,
	0xCF, 0x97, 0x4B, 0xA7, 0x7B, 0x3B, 0x8B, 0x67, 0x2F, 0x6F, 0x53, 0x27,
	0xEB, 0x9F, 0x27, 0xCB, 0x8B, 0x23, 0xAF, 0x77, 0x1F, 0x93, 0x63, 0x1B,
	0x77, 0x4F, 0x17, 0x5B, 0x3B, 0x0F, 0x3F, 0x27, 0x0B, 0x23, 0x17, 0x07,
	0xA7, 0x3B, 0x2B, 0x9F, 0x2F, 0x23, 0x97, 0x2B, 0x1B, 0x8B, 0x27, 0x13,
	0x7F, 0x1F, 0x0F, 0x73, 0x17, 0x0B, 0x67, 0x17, 0x07, 0x57, 0x13, 0x00,
	0x4B, 0x0F, 0x00, 0x43, 0x0F, 0x00, 0x3B, 0x0F, 0x00, 0x33, 0x0B, 0x00,
	0x2B, 0x0B, 0x00, 0x23, 0x0B, 0x00, 0x1B, 0x07, 0x00, 0x13, 0x07, 0x00,
	0x7B, 0x5F, 0x4B, 0x73, 0x57, 0x43, 0x6B, 0x53, 0x3F, 0x67, 0x4F, 0x3B,
	0x5F, 0x47, 0x37, 0x57, 0x43, 0x33, 0x53, 0x3F, 0x2F, 0x4B, 0x37, 0x2B,
	0x43, 0x33, 0x27, 0x3F, 0x2F, 0x23, 0x37, 0x27, 0x1B, 0x2F, 0x23, 0x17,
	0x27, 0x1B, 0x13, 0x1F, 0x17, 0x0F, 0x17, 0x0F, 0x0B, 0x0F, 0x0B, 0x07,
	0x6F, 0x3B, 0x17, 0x5F, 0x37, 0x17, 0x53, 0x2F, 0x17, 0x43, 0x2B, 0x17,
	0x37, 0x23, 0x13, 0x27, 0x1B, 0x0F, 0x1B, 0x13, 0x0B, 0x0F, 0x0B, 0x07,
	0xB3, 0x5B, 0x4F, 0xBF, 0x7B, 0x6F, 0xCB, 0x9B, 0x93, 0xD7, 0xBB, 0xB7,
	0xCB, 0xD7, 0xDF, 0xB3, 0xC7, 0xD3, 0x9F, 0xB7, 0xC3, 0x87, 0xA7, 0xB7,
	0x73, 0x97, 0xA7, 0x5B, 0x87, 0x9B, 0x47, 0x77, 0x8B, 0x2F, 0x67, 0x7F,
	0x17, 0x53, 0x6F, 0x13, 0x4B, 0x67, 0x0F, 0x43, 0x5B, 0x0B, 0x3F, 0x53,
	0x07, 0x37, 0x4B, 0x07, 0x2F, 0x3F, 0x07, 0x27, 0x33, 0x00, 0x1F, 0x2B,
	0x00, 0x17, 0x1F, 0x00, 0x0F, 0x13, 0x00, 0x07, 0x0B, 0x00, 0x00, 0x00,
	0x8B, 0x57, 0x57, 0x83, 0x4F, 0x4F, 0x7B, 0x47, 0x47, 0x73, 0x43, 0x43,
	0x6B, 0x3B, 0x3B, 0x63, 0x33, 0x33, 0x5B, 0x2F, 0x2F, 0x57, 0x2B, 0x2B,
	0x4B, 0x23, 0x23, 0x3F, 0x1F, 0x1F, 0x33, 0x1B, 0x1B, 0x2B, 0x13, 0x13,
	0x1F, 0x0F, 0x0F, 0x13, 0x0B, 0x0B, 0x0B, 0x07, 0x07, 0x00, 0x00, 0x00,
	0x97, 0x9F, 0x7B, 0x8F, 0x97, 0x73, 0x87, 0x8B, 0x6B, 0x7F, 0x83, 0x63,
	0x77, 0x7B, 0x5F, 0x73, 0x73, 0x57, 0x6B, 0x6B, 0x4F, 0x63, 0x63, 0x47,
	0x5B, 0x5B, 0x43, 0x4F, 0x4F, 0x3B, 0x43, 0x43, 0x33, 0x37, 0x37, 0x2B,
	0x2F, 0x2F, 0x23, 0x23, 0x23, 0x1B, 0x17, 0x17, 0x13, 0x0F, 0x0F, 0x0B,
	0x9F, 0x4B, 0x3F, 0x93, 0x43, 0x37, 0x8B, 0x3B, 0x2F, 0x7F, 0x37, 0x27,
	0x77, 0x2F, 0x23, 0x6B, 0x2B, 0x1B, 0x63, 0x23, 0x17, 0x57, 0x1F, 0x13,
	0x4F, 0x1B, 0x0F, 0x43, 0x17, 0x0B, 0x37, 0x13, 0x0B, 0x2B, 0x0F, 0x07,
	0x1F, 0x0B, 0x07, 0x17, 0x07, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x77, 0x7B, 0xCF, 0x6F, 0x73, 0xC3, 0x67, 0x6B, 0xB7, 0x63, 0x63, 0xA7,
	0x5B, 0x5B, 0x9B, 0x53, 0x57, 0x8F, 0x4B, 0x4F, 0x7F, 0x47, 0x47, 0x73,
	0x3F, 0x3F, 0x67, 0x37, 0x37, 0x57, 0x2F, 0x2F, 0x4B, 0x27, 0x27, 0x3F,
	0x23, 0x1F, 0x2F, 0x1B, 0x17, 0x23, 0x13, 0x0F, 0x17, 0x0B, 0x07, 0x07,
	0x9B, 0xAB, 0x7B, 0x8F, 0x9F, 0x6F, 0x87, 0x97, 0x63, 0x7B, 0x8B, 0x57,
	0x73, 0x83, 0x4B, 0x67, 0x77, 0x43, 0x5F, 0x6F, 0x3B, 0x57, 0x67, 0x33,
	0x4B, 0x5B, 0x27, 0x3F, 0x4F, 0x1B, 0x37, 0x43, 0x13, 0x2F, 0x3B, 0x0B,
	0x23, 0x2F, 0x07, 0x1B, 0x23, 0x00, 0x13, 0x17, 0x00, 0x0B, 0x0F, 0x00,
	0x00, 0xFF, 0x00, 0x23, 0xE7, 0x0F, 0x3F, 0xD3, 0x1B, 0x53, 0xBB, 0x27,
	0x5F, 0xA7, 0x2F, 0x5F, 0x8F, 0x33, 0x5F, 0x7B, 0x33, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xD3, 0xFF, 0xFF, 0xA7, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0x53,
	0xFF, 0xFF, 0x27, 0xFF, 0xEB, 0x1F, 0xFF, 0xD7, 0x17, 0xFF, 0xBF, 0x0F,
	0xFF, 0xAB, 0x07, 0xFF, 0x93, 0x00, 0xEF, 0x7F, 0x00, 0xE3, 0x6B, 0x00,
	0xD3, 0x57, 0x00, 0xC7, 0x47, 0x00, 0xB7, 0x3B, 0x00, 0xAB, 0x2B, 0x00,
	0x9B, 0x1F, 0x00, 0x8F, 0x17, 0x00, 0x7F, 0x0F, 0x00, 0x73, 0x07, 0x00,
	0x5F, 0x00, 0x00, 0x47, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x1B, 0x00, 0x00,
	0xEF, 0x00, 0x00, 0x37, 0x37, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF,
	0x2B, 0x2B, 0x23, 0x1B, 0x1B, 0x17, 0x13, 0x13, 0x0F, 0xEB, 0x97, 0x7F,
	0xC3, 0x73, 0x53, 0x9F, 0x57, 0x33, 0x7B, 0x3F, 0x1B, 0xEB, 0xD3, 0xC7,
	0xC7, 0xAB, 0x9B, 0xA7, 0x8B, 0x77, 0x87, 0x6B, 0x57, 0x9F, 0x5B, 0x53
};

//-----------------------------------------------------------
//  NOTE - use the first mip, convert to 32 bit and generate mips in hardware

Texture2D Q2BSP_LoadTexture(const char *fileName)
{
Texture2D ret = {0};
    int len;
    uint8_t *data = LoadFileData(TextFormat("textures/%s.wal",fileName),&len);
    if (data!=NULL)
    {
        Q2BSP_wal_texture_t *wal = (Q2BSP_wal_texture_t*)data;
        Image wal_image =
        {
            .width = wal->width,
            .height=wal->height,
            .mipmaps = 1,
            .data = RL_CALLOC((wal->width*wal->height),sizeof(uint32_t)),
            .format=PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
        };

        uint8_t *pixel = &data[wal->offsets[0]];
        for (uint32_t y=0;y<wal->height;y++)
        {
            for (uint32_t x=0;x<wal->width;x++)
            {
                Color color;
                uint16_t pindex = pixel[x+(y*wal->width)]*3;
                color.r = Q2BSP_ColorTable[pindex];
                color.g = Q2BSP_ColorTable[pindex+1];
                color.b = Q2BSP_ColorTable[pindex+2];
                color.a = 255;
                ImageDrawPixel(&wal_image, x,y, color);
            }
        }

        ret = LoadTextureFromImage(wal_image);
        GenTextureMipmaps(&ret);
        SetTextureFilter( ret,TEXTURE_FILTER_TRILINEAR);
    }
    else 
    {
        //  generate the name of the texture as a texture, so we can see in the application what texture failed
//        ret = LoadTextureFromImage(ImageText(fileName,24,WHITE));;
        ret = LoadTextureFromImage(GenImageColor(8, 8, RED));
    }
    RL_FREE(data);
    return (ret);
}

//-----------------------------------------------------------
//  remove duplicate textures, just return the previously found one in our cache

static Texture2D Q2BSP_LoadTextureCached(Q2BSP_info_t *bsp,const char *name)
{
    for (int q=0;q<bsp->nCachedTextures;q++)
    {
        if (qstricmp(bsp->CachedTextures[q].Name,name)==0)
            return bsp->CachedTextures[q].Texture;
    }
    int index = bsp->nCachedTextures;
    bsp->nCachedTextures++;
    bsp->CachedTextures = RL_REALLOC(bsp->CachedTextures,sizeof(CachedTexture2D) * bsp->nCachedTextures);
    bsp->CachedTextures[index].Name = strdup(name);    
    bsp->CachedTextures[index].Texture = Q2BSP_LoadTexture(name);
    return bsp->CachedTextures[index].Texture;
}

//-----------------------------------------------------------
//  generate a passable normal

static Vector3 VerticesNormal(Vector3 a, Vector3 b, Vector3 c)
{
	Vector3 ba = Vector3Subtract(b, a);
	Vector3 ca = Vector3Subtract(c, a);
	return Vector3Normalize(Vector3CrossProduct(ba, ca));
}

//-----------------------------------------------------------
//  used to calculate the size of the face for lightmap UVs

static iVector4 Q2BSP_CalculateFaceExtents(Q2BSP_info_t* bsp, Q2BSP_face_t* face, int index)
{
    Vector2 UVMin = (Vector2){ FLT_MAX,FLT_MAX };
    Vector2 UVMax = (Vector2){ -FLT_MAX, -FLT_MAX };
    int lindex = bsp->FaceEdges[face->first_edge];
    Q2BSP_edge_t* edge = (lindex > 0) ? &bsp->Edges[lindex] : &bsp->Edges[-lindex];
    Q2BSP_texinfo_t* texinfo = &bsp->TexInfo[face->texture_info];

    Vector3* v[3];
    Vector2 uv;

    for (uint32_t e = 0; e < face->num_edges; e++)
    {
        lindex = bsp->FaceEdges[face->first_edge + e];
        edge = (lindex > 0) ? &bsp->Edges[lindex] : &bsp->Edges[-lindex];
        if (lindex > 0)
            v[0] = &bsp->Vertices[edge->v[0]];
        else
            v[0] = &bsp->Vertices[edge->v[1]];
        uv.x = (Vector3DotProduct(*v[0], texinfo->u_axis) + texinfo->u_offset);
        uv.y = (Vector3DotProduct(*v[0], texinfo->v_axis) + texinfo->v_offset);

        if (UVMin.x > uv.x) UVMin.x = (float)uv.x;
        if (UVMin.y > uv.y) UVMin.y = (float)uv.y;
        if (UVMax.x < uv.x) UVMax.x = (float)uv.x;
        if (UVMax.y < uv.y) UVMax.y = (float)uv.y;
    }
    int32_t minX = (int32_t)floorf((UVMin.x / 16.0f));
    int32_t minY = (int32_t)floor((UVMin.y / 16.0f));
    int32_t maxX = (int32_t)ceil((UVMax.x / 16.0f));
    int32_t maxY = (int32_t)ceil((UVMax.y / 16.0f));
    iVector4 result;
    result.x = (int)(minX * 16);
    result.y = (int)(minY * 16);
    result.z = (int)((maxX - minX) * 16);
    result.w = (int)((maxY - minY) * 16);
    return result;
}

//-----------------------------------------------------------
//  add a Q2 Bsp face to our meshes

static void Q2BSP_AddFace(Q2BSP_info_t *bsp,Q2BSP_face_t *face,int index)
{
    if (face->first_edge > bsp->nFaceEdges) return;
    int lindex = bsp->FaceEdges[face->first_edge];                    
    Q2BSP_edge_t *edge = (lindex > 0) ? &bsp->Edges[lindex] : &bsp->Edges[-lindex];
    Vector3 *v[3];
    if (lindex>0)
        v[0]= &bsp->Vertices[edge->v[0]];
    else
        v[0]= &bsp->Vertices[edge->v[1]];

    float uscale,vscale;

    Q2BSP_texinfo_t *texinfo = &bsp->TexInfo[face->texture_info];
    int matIndex = face->texture_info;
    if (matIndex>bsp->model->materialCount) 
    {
        TraceLog(LOG_ERROR,"material index is invalid");
        return;
    }
    VertexContainer *vc = &bsp->meshVertices[matIndex];
    uscale = (float)bsp->model->materials[matIndex].maps[MATERIAL_MAP_ALBEDO].texture.width;
    vscale = (float)bsp->model->materials[matIndex].maps[MATERIAL_MAP_ALBEDO].texture.height;

    iVector4 extents;
    uint8_t* LightmapTexels = NULL;
    int32_t LightmapBlockWidth = 0;
    int32_t LightmapBlockHeight = 0;
    int32_t lmpOffX = 0;
    int32_t lmpOffY = 0;
    
    extents.x = 0;
    extents.y = 0;
    extents.z = 0;
    extents.w = 0;
    
    if (face->lightmap_offset!=0xffffffff)
    {
        extents = Q2BSP_CalculateFaceExtents(bsp, face, index);
        LightmapTexels = &bsp->lightMapRaw[face->lightmap_offset];
        LightmapBlockWidth = (((int)extents.z) >> 4) + 1;
        LightmapBlockHeight = (((int)extents.w) >> 4) + 1;
        LightmapAllocBlock(LightmapBlockWidth, LightmapBlockHeight, &lmpOffX, &lmpOffY);

        for (int32_t Y = 0; Y < LightmapBlockHeight; Y++)
        {
            for (int32_t X = 0; X < LightmapBlockWidth; X++)
            {
                Color color = BLACK;
                {
        //          24 bpp
                    color.r = *LightmapTexels++;
                    color.g = *LightmapTexels++;
                    color.b = *LightmapTexels++;
                }
                ImageDrawPixel(&bsp->lightMap.image, X + lmpOffX, Y + lmpOffY, color);
            }
        }
    }

    if (bsp->TexInfo[face->texture_info].flags&SURF_TRANS!=0)
        LightmapBlockWidth=0;

    Vector3 normal;
    for (uint16_t e = 1; e < face->num_edges-1 ; e++)
    {
        lindex = bsp->FaceEdges[face->first_edge + e];                    
        edge = (lindex > 0) ? &bsp->Edges[lindex] : &bsp->Edges[-lindex];

        if (lindex<0)
        {
            v[1]= &bsp->Vertices[edge->v[0]];
            v[2]= &bsp->Vertices[edge->v[1]];
        }
        else
        {
            v[1]= &bsp->Vertices[edge->v[1]];
            v[2]= &bsp->Vertices[edge->v[0]];
        }
        normal = VerticesNormal(*v[0],*v[1],*v[2]);
        for (int i=0;i<3;i++)
        {
            Vector2 uv;
            Vector2 uv2;

		    uv.x = (Vector3DotProduct(*v[i], texinfo->u_axis) + texinfo->u_offset) / uscale;
            uv.y = (Vector3DotProduct(*v[i], texinfo->v_axis) + texinfo->v_offset) / vscale;
            if (LightmapBlockWidth==0)
            {
                uv2.x = 0.0f;
                uv2.y = 0.0f;
            }
            else 
            {
                uv2.x = (Vector3DotProduct(*v[i], texinfo->u_axis) + texinfo->u_offset);
                uv2.y = (Vector3DotProduct(*v[i], texinfo->v_axis) + texinfo->v_offset);
                uv2.x -= extents.x;
                uv2.x += lmpOffX * 16;
                uv2.x += 8;
                uv2.y -= extents.y;
                uv2.y += lmpOffY * 16;
                uv2.y += 8;
                uv2.x /= (float)(LIGHTMAP_WIDTH * 16);
                uv2.y /= (float)(LIGHTMAP_HEIGHT * 16);
            }

            MeshAppendVertex(vc,*v[i],normal,uv,uv2);
        }
    }
}

//-----------------------------------------------------------
//  called once per frame, to update slimes ( and maybe other stuffs )

void Q2BSP_Tick(Model *mdl,Camera *cam)
{
    Q2BSP_info_t *bsp=(Q2BSP_info_t *)mdl->bones;

    float screenSize[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };
    SetShaderValue(bsp->Warp.shader, GetShaderLocation(bsp->Warp.shader, "size"), &screenSize, SHADER_UNIFORM_VEC2);
    bsp->Warp.time += GetFrameTime();
    SetShaderValue(bsp->Warp.shader, bsp->Warp.secondsLoc, &bsp->Warp.time, SHADER_UNIFORM_FLOAT);
}

//-----------------------------------------------------------
//  get the entity string from the loaded data

char *Q2BSP_GetEntityString(Model *mdl)
{
    Q2BSP_info_t *bsp=(Q2BSP_info_t *)mdl->bones;
    return (bsp->EntityString);
}

//-----------------------------------------------------------
//  process q2bsp from ram

Model Q2BSP_LoadFromMemory(uint8_t *q2_raw)
{
    Q2BSP_info_t *bsp = RL_CALLOC(1,sizeof(Q2BSP_info_t));

    Model mdl = {0};
    bsp->model = &mdl;
    Q2BSP_header_t *header = (Q2BSP_header_t*)q2_raw;

    if ((header->magic!=0x50534249) || (header->version!=0x26))
    {
        TraceLog(LOG_INFO,"Q2BSP_LoadFromMemory:Unsupported File\n");
        return mdl;
    }

    Q2BSP_lump_t *lumps =(Q2BSP_lump_t*)&q2_raw[sizeof(Q2BSP_header_t)];

    bsp->lightMap.shader = LoadShader(LIGHTMAP_VS,LIGHTMAP_FS);
    bsp->Sky.shader = LoadShader(SKY_VS,SKY_FS);
    bsp->Warp.shader = LoadShader(NULL,WAVE_FS);
    
    bsp->Warp.secondsLoc = GetShaderLocation(bsp->Warp.shader, "seconds");
    bsp->Warp.freqXLoc = GetShaderLocation(bsp->Warp.shader, "freqX");
    bsp->Warp.freqYLoc = GetShaderLocation(bsp->Warp.shader, "freqY");
    bsp->Warp.ampXLoc = GetShaderLocation(bsp->Warp.shader, "ampX");
    bsp->Warp.ampYLoc = GetShaderLocation(bsp->Warp.shader, "ampY");
    bsp->Warp.speedXLoc = GetShaderLocation(bsp->Warp.shader, "speedX");
    bsp->Warp.speedYLoc = GetShaderLocation(bsp->Warp.shader, "speedY");

    float freqX = 10.0f;
    float freqY = 12.0f;
    float ampX = 10.0f;
    float ampY = 15.0f;
    float speedX = 12.0f;
    float speedY = 4.0f;

    SetShaderValue(bsp->Warp.shader, bsp->Warp.freqXLoc, &freqX, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bsp->Warp.shader, bsp->Warp.freqYLoc, &freqY, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bsp->Warp.shader, bsp->Warp.ampXLoc, &ampX, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bsp->Warp.shader, bsp->Warp.ampYLoc, &ampY, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bsp->Warp.shader, bsp->Warp.speedXLoc, &speedX, SHADER_UNIFORM_FLOAT);
    SetShaderValue(bsp->Warp.shader, bsp->Warp.speedYLoc, &speedY, SHADER_UNIFORM_FLOAT);

#define ParseLump(i, name, type)                       \
    bsp->name = (type *)(&q2_raw[lumps[i].offset]);\
    bsp->n##name = lumps[i].length / sizeof(type);

    ParseLump(Lump_Vertices,Vertices,Vector3);
    ParseLump(Lump_Faces, Faces, Q2BSP_face_t);
    ParseLump(Lump_Edges, Edges, Q2BSP_edge_t);
    ParseLump(Lump_Models,Models,Q2BSP_model_t);
    ParseLump(Lump_Face_Edge_Table,FaceEdges,uint32_t);
    ParseLump(Lump_Texture_Information,TexInfo,Q2BSP_texinfo_t);
    ParseLump(Lump_Lightmaps, lightMapRaw, uint8_t)
    ParseLump(Lump_Entities, EntityString, uint8_t);

    //  we need to copy this to keep it for later
    char* dupe = RL_CALLOC(1, bsp->nEntityString);
    memcpy(dupe, bsp->EntityString, bsp->nEntityString);
    bsp->EntityString = dupe;

    Q2BSP_PrepareLightmap(bsp);

    mdl.transform = MatrixIdentity();
    mdl.meshCount = bsp->nTexInfo;
    mdl.materialCount = bsp->nTexInfo;
    mdl.meshes = (Mesh*)RL_CALLOC(mdl.meshCount,sizeof(Mesh));
    mdl.materials = (Material*)RL_CALLOC(mdl.materialCount,sizeof(Material));
    mdl.meshMaterial = (int*)RL_CALLOC(mdl.materialCount,sizeof(int));
    bsp->meshVertices = (VertexContainer*)RL_CALLOC(mdl.meshCount,sizeof(VertexContainer));

    //  q2 will repeat textures 
    for (int q=0;q<bsp->nTexInfo;q++)
    {
        bsp->meshVertices[q].Flags = bsp->TexInfo[q].flags;
        mdl.meshMaterial[q] = q;
        mdl.materials[q] = LoadMaterialDefault();
        mdl.materials[q].maps[MATERIAL_MAP_METALNESS].texture = bsp->lightMap.texture;
        mdl.materials[q].maps[MATERIAL_MAP_ALBEDO].texture = Q2BSP_LoadTextureCached(bsp,bsp->TexInfo[q].texture_name);
        if ((bsp->TexInfo[q].flags&SURF_SKY3D)==0)
        {
            mdl.materials[q].shader = bsp->lightMap.shader;

            if ((bsp->TexInfo[q].flags&SURF_TRANS33)!=0)
                mdl.materials[q].maps[MATERIAL_MAP_ALBEDO].color = (Color){255,255,255,84};
            if ((bsp->TexInfo[q].flags&SURF_TRANS66)!=0)
                mdl.materials[q].maps[MATERIAL_MAP_ALBEDO].color = (Color){255,255,255,168};
        }
        else
        {
            mdl.materials[q].shader = bsp->Sky.shader;
        }
        if ((bsp->TexInfo[q].flags&SURF_WARP)!=0)
        {
            mdl.materials[q].maps[MATERIAL_MAP_ALBEDO].color = (Color){255,255,255,192};
            mdl.materials[q].shader = bsp->Warp.shader;
        }
    }

    //  some sky faces have trans, some don't 
    //  so we just add them here
    for (int f=0;f<bsp->nFaces;f++)
    {
        Q2BSP_face_t* face = &bsp->Faces[f];
        Q2BSP_texinfo_t *tinfo = &bsp->TexInfo[face->texture_info];
        if (((tinfo->flags & SURF_SKY3D) != 0))
        {
            tinfo->flags &= ~((uint32_t)SURF_TRANS | SURF_NODRAW);
        }
    }

    for (int f=0;f<bsp->nFaces;f++)
    {
        Q2BSP_face_t* face = &bsp->Faces[f];
        Q2BSP_texinfo_t *tinfo = &bsp->TexInfo[face->texture_info];
        if (((tinfo->flags & SURF_NODRAW) == 0))
            {
                Q2BSP_AddFace(bsp, face,f);
            }
    }

    //  solids
    int realMeshCount = 0;
    for (int m=0;m<mdl.meshCount;m++)
    {
        if (bsp->meshVertices[m].Count != 0)
        {
            if ((bsp->meshVertices[m].Flags&SURF_TRANS)==0)
            {
                mdl.meshes[realMeshCount].vertexCount = bsp->meshVertices[m].Count;
                mdl.meshes[realMeshCount].triangleCount = mdl.meshes[realMeshCount].vertexCount / 3;
                mdl.meshes[realMeshCount].vertices = (float*)bsp->meshVertices[m].Positions;
                mdl.meshes[realMeshCount].texcoords = (float*)bsp->meshVertices[m].TexCoords0;
                mdl.meshes[realMeshCount].texcoords2 = (float*)bsp->meshVertices[m].TexCoords1;
                mdl.meshes[realMeshCount].normals = (float*)bsp->meshVertices[m].Normals;
                UploadMesh(&mdl.meshes[realMeshCount], false);
                mdl.meshMaterial[realMeshCount] = m;
                realMeshCount++;
            }
        }
    }
    //  transparent
    for (int m=0;m<mdl.meshCount;m++)
    {
        if (bsp->meshVertices[m].Count != 0)
        {
            if ((bsp->meshVertices[m].Flags&SURF_TRANS)!=0)
            {
                mdl.meshes[realMeshCount].vertexCount = bsp->meshVertices[m].Count;
                mdl.meshes[realMeshCount].triangleCount = mdl.meshes[realMeshCount].vertexCount / 3;
                mdl.meshes[realMeshCount].vertices = (float*)bsp->meshVertices[m].Positions;
                mdl.meshes[realMeshCount].texcoords = (float*)bsp->meshVertices[m].TexCoords0;
                mdl.meshes[realMeshCount].texcoords2 = (float*)bsp->meshVertices[m].TexCoords1;
                mdl.meshes[realMeshCount].normals = (float*)bsp->meshVertices[m].Normals;
                UploadMesh(&mdl.meshes[realMeshCount], false);
                mdl.meshMaterial[realMeshCount] = m;
                realMeshCount++;
            }
        }
    }

    //  free memory
    for (int m=0;m<mdl.meshCount;m++)
    {
        if (bsp->meshVertices[m].Count != 0)
        {
            RL_FREE(bsp->meshVertices[m].Normals);
            RL_FREE(bsp->meshVertices[m].Positions);
            RL_FREE(bsp->meshVertices[m].TexCoords0);
            RL_FREE(bsp->meshVertices[m].TexCoords1);
        }
    }
    RL_FREE(bsp->meshVertices);

    mdl.meshCount = realMeshCount;
#ifdef DEBUG_LIGHTMAP    
    ExportImage(bsp->lightMap.image, "lightmap.png");
#endif
    UpdateTexture(bsp->lightMap.texture, bsp->lightMap.image.data);
    GenTextureMipmaps(&bsp->lightMap.texture);
    SetTextureFilter(bsp->lightMap.texture, TEXTURE_FILTER_BILINEAR);
    mdl.bones = (BoneInfo*)bsp;
    return (mdl);
}

Model Q2BSP_Load(const char *filename)
{
    Model m = {0};
    int q2_raw_length;
    uint8_t *q2_raw = LoadFileData(filename,&q2_raw_length);
    if (q2_raw!=NULL)
    {
        m = Q2BSP_LoadFromMemory(q2_raw);
        RL_FREE(q2_raw);
    }
    else 
    {
        TraceLog(LOG_ERROR,"File Not Found %s",filename);
    }
    return m;
}

void Q2BSP_Unload(Model mdl)
{
    Q2BSP_info_t *bsp=(Q2BSP_info_t *)mdl.bones;
    RL_FREE(bsp->EntityString);
    for (int q = 0; q < bsp->nCachedTextures; q++)
    {
        UnloadTexture(bsp->CachedTextures[q].Texture);
        free(bsp->CachedTextures[q].Name);
    }

    UnloadImage(bsp->lightMap.image);
    UnloadTexture(bsp->lightMap.texture);
    UnloadShader(bsp->lightMap.shader);
    UnloadShader(bsp->Warp.shader);
    UnloadShader(bsp->Sky.shader);

    RL_FREE(mdl.materials);
    RL_FREE(mdl.meshMaterial);
    RL_FREE(mdl.meshes);
}


#else 

Model Q2BSP_LoadFromMemory(uint8_t *q2_raw);
Model Q2BSP_Load(const char *filename);
void Q2BSP_Tick(Model *mdl,Camera *cam);
char *Q2BSP_GetEntityString(Model *mdl);

#endif


