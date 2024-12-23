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

struct fLighting
{
    vec3 lightPosition;
    float ambientStrength;
    vec3 lightColor;
    float specularStrength;
    vec3 viewPosition;
};

#if VULKAN
layout(set = 1, binding = 0) uniform fUniformMatrix
#else
layout(std140, binding = 1) uniform fUniformMatrix
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
layout(std140, binding = 2) uniform fUniformMaterial
#endif
{
    fMaterial f_material[MAX_TEXTURES];
};

#if VULKAN
layout(set = 1, binding = 3) uniform fLightingMatrix
#else
layout(std140, binding = 3) uniform fLightingMatrix
#endif
{
    fLighting lightingMatrix[MAX_LIGHTS];
};

void main()
{
    vec3 resultColor = vec3(0.0);
    for (int l = 0; l < MAX_LIGHTS; ++l)
    {
        //Ambient Lighting
        vec3 ambient = lightingMatrix[l].ambientStrength * lightingMatrix[l].lightColor;

        //Diffuse Lighting
        vec3 normal = normalize(i_normal);
        vec3 light_direction = normalize(lightingMatrix[l].lightPosition - i_fragment_position);

        float diff = max(dot(normal, light_direction), 0.0);
        vec3 diffuse = diff * lightingMatrix[l].lightColor;

        //Specular Lighting
        vec3 specular = vec3(0.0);
        if (diff > 0.0)
        {
            vec3 view_direction = normalize(lightingMatrix[l].viewPosition - i_fragment_position);
            // vec3 reflect_direction = reflect(-light_direction, normal);
            vec3 halfway_vector = normalize(view_direction + light_direction);

            // float spec = pow(max(dot(view_direction, reflect_direction), 0.0), f_material[i_index].shininess);
            float spec = pow(max(dot(halfway_vector, normal), 0.0), f_material[i_object_index].shininess);
            specular = lightingMatrix[l].specularStrength * spec * lightingMatrix[l].lightColor * f_material[i_object_index].specularColor;
        }

        resultColor += ambient + diffuse + specular;
    }

    fragmentColor = vec4(resultColor, 1.0) * (i_col + 0.5);
}