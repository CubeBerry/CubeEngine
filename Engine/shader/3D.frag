#version 460
#if VULKAN
#extension GL_EXT_nonuniform_qualifier : enable
#endif

#define MAX_TEXTURES 500
#define MAX_LIGHTS 10

layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec4 i_col;
layout(location = 2) in flat int i_object_index;
//Lighting
layout(location = 6) in vec3 i_normal;
layout(location = 7) in vec3 i_fragment_position;

layout(location = 0) out vec4 fragmentColor;

struct fMatrix
{
    int texIndex;
};

struct fMaterial
{
    vec3 specularColor;
    float shininess;
};

struct fDirectionalLight
{
    vec3 lightDirection;
    float ambientStrength;
    vec3 lightColor;
    float specularStrength;
    vec3 viewPosition;
};

struct fPointLight
{
    vec3 lightPosition;
    float ambientStrength;
    vec3 lightColor;
    float specularStrength;
    vec3 viewPosition;

    float constant;
    float linear;
    float quadratic;
};

#if VULKAN
layout(set = 1, binding = 0) uniform fUniformMatrix
#else
layout(std140, binding = 3) uniform fUniformMatrix
#endif
{
    fMatrix f_matrix[MAX_TEXTURES];
};

#if VULKAN
layout(set = 1, binding = 1) uniform sampler2D tex[MAX_TEXTURES];
#else
uniform sampler2D tex[MAX_TEXTURES];
#endif

#if VULKAN
layout(set = 1, binding = 2) uniform fUniformMaterial
#else
layout(std140, binding = 4) uniform fUniformMaterial
#endif
{
    fMaterial f_material[MAX_TEXTURES];
};

#if VULKAN
layout(set = 1, binding = 3) uniform fDirectionalLightList
#else
layout(std140, binding = 5) uniform fDirectionalLightList
#endif
{
    fDirectionalLight directionalLightList[MAX_LIGHTS];
};

#if VULKAN
layout(set = 1, binding = 4) uniform fPointLightList
#else
layout(std140, binding = 6) uniform fPointLightList
#endif
{
    fPointLight pointLightList[MAX_LIGHTS];
};

#if VULKAN
layout(push_constant) uniform ActiveLights
{
    int activePointLights;
    int activeDirectionalLights;
} activeLights;
#else
uniform int activePointLights;
uniform int activeDirectionalLights;
#endif

// vec3 BlinnPhong(fLighting current, int i)
// {
//     //Ambient Lighting
//     vec3 ambient = current.ambientStrength * current.lightColor;

//     //Diffuse Lighting
//     vec3 normal = normalize(i_normal);
//     vec3 light_direction = normalize(current.lightPosition - i_fragment_position);

//     float diff = max(dot(normal, light_direction), 0.0);
//     vec3 diffuse = diff * current.lightColor;

//     //Specular Lighting
//     vec3 specular = vec3(0.0);
//     if (diff > 0.0)
//     {
//         vec3 view_direction = normalize(current.viewPosition - i_fragment_position);
//         // vec3 reflect_direction = reflect(-light_direction, normal);
//         vec3 halfway_vector = normalize(view_direction + light_direction);

//         // float spec = pow(max(dot(view_direction, reflect_direction), 0.0), f_material[i_index].shininess);
//         float spec = pow(max(dot(halfway_vector, normal), 0.0), f_material[i_object_index].shininess);
//         specular = current.specularStrength * spec * current.lightColor * f_material[i_object_index].specularColor;
//     }

//     return vec3(ambient + diffuse + specular);
// }

vec3 BlinnPhong(vec3 lightPosition, vec3 lightColor, vec3 viewPosition, float ambientStrength, float specularStrength, bool isPointLight, int lightIndex)
{
    vec3 light_direction = vec3(0.0);
    float attenuation = 0.0;
    if (isPointLight)
    {
        light_direction = normalize(lightPosition - i_fragment_position);
        float distance = length(lightPosition - i_fragment_position);
        attenuation = 1.0 / (pointLightList[lightIndex].constant + pointLightList[lightIndex].linear * distance + pointLightList[lightIndex].quadratic * (distance * distance));
    }
    else
        light_direction = normalize(-lightPosition);

    //Ambient Lighting
    vec3 ambient = ambientStrength * lightColor;

    //Diffuse Lighting
    vec3 normal = normalize(i_normal);
    float diff = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = diff * lightColor;

    //Specular Lighting
    vec3 specular = vec3(0.0);
    if (diff > 0.0)
    {
        vec3 view_direction = normalize(viewPosition - i_fragment_position);
        // vec3 reflect_direction = reflect(-light_direction, normal);
        vec3 halfway_vector = normalize(view_direction + light_direction);

        // float spec = pow(max(dot(view_direction, reflect_direction), 0.0), f_material[i_index].shininess);
        float spec = pow(max(dot(halfway_vector, normal), 0.0), f_material[i_object_index].shininess);
        specular = specularStrength * spec * lightColor * f_material[i_object_index].specularColor;
    }

    if (isPointLight)
    {
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
    }

    return ambient + diffuse + specular;
}

void main()
{
    vec3 resultColor = vec3(0.0);

    //Calculate Directional Lights
#if VULKAN
    for (int l = 0; l < activeLights.activeDirectionalLights; ++l)
#else
    for (int l = 0; l < activeDirectionalLights; ++l)
#endif
    {
        fDirectionalLight currentLight = directionalLightList[l];
        resultColor += clamp(BlinnPhong(currentLight.lightDirection, currentLight.lightColor, currentLight.viewPosition, currentLight.ambientStrength, currentLight.specularStrength, false, l), 0.0, 1.0);
    }

    //Calculate Point Lights
#if VULKAN
    for (int l = 0; l < activeLights.activePointLights; ++l)
#else
    for (int l = 0; l < activePointLights; ++l)
#endif
    {
        fPointLight currentLight = pointLightList[l];
        resultColor += clamp(BlinnPhong(currentLight.lightPosition, currentLight.lightColor, currentLight.viewPosition, currentLight.ambientStrength, currentLight.specularStrength, true, l), 0.0, 1.0);
    }

    fragmentColor = vec4(resultColor, 1.0) * (i_col + 0.5);
}