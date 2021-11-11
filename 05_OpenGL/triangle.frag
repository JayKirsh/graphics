#version 450 core

uniform mat4 model;

uniform vec3 lightPosition;
uniform vec3 direction;
uniform vec3 lightIntensity;

in vec3 vertexPosition;
in vec3 vertexColour;
in vec3 vertexNormal;

out vec4 fragColour;

void main()
{
	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vec3 normal = normalize(normalMatrix * vertexNormal);

	vec3 surfaceToLight = normalize(lightPosition - vertexPosition);

	float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
	brightness = clamp(brightness, 0, 1);

    fragColour = vec4(brightness * lightIntensity * vertexColour, 1.0);
}
