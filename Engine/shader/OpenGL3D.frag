#version 460

#define MAX_TEXTURES 20

layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec4 i_col;
layout(location = 2) in flat int i_index;
//Lighting
layout(location = 3) in vec3 i_light_position;
layout(location = 4) in vec3 i_light_color;
layout(location = 5) in float i_ambient_strength;
layout(location = 6) in vec3 i_normal;
layout(location = 7) in vec3 i_fragment_position;
layout(location = 8) in vec3 i_view_position;
layout(location = 9) in float i_specular_strength;
layout(location = 10) in float i_is_lighting;

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

layout(std140, binding = 1) uniform fUniformMatrix
{
    fMatrix f_matrix[MAX_TEXTURES];
};

uniform sampler2D tex[MAX_TEXTURES];

layout(std140, binding = 2) uniform fUniformMaterial
{
    fMaterial f_material[MAX_TEXTURES];
};

void main()
{
    if (abs(i_is_lighting) > 0.001)
    {
        //Ambient Lighting
        vec3 ambient = i_ambient_strength * i_light_color;

        //Diffuse Lighting
        vec3 normal = normalize(i_normal);
        vec3 light_direction = normalize(i_light_position - i_fragment_position);

        float diff = max(dot(normal, light_direction), 0.0);
        vec3 diffuse = diff * i_light_color;

        //Specular Lighting
        vec3 specular = vec3(0.0);
        if (diff > 0.0)
        {
            vec3 view_direction = normalize(i_view_position - i_fragment_position);
            // vec3 reflect_direction = reflect(-light_direction, normal);
            vec3 halfway_vector = normalize(view_direction + light_direction);

            // float spec = pow(max(dot(view_direction, reflect_direction), 0.0), f_material[i_index].shininess);
            float spec = pow(max(dot(halfway_vector, normal), 0.0), f_material[i_index].shininess);
            specular = i_specular_strength * spec * i_light_color * f_material[i_index].specularColor;
        }

        fragmentColor = vec4(ambient + diffuse + specular, 1.0) * (i_col + 0.5);
    }
    else
        fragmentColor = i_col;
}