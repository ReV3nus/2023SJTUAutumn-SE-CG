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
uniform int type;

uniform bool isFireballActive;
uniform Light fireballLight;

vec3 color_wall0 = vec3(0.05859375f, 0.734375f, 0.97265625f);
vec3 color_wall1 = vec3(0.04296875f, 0.90625f, 0.50390625f);
vec3 color_wall2 = vec3(0.93359375f, 0.33984375f, 0.46484375f);
vec3 color_wall3 = vec3(0.8203125f, 0.8515625f, 0.8828125f);
vec3 color_tumbler = vec3(0.9f, 0.05f, 0.05f);
vec3 color_default = vec3(1.0f, 1.0f, 1.0f);

vec3 calcFire(){
    if(!isFireballActive)return vec3(0.0f, 0.0f, 0.0f);
    // ambient
    vec3 ambient = fireballLight.ambient * texture(texture_diffuse1, TexCoords).rgb;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(fireballLight.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = fireballLight.diffuse * diff * texture(texture_diffuse1, TexCoords).rgb;  
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = fireballLight.specular * spec * texture(texture_diffuse1, TexCoords).rgb;  
    
    // attenuation
    float distance    = length(fireballLight.position - FragPos);
    float attenuation = 1.0 / (fireballLight.constant + fireballLight.linear * distance + fireballLight.quadratic * (distance * distance));    

    ambient  *= attenuation;  
    diffuse   *= attenuation;
    specular *= attenuation;   
        
    vec3 result = ambient + diffuse;
    return result;
}
void main()
{
    vec3 color_ground = texture(texture_diffuse1, TexCoords).rgb;

    vec3 color;
    if(type == 0) color = color_default;
    if(type == 1) color = color_wall0;
    if(type == 2) color = color_wall1;
    if(type == 3) color = color_wall2;
    if(type == 4) color = color_wall3;
    if(type == 5) color = color_ground;
    if(type == 6) color = color_tumbler;

    // ambient
    vec3 ambient = light.ambient * color;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * color;  
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular * spec * color;  
    
    // attenuation
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    ambient  *= attenuation;  
    diffuse   *= attenuation;
     specular *= attenuation;   
        
    vec3 result = ambient + diffuse + specular;
    vec3 fireResult = calcFire();
    FragColor = vec4(result + fireResult, 1.0);
} 