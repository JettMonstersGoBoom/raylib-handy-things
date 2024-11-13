//  
//  simple growing vertex container 
//  

#ifndef BUILDER_SLOTSIZE
    #define BUILDER_SLOTSIZE 2048
#endif

typedef struct 
{
    uint32_t    Count;
    uint32_t    Capacity;
    uint32_t    Flags;
    Vector3     *Positions;
    Vector3     *Normals;
    Vector2     *TexCoords0;
    Vector2     *TexCoords1;
} VertexContainer;

static void *memory_realloc(void *source,int size_in_bytes,int size_out_bytes)
{
    uint8_t *ptr = RL_CALLOC(1,size_out_bytes);
    if (ptr==NULL)
    {
        TraceLog(LOG_ERROR,"Malloc of %d bytes failed",size_out_bytes);
        exit(0);
    }
    if (source!=NULL)
    {
        memcpy(ptr,source,size_in_bytes);
        RL_FREE(source);
    }
    return ptr;
}

static uint32_t MeshAppendVertex(VertexContainer *vc, Vector3 position,Vector3 normal,Vector2 uv0,Vector2 uv1)
{
    if (vc->Count >= vc->Capacity)
    {
        int oCapacity = vc->Capacity;
        if (vc->Capacity == 0)
            vc->Capacity = BUILDER_SLOTSIZE;
        else
        {
            vc->Capacity += BUILDER_SLOTSIZE;
            TraceLog(LOG_INFO,"MeshAppendVertex resize %d to %d",vc->Count,vc->Capacity);
        }
        vc->Positions =     (Vector3*)memory_realloc(vc->Positions, oCapacity * sizeof(Vector3),    vc->Capacity * sizeof(Vector3));
        vc->Normals =       (Vector3*)memory_realloc(vc->Normals,   oCapacity * sizeof(Vector3),    vc->Capacity * sizeof(Vector3));
        vc->TexCoords0 =    (Vector2*)memory_realloc(vc->TexCoords0,oCapacity * sizeof(Vector2),    vc->Capacity * sizeof(Vector2));
        vc->TexCoords1 =    (Vector2*)memory_realloc(vc->TexCoords1,oCapacity * sizeof(Vector2),    vc->Capacity * sizeof(Vector2));
    }
    uint32_t o = vc->Count;
    //  note correction for Q2 handedness
    vc->Positions[o] = (Vector3){position.x * Q2_VERTEX_SCALE,position.z * Q2_VERTEX_SCALE,-position.y * Q2_VERTEX_SCALE};
    vc->Normals[o] = (Vector3){normal.x,normal.z,-normal.y};

    vc->TexCoords0[o] = uv0;
    vc->TexCoords1[o] = uv1;
    vc->Count++;
    return o;
}


