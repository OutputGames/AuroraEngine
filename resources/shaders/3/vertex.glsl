#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;


out vec3 vpos;
out vec2 TexCoords;

void main(){

    gl_Position.xyz = vertexPosition_modelspace;
    gl_Position.w = 1.0;
    vpos = vertexPosition_modelspace;
    TexCoords = (vertexPosition_modelspace.xy+vec2(1,1))/2.0;;
}