#version 330 core
out vec4 FragColor;
struct Light {
    vec3 position;  
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
  
uniform vec3 viewPos;
uniform sampler2D texture_diffuse1;
uniform Light light;
uniform vec3 particleColor;

uniform bool isFireballActive;
uniform Light fireballLight;
void main()
{   
    FragColor = vec4(particleColor.x, particleColor.y, particleColor.z, 1.0f);
}