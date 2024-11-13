#version 330 core
layout (location = 0) out vec4 finalColor;

in vec2 fragTexCoord;
in vec2 fragTexCoord2;
in vec4 fragColor;
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform vec4 colDiffuse;

void main()
{
    vec4 texelColor = texture2D(texture0,fragTexCoord);
    vec4 lightmapColor = texture2D(texture1,fragTexCoord2) * 3.5;
    //  gamma 
    finalColor = texelColor * (lightmapColor) * colDiffuse;

    finalColor.a = colDiffuse.a * texelColor.a;
}

