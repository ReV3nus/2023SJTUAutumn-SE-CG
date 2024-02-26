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

in vec2 TexCoords;
in vec3 Normal;  
in vec3 FragPos;  

//uniform Material material;
uniform Light light;
uniform vec3 objectColor;
uniform vec3 viewPos; 

uniform bool isFireballActive;
uniform Light fireballLight;

vec3 calcFire(){
    if(!isFireballActive)return vec3(0.0f, 0.0f, 0.0f);
    // ambient
    vec3 ambient = fireballLight.ambient * objectColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(fireballLight.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = fireballLight.diffuse * diff * objectColor;  
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = fireballLight.specular * spec * objectColor;  
    
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
    // ambient
    vec3 ambient = light.ambient * objectColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * objectColor;  
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular * spec * objectColor;  
    
    // attenuation
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    ambient  *= attenuation;  
    diffuse   *= attenuation;
    specular *= attenuation;   
        
    vec3 result = ambient + diffuse;

    vec3 fireResult = calcFire();
    FragColor = vec4(result + fireResult, 1.0);
}