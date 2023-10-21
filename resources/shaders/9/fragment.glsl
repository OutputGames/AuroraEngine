#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gCombined;

uniform sampler2D brdf;
uniform samplerCube prefilter;
uniform samplerCube irradiance;
uniform sampler2DArray shadowMap;

uniform mat4 view;

uniform vec3 viewPos;
uniform float far_plane;

layout (std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};
uniform float cascadePlaneDistances[16];
uniform int cascadeCount;   // number of frusta - 1

struct Light {
    vec3 position;
    vec3 direction;
    vec3 color;
    bool enabled;
    float power;
    //mat4 lightSpaceMatrix;
};
const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];

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

float ShadowCalculation(Light light, vec3 WorldPos, vec3 Normal)
{
    // select cascade layer
    vec4 fragPosViewSpace = view * vec4(WorldPos, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (depthValue < cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = cascadeCount;
    }

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(WorldPos, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = Normal;
    float bias = max(0.05 * (1.0 - dot(normal, light.direction)), 0.005);
    const float biasModifier = 0.5f;
    if (layer == cascadeCount)
    {
        bias *= 1 / (far_plane * biasModifier);
    }
    else
    {
        bias *= 1 / (cascadePlaneDistances[layer] * biasModifier);
    }

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
        
    return shadow;
}

uniform float near_plane;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
    float Roughness = texture(gCombined, TexCoords).r;
    float metallic = texture(gCombined, TexCoords).g;

    if (texture(gAlbedo, TexCoords).a == 0) {
        //gl_FragDepth = 0;
        //discard;
    }
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 1.0; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);

    vec3 WorldPos = FragPos;

    vec3 N = Normal;
    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-V, N); 

    vec3 alb = Diffuse;
    float rgh = Roughness;

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, alb, metallic);

    float shadow = 0;

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < NR_LIGHTS; ++i) 
    {
        Light light = lights[i];

        vec3 lightDir = normalize(-light.direction);



        // calculate per-light radiance
        vec3 L = normalize(light.position - WorldPos);

        L = lightDir;
        
        //L =  lightDir;
        vec3 H = normalize(V + L);
        float distance = length(light.position- WorldPos);

        float attenuation = 1.0 / (distance * distance);

        attenuation = 1.0;

        vec3 radiance = light.color * attenuation;

        radiance *= light.power;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, rgh);   
        float G   = GeometrySmith(N, V, L, rgh);      
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);   
        
        shadow += ShadowCalculation(light, FragPos, Normal);

        if (light.enabled)
            Lo += (kD * alb / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).

    // ambient lighting (we now use IBL as the ambient term)
    
    //sampler2D irradianceMap = irradiance;
    //sampler2D prefilterMap = prefilter;
    //sampler2D brdfLUT = brdf;
    
    // ambient lighting (we now use IBL as the ambient term)

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0,rgh);
	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	
	kD *= 1.0 - metallic;	  
	
	  
	vec3 irradianceM = texture(irradiance, N).rgb;
	vec3 diffuse    = irradianceM * alb;
	  
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(prefilter, R,  rgh * MAX_REFLECTION_LOD).rgb;   
	vec2 envBRDF  = texture(brdf, vec2(max(dot(N, V), 0.0), rgh)).rg;
	vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);



    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = (kD * diffuse + specular) * 1.0 * (1.0 - shadow);

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(vec3(color), texture(gAlbedo, TexCoords).a);

    //int layer = 1;

    //float depthValue = texture(shadowMap, vec3(TexCoords, layer)).r;

    //FragColor = vec4(vec3(shadow), 1.0);
}