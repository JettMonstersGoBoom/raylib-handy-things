
#version 100

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec4 vertexColor;
attribute vec4 vertexBoneIds;
attribute vec4 vertexBoneWeights;

uniform float fframeA;
uniform float fframeB;
uniform vec2 texSize;

uniform mat4 mvp;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D AnimationTexture;

mat4 boneMatrices[4];
int boneOffsets[4];

//#define SMOOTH_BLEND

vec4 texelFetch(sampler2D tex, ivec2 pixelCoord,int lod)
{
  vec2 uv = vec2(float(pixelCoord.x) + 0.5,float(pixelCoord.y)+0.5) / texSize;
  return texture2D(tex, uv);
} 

void main()
{
    boneOffsets[0] = int(vertexBoneIds.x) * 4;
    boneOffsets[1] = int(vertexBoneIds.y) * 4;
    boneOffsets[2] = int(vertexBoneIds.z) * 4;
    boneOffsets[3] = int(vertexBoneIds.w) * 4;

    int frameA = int(fframeA);
    int frameB = int(fframeB);     
    //  get mix ratio
    float fmix = mod(fframeA,1.0);   

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
#ifdef SMOOTH_BLEND
            boneMatrices[i][j] = mix(texelFetch(AnimationTexture,ivec2(boneOffsets[i]+j,frameA),0),texelFetch(AnimationTexture,ivec2(boneOffsets[i]+j,frameB),0),fmix);
#else 
        //  unlerped
        boneMatrices[i][j] = texelFetch(AnimationTexture,ivec2(boneOffsets[i]+0,frameA),0);
#endif        
        }
        boneMatrices[i][0][3] = 0.0;
        boneMatrices[i][1][3] = 0.0;
        boneMatrices[i][2][3] = 0.0;
        boneMatrices[i][3][3] = 1.0;
    }
    vec4 skinnedPosition =  vertexBoneWeights.x * (boneMatrices[0] * vec4(vertexPosition, 1.0)) +
                            vertexBoneWeights.y * (boneMatrices[1] * vec4(vertexPosition, 1.0)) + 
                            vertexBoneWeights.z * (boneMatrices[2] * vec4(vertexPosition, 1.0)) + 
                            vertexBoneWeights.w * (boneMatrices[3] * vec4(vertexPosition, 1.0));
    
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    gl_Position = mvp * skinnedPosition;
}
