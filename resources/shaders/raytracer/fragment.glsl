#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gCombined;

uniform sampler2D brdf;
uniform samplerCube prefilter;
uniform samplerCube irradiance;

struct Light {
    vec3 position;
    vec3 direction;
    vec3 color;
    bool enabled;
    float power;
    mat4 lightSpaceMatrix;
};
const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

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


void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
    float Roughness = texture(gCombined, TexCoords).r;
    float metallic = texture(gCombined, TexCoords).g;
    
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

        vec3 lightDir = normalize(light.position - FragPos);
        vec3 normal = normalize(N);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = diff * alb;
        // specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, normal);

        vec3 halfwayDir = normalize(lightDir + viewDir);
        if (light.enabled)
            Lo += (pow(max(dot(normal, halfwayDir), 0.0), 32.0) * light.color * light.power) * F0;
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
    vec3 ambient = (kD * diffuse + specular) * 1.0;

    ambient = vec3(0.5) * alb;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(vec3(color), 1.0);

    //FragColor = vec4(vec3(Roughness), 1.0);
}