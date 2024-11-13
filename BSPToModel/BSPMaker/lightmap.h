#ifndef _LIGHTMAP_H_ 
#define _LIGHTMAP_H_

//  LIGHTMAP CODE
static int32_t lightmap_allocation[LIGHTMAP_WIDTH];
bool LightmapAllocBlock(int32_t Width, int32_t Height, int32_t* X, int32_t* Y)
{
    int32_t Best = LIGHTMAP_HEIGHT;

    for (int32_t I = 0; I < LIGHTMAP_WIDTH - Width; I++)
    {
        int32_t Best2 = 0;
        int32_t J;
        for (J = 0; J < Width; J++)
        {
            if (lightmap_allocation[I + J] >= Best) break;
            if (lightmap_allocation[I + J] > Best2) Best2 = lightmap_allocation[I + J];
        }
        if (J == Width)
        {
            *X = I;
            *Y = Best = Best2;
        }
    }

    if (Best + Height > LIGHTMAP_HEIGHT)
    {
        TraceLog(LOG_ERROR, "Lightmap FULL!");
        return false;
    }

    for (int I = 0; I < Width; I++)
    {
        lightmap_allocation[*X + I] = Best + Height;
    }
#ifdef DEBUG_LIGHTMAP
    TraceLog(LOG_INFO,"Lmap Alloced %d %d at %d %d",Width,Height,*X,*Y);
#endif    
    return true;
}

#else
bool LightmapAllocBlock(int32_t Width, int32_t Height, int32_t* X, int32_t* Y);
#endif
