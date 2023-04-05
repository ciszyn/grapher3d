#version 330 core

in vec3 normal;
in vec3 fragPos;

// Ouput data
out vec4 color;

uniform vec3 lightPos;

void main()
{
	vec3 lightColor = vec3(1, 1, 1);
	vec3 ambient = vec3(0.2, 0.2, 0.2);
	vec3 objectColor = vec3(1, 1, 1);



	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPos - fragPos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 result = (ambient + diffuse) * objectColor;

	color = vec4(result, 1.0);
}