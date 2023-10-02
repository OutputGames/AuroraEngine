#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gCombined;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

const int light_count = 10;

struct Material {
    sampler2D texture_brdf0;
    samplerCube texture_rad0;
    samplerCube texture_pref0;
    sampler2D texture_shad0;
    sampler2D texture_diffuse0;
    sampler2D texture_normal0;
    sampler2D texture_rgh0;
    sampler2D texture_ao0;
};

uniform Material material;

struct Light {
    vec3 position;
    vec3 direction;
    vec3 color;
    bool enabled;
    float power;
    mat4 lightSpaceMatrix;
};

// lights
uniform Light lights[light_count];

uniform vec3 viewPos;

vec3 getNormalFromMap()
{

    vec3 normalTex = texture(material.texture_normal0, vec2(TexCoords)).xyz;

    if (normalTex.b == 0) {
        normalTex.b = 1;
        normalTex.r = 1 - normalTex.r;
        normalTex.g = 1 - normalTex.g;
    }
    
/*
    if (_2cl == 1) {
        vec2 s = 1.0/vec2(64,64);
    
        float offset = 0.125;

        float p = _2cl;
        float h1 = digital2cl(get2cl(TexCoords+s*vec2(offset,0)));
        float v1 = digital2cl(get2cl(TexCoords+s*vec2(0,offset)));
       
   	    vec2 normal = (p - vec2(h1, v1));

        normal /= 10;
        normal += 0.5;

        normalTex = vec3(normal, 1.);
    }
*/

    vec3 tangentNormal = normalTex * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

const float PI = 3.14159;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

float ShadowCalculation(Light light)
{


    vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(WorldPos, 1.0);

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    
    float range = 0.5;
    
    projCoords = projCoords * range + range;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(material.texture_shad0, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 N = normalize(Normal);
        //vec3 L = normalize(-light.direction);
        vec3 L = normalize(light.position - WorldPos);
    float bias = max(0.05 * (1.0 - dot(N, L)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(material.texture_shad0, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(material.texture_shad0, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 10.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{		
    gPosition = vec4(WorldPos,1.0);
    gNormal = vec4(normalize(Normal),1.0);
    gAlbedoSpec = vec4(albedo, 1.0);
    gCombined = vec4(roughness, metallic, 1.0,1.0);
}  