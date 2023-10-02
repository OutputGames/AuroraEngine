#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

struct Material {
	samplerCube texture_cubemap;
};

// Crude sky colour function for background light
vec3 GetEnvironmentLight(vec3 p)
{
			
	vec4 GroundColour = vec4(0.6);
	vec4 SkyColourHorizon=vec4(1);
	vec4 SkyColourZenith=vec4(0.65,0.85,1,1);
	float SunFocus=01;
	float SunIntensity=0;
	vec3 SunPos=vec3(10,10,10);
						
	float skyGradientT = pow(smoothstep(0, 0.4, p.y), 0.35);
	float groundToSkyT = smoothstep(-0.01, 0, p.y);
	vec3 skyGradient = mix(vec3(SkyColourHorizon), vec3(SkyColourZenith), skyGradientT);
	float sun = pow(max(0, dot(p, SunPos)), SunFocus) * SunIntensity;
	// Combine ground, sky, and sun
	vec3 composite = mix(vec3(GroundColour), skyGradient, groundToSkyT) + sun * float(groundToSkyT>=1);
	return composite;
}

uniform Material material;

void main()
{    
    //FragColor = texture(material.texture_cubemap, TexCoords);
    
    FragColor = vec4(GetEnvironmentLight(TexCoords),1.0);
}