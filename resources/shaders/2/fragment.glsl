#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

struct Material {
	sampler2D texture_diffuse;
};

uniform Material material;

uniform vec3 color;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	vec4 tex = texture(material.texture_diffuse, vec2(TexCoord));
	tex.rgb = 1.0 - tex.rgb;
	FragColor = vec4(color,tex.a);
}