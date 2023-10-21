#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform mat4 offsets[200];

void main()
{
    TexCoords = aTexCoords;
    WorldPos = vec3(offsets[gl_InstanceID] * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;   

    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}