#version 330 core

#define MAX_TEXTURES 4

struct Material {
    sampler2D diffuse[MAX_TEXTURES];
    sampler2D specular[MAX_TEXTURES];
    float shininess;
};
uniform Material material;
uniform int diffuseCount;
uniform int specularCount;


struct DirLight{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);

struct PointLight{

    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

vec3 calcPointDir(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
uniform SpotLight spotLight;
uniform bool isSpotLightOn;

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

out vec4 FragColor;

uniform vec3 lightingColor;

uniform vec3 viewPos;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

void main(){
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = calcDirLight(dirLight, norm, viewDir);

    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        result += calcPointDir(pointLights[i], norm, FragPos, viewDir);
    }

    if(isSpotLightOn == true){
        result += calcSpotLight(spotLight, norm, FragPos, viewDir);
    }

    FragColor = vec4(result, 1.0);
}


vec3 getDiffuseColor(){
    vec3 color = vec3(0.0);
    for(int i = 0; i < MAX_TEXTURES; i++){
        color += texture(material.diffuse[i], TexCoords).rgb;
    }
    //average blend
    return color / max(float(diffuseCount), 1.0);
}


vec3 getSpecularColor(){
    vec3 color = vec3(0.0);
    for(int i = 0; i < MAX_TEXTURES; i++){
        color += texture(material.specular[i], TexCoords).rgb;
    }
    //average blend
    return color / max(float(specularCount), 1.0);
}


vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
    vec3 lightDir = normalize(light.direction - fragPos);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float diff = max(dot(normal, lightDir), 0.0);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * getDiffuseColor();
    vec3 diffuse = diff * light.diffuse * getDiffuseColor();
    vec3 specular = getSpecularColor() * spec * light.specular;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    diffuse *= intensity;
    specular *= intensity;

    return (ambient + diffuse + specular);
}


vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir){
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * getDiffuseColor();
    vec3 diffuse = diff * light.diffuse * getDiffuseColor();
    vec3 specular = getSpecularColor() * spec * light.specular;

    return (ambient + diffuse + specular);

}


vec3 calcPointDir(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){

    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * getDiffuseColor();
    vec3 diffuse = diff * light.diffuse * getDiffuseColor();
    vec3 specular = getSpecularColor() * spec * light.specular;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);

}
