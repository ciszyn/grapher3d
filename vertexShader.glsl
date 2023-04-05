#version 330 core

layout(location = 0) in vec3 apos;
layout(location = 1) in vec3 anormal;

out vec3 normal;
out vec3 fragPos;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;

void main()
{
	gl_Position =  MVP * vec4(apos, 1.0);
	fragPos = vec3(M * vec4(apos, 1.0));
	normal = mat3(transpose(inverse(M))) * anormal;
}
