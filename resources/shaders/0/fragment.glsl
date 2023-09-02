#version 330 core
out vec4 FragColor;
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


const float PI = 3.141592653589793238462643383279502884197;
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


    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - WorldPos);
        vec3 R = reflect(-V, N); 

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    float shadow = 0;

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < light_count; ++i) 
    {
        Light light = lights[i];

        vec3 lightDir = normalize(-light.direction);

        // calculate per-light radiance
        vec3 L = normalize(light.position - WorldPos);
        
        //L =  lightDir;
        vec3 H = normalize(V + L);
        float distance = length(light.position- WorldPos);

        float attenuation = 1.0 / (distance * distance);

        //attenuation = 1.0;

        vec3 radiance = light.color * attenuation;

        radiance *= light.power;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
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
        
        shadow += ShadowCalculation(light);

        if (light.enabled)
            Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).

    // ambient lighting (we now use IBL as the ambient term)
    
    //sampler2D irradianceMap = material.texture_rad0;
    //sampler2D prefilterMap = material.texture_pref0;
    //sampler2D brdfLUT = material.texture_brdf0;
    
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0,roughness);
	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	
	kD *= 1.0 - metallic;	  
	  
	vec3 irradiance = texture(material.texture_rad0, N).rgb;
	vec3 diffuse    = irradiance * albedo;
	  
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(material.texture_pref0, R,  roughness * MAX_REFLECTION_LOD).rgb;   
	vec2 envBRDF  = texture(material.texture_brdf0, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);


    vec3 ambient = (kD * diffuse + specular) * ao * (1.0 - shadow);

	//ambient = vec3(specular);

    vec3 color = ambient + Lo;

    //color -= (shadow*vec3(0.25));

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 
    
    //FragColor = vec4(vec3(1.0,0,0),1.0);

    FragColor = vec4(vec3(color), 1.0);
}