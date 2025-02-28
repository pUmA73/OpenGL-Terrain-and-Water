// Fragment shader
#version 410 core

in vec4 clipSpace;
in vec2 textureCoords;
in vec3 toCameraVector;

out vec4 FragColor;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;

uniform float moveFactor;

const float waveDistortionStrength = 0.02;

void main()
{
	vec2 normalizedDeviceSpace = (clipSpace.xy / clipSpace.w) / 2.0 + 0.5;
	vec2 refractTexCoords = vec2(normalizedDeviceSpace.x, normalizedDeviceSpace.y);
	vec2 reflectTexCoords = vec2(normalizedDeviceSpace.x, -normalizedDeviceSpace.y);

	vec2 distortion1 = texture(dudvMap, vec2(textureCoords.x + moveFactor, textureCoords.y)).rg * 2.0 - 1.0;
	distortion1 *= waveDistortionStrength;

	vec2 distortion2 = texture(dudvMap, vec2(-textureCoords.x + moveFactor, textureCoords.y + moveFactor)).rg * 2.0 - 1.0;
	distortion2 *= waveDistortionStrength;

	vec2 totalDistortion = distortion1 + distortion2;

	reflectTexCoords += totalDistortion;
	reflectTexCoords.x = clamp(reflectTexCoords.x, 0.001, 0.999);
	reflectTexCoords.y = clamp(reflectTexCoords.y, -0.999, -0.001);

	refractTexCoords += totalDistortion;
	refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);

	vec4 reflectColor = texture(reflectionTexture, reflectTexCoords);
	vec4 refractColor = texture(refractionTexture, refractTexCoords);

	vec3 viewVector = normalize(toCameraVector);
	float refractiveFactor = dot(viewVector, vec3(0.0, 1.0, 0.0));
	refractiveFactor = pow(refractiveFactor, 1.0);

	vec4 nearFinal = mix(reflectColor, refractColor, refractiveFactor);
	FragColor = mix(nearFinal, vec4(0.0, 0.3, 0.5, 1.0), 0.1);
	//FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
