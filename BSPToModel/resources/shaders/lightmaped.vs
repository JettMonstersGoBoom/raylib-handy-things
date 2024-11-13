#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec2 vertexTexCoord2;
in vec3 vertexNormal;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec2 fragTexCoord2;

void main()
{
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 0.0)));
	fragTexCoord = vertexTexCoord;
	fragTexCoord2 = vertexTexCoord2;
    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}