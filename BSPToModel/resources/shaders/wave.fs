#version 330
// Output fragment color
layout (location = 0) out vec4 finalColor;

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec2 fragTexCoord2;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform vec4 colDiffuse;


uniform float seconds;

uniform vec2 size;

uniform float freqX;
uniform float freqY;
uniform float ampX;
uniform float ampY;
uniform float speedX;
uniform float speedY;

void main() {
    float pixelWidth = 1.0 / size.x;
    float pixelHeight = 1.0 / size.y;
    float aspect = pixelHeight / pixelWidth;
    float boxLeft = 0.0;
    float boxTop = 0.0;

    vec2 p = fragTexCoord;
    p.x += cos((fragTexCoord.y - boxTop) * freqX / ( pixelWidth * 1024.0) + (seconds * speedX)) * ampX * pixelWidth;
    p.y += sin((fragTexCoord.x - boxLeft) * freqY * aspect / ( pixelHeight * 1024.0) + (seconds * speedY)) * ampY * pixelHeight;

    vec4 texelColor =texture(texture0, p);
    vec4 lightmapColor = texture2D(texture1,fragTexCoord2);
    finalColor = texelColor * (lightmapColor*2) * colDiffuse;
    finalColor.a = colDiffuse.a * texelColor.a;
}
