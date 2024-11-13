#version 330 core
layout (location = 0) out vec4 finalColor;

in vec3 fragPosition;

vec4 skyTop=vec4(0.8,0.95,1,1);
vec4 skyBot=vec4(0.0,0.1,0.2,1);

void main()
{
    //  just color ramp 
    finalColor = mix(skyTop,skyBot,(gl_FragCoord.y/1024.0)+0.5f);
}

