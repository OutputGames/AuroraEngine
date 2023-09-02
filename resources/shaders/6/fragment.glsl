#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

vec3 GetEnvironmentLight(vec3 p)
{
			
	vec4 GroundColour = vec4(0.6);
	vec4 SkyColourHorizon=vec4(1);
	vec4 SkyColourZenith=vec4(0.65,0.85,1,1);
	float SunFocus=1.0;
	float SunIntensity=0.1;
	vec3 SunPos=vec3(20,-19,-4);
						
	float skyGradientT = pow(smoothstep(0, 0.4, p.y), 0.35);
	float groundToSkyT = smoothstep(-0.01, 0, p.y);
	vec3 skyGradient = mix(vec3(SkyColourHorizon), vec3(SkyColourZenith), skyGradientT);
	float sun = pow(max(0, dot(p, SunPos)), SunFocus) * SunIntensity;
	// Combine ground, sky, and sun
	vec3 composite = mix(vec3(GroundColour), skyGradient, groundToSkyT) + sun * float(groundToSkyT>=1);
	return composite;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    //vec3 color = GetEnvironmentLight(WorldPos);

    FragColor = vec4(color, 1.0);
}