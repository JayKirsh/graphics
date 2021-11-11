#version 450 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec3 normal;

out vec3 vertexPosition;
out vec3 vertexColour;
out vec3 vertexNormal;

void main()
{
	vertexPosition = position;
	vertexColour = colour;
	vertexNormal = normal;

    gl_Position = projection * view * model * vec4(position, 1.0);
}
