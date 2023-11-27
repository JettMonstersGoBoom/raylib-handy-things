
#include "raylib.h"

typedef struct 
{
    int     ReferenceCount;
    char    *Name;
    Model   ModelResource;
} Resource_t;

typedef struct Node_t
{
    char            *Name;
    int             ResourceIndex;
    Matrix          localMatrix;
    Matrix          worldMatrix;
    struct Node_t   *Parent;

} Node_t;

typedef struct 
{
    int         nResources;
    int         nNodes;
    Resource_t  *Resources;
    Node_t      *Nodes;
} Scene_T;

//  search backwards from the current Node. 
//  parent is usually not far up the chain 
Node_t *FindNode(Scene_T *scene, Node_t *cNode, const char *parent)
{
    while(cNode!=&scene->Nodes[0])
    {
        cNode--;
        if (cNode->Name!=NULL)
        {
            if (stricmp(cNode->Name,parent)==0)
            {
                return cNode;
            }
        }
    }
    return NULL;
}

void UnLoadAscii3D(Scene_T *scene)
{
    for (int q=0;q<scene->nNodes;q++)
    {
        Node_t *node = &scene->Nodes[q];
        RL_FREE(node->Name);
    }
    for (int q=0;q<scene->nResources;q++)
    {
        UnloadModel(scene->Resources[q].ModelResource);
        RL_FREE(scene->Resources[q].Name);
    }
    RL_FREE(scene->Nodes);
    RL_FREE(scene->Resources);
    RL_FREE(scene);
}


Scene_T *LoadAscii3D(const char *fname)
{
    char linebuffer[128];

    FILE *fp = fopen(fname,"rt");

    Scene_T *scene = RL_CALLOC(1,sizeof(Scene_T));

    fscanf(fp,"resources:%d\n",&scene->nResources);
    printf("%d\n",scene->nResources);
    scene->Resources = RL_CALLOC(scene->nResources,sizeof(Resource_t));

    for (int q=0;q<scene->nResources;q++)
    {
        fgets(linebuffer,sizeof(linebuffer),fp);
        linebuffer[strlen(linebuffer)-1]=0;
        scene->Resources[q].ReferenceCount = 0;
        scene->Resources[q].ModelResource = LoadModel(linebuffer);
        scene->Resources[q].Name = strdup(linebuffer);
    }

    fscanf(fp,"nodes:%d\n",&scene->nNodes);
    scene->Nodes = RL_CALLOC(scene->nNodes,sizeof(Node_t));

    for (int q=0;q<scene->nNodes;q++)
    {
        Node_t *cNode = &scene->Nodes[q];

        fgets(linebuffer,sizeof(linebuffer),fp);
        linebuffer[strlen(linebuffer)-1]=0;

        char *ClassName = strtok(linebuffer,",");
        char *NodeName =  strtok(NULL,",");
        char *ParentName =  strtok(NULL,",");
        int ResourceID = atoi(strtok(NULL,","));

        cNode->localMatrix = MatrixIdentity();

        cNode->localMatrix.m0 = strtod(strtok(NULL,","),NULL);
        cNode->localMatrix.m1 = strtod(strtok(NULL,","),NULL);
        cNode->localMatrix.m2 = strtod(strtok(NULL,","),NULL);

        cNode->localMatrix.m4 = strtod(strtok(NULL,","),NULL);
        cNode->localMatrix.m5 = strtod(strtok(NULL,","),NULL);
        cNode->localMatrix.m6 = strtod(strtok(NULL,","),NULL);

        cNode->localMatrix.m8 =  strtod(strtok(NULL,","),NULL);
        cNode->localMatrix.m9 =  strtod(strtok(NULL,","),NULL);
        cNode->localMatrix.m10 = strtod(strtok(NULL,","),NULL);

        cNode->localMatrix.m12 = strtod(strtok(NULL,","),NULL);
        cNode->localMatrix.m13 = strtod(strtok(NULL,","),NULL);
        cNode->localMatrix.m14 = strtod(strtok(NULL,","),NULL);

        cNode->ResourceIndex = ResourceID;
        cNode->Name = strdup(NodeName);
        cNode->Parent = FindNode(scene,cNode,ParentName);

        if (stricmp(ClassName,"OmniLight3D")==0)
        {
            char *nxt = strtok(NULL,",");
            // uint32_t = (uint32_t)strtol(&nxt[1], NULL, 16);
            // nxt is $RRGGBBAA hex value 
        }
    }
    return scene;
}

//  we need to draw a mesh with a direct matrix 
//  instead of trashing the ModelResource matrix every drawcall
void DrawModelMatrix(Model ModelResource, Matrix matTransform, Color tint)
{
    for (int i = 0; i < ModelResource.meshCount; i++)
    {
        DrawMesh(ModelResource.meshes[i], ModelResource.materials[ModelResource.meshMaterial[i]], matTransform);
    }
}

void DrawAscii3D(Scene_T *scene)
{
    for (int q=0;q<scene->nNodes;q++)
    {
        Node_t *node = &scene->Nodes[q];
        Matrix m = node->localMatrix;
        if (node->Parent!=NULL)
        {
            m = MatrixMultiply(m,node->Parent->worldMatrix);
        }
        node->worldMatrix = m;
        if (node->ResourceIndex!=-1)
        {
            DrawModelMatrix(scene->Resources[scene->Nodes[q].ResourceIndex].ModelResource,m,WHITE);
        }
    }
}
