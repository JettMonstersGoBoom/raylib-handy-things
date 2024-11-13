#version 330

// Input vertex attributes
in vec3 vertexPosition;

// Input uniform values
uniform mat4 mvp;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
void main()
{
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}