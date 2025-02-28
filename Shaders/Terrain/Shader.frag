#version 410 core

in float Height;
in vec2 FragTexCoord;
in vec3 WorldPos;

out vec4 FragColor;

// textures for the different height levels
uniform sampler2D textureHeight0;
uniform sampler2D textureHeight1;
uniform sampler2D textureHeight2;
uniform sampler2D textureHeight3;

// thresholds for the texture heights
uniform float tHeight0 = 64.0;
uniform float tHeight1 = 128.0;
uniform float tHeight2 = 193.0;
uniform float tHeight3 = 256.0;

vec4 CalcTexColor(vec2 texCoord)
{
	vec4 TexColor;

	// bring height values in the [0, 256] interval
	float h = (Height + 16) * 4.0;

	if(h < tHeight0)
	{
		TexColor = texture(textureHeight0, texCoord);
	}
	else if(h < tHeight1)
	{
		vec4 color0 = texture(textureHeight0, texCoord);
		vec4 color1 = texture(textureHeight1, texCoord);
		float delta = tHeight1 - tHeight0;
		float factor = (h - tHeight0) / delta;
		TexColor = mix(color0, color1, factor);
	}
	else if(h < tHeight2)
	{
		vec4 color0 = texture(textureHeight1, texCoord);
		vec4 color1 = texture(textureHeight2, texCoord);
		float delta = tHeight2 - tHeight1;
		float factor = (h - tHeight1) / delta;
		TexColor = mix(color0, color1, factor);
	}
	else if(h < tHeight3)
	{
		vec4 color0 = texture(textureHeight2, texCoord);
		vec4 color1 = texture(textureHeight3, texCoord);
		float delta = tHeight3 - tHeight2;
		float factor = (h - tHeight2) / delta;
		TexColor = mix(color0, color1, factor);
	}
	else
	{
		TexColor = texture(textureHeight3, texCoord);
	}

	return TexColor;
}

void main()
{
	//float h = (Height + 16) / 64.0f;
	//FragColor = vec4(h, h, h, 1.0);
	//vec4 color = texture(textureSampler, FragTexCoord);
	//FragColor = color;
	

	float tileFactor = 0.1;
	vec2 worldTexCoord = WorldPos.xz * tileFactor;

	vec4 TexColor = CalcTexColor(worldTexCoord);

	//if(gl_FrontFacing)
	//{
	//	FragColor = vec4(0.0, 0.0, 0.0, 0.0); // RGBA with 0 alpha
	//}
	//else
	//{
	//	FragColor = TexColor;
	//}

	FragColor = TexColor;
	
	//if (gl_FrontFacing)
	//{
	//	FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red for front faces
	//}
	//else 
	//{
	//	FragColor = vec4(0.0, 0.0, 1.0, 1.0); // Blue for back faces
	//}
}
