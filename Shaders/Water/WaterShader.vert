// vertex shader
#version 410 core

layout (location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec4 clipSpace;
out vec2 textureCoords;
out vec3 toCameraVector;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 cameraPosition;

uniform float width;  // Width of the water surface
uniform float height; // Height of the water surface

const float dudvTiling = 8.0;

void main()
{
	vec4 worldPosition = model * vec4(aPos.x, aPos.y, aPos.z, 1.0);

	clipSpace = projection * view * worldPosition;
	gl_Position = clipSpace;

	// Normalize aPos to the range [-1, 1]
    float normalizedX = aPos.x / (width / 2.0);
    float normalizedY = aPos.y / (height / 2.0);

	//textureCoords = vec2(normalizedX / 2.0 + 0.5, normalizedY / 2.0 + 0.5) * dudvTiling;
	textureCoords = aTexCoord * dudvTiling;

	toCameraVector = cameraPosition - worldPosition.xyz;
}
