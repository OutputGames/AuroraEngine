#version 330 core
layout (location = 0) in vec3 INPUT_POSITION;
layout (location = 1) in vec3 INPUT_NORMAL;
layout (location = 2) in vec2 INPUT_TEXCOORDS;
layout(location = 3) in ivec4 INPUT_BONEIDS; 
layout(location = 4) in vec4 INPUT_WEIGHTS;

out vec2 OUTPUT_TEXCOORDS;
out vec3 OUTPUT_POS;
out vec3 OUTPUT_NORMAL;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// placeuniformshere

const int MAX_BONES = 252;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

vec3 SAMPLE_POSITION(vec3 p) {

    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(INPUT_BONEIDS[i] == -1) 
            continue;
        if(INPUT_BONEIDS[i] >=MAX_BONES) 
        {
            totalPosition = vec4(p,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[INPUT_BONEIDS[i]] * vec4(p,1.0f);
        totalPosition += localPosition * INPUT_WEIGHTS[i];
        vec3 localNormal = mat3(finalBonesMatrices[INPUT_BONEIDS[i]]) * INPUT_NORMAL;
   }

    if (totalPosition == vec4(0.0))
        totalPosition = vec4(p,1.0);

    return vec3(model * totalPosition);
}

vec3 SAMPLE_NORMAL(vec3 n) {
    return mat3(transpose(inverse(model))) * n; 
}

vec4 TRANSFORMTOVIEW(vec3 p) {
    return projection * view * vec4(p, 1.0);
}
