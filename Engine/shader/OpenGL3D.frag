#version 460
precision mediump float;

#define MAX_TEXTURES 20

layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec4 i_col;
//Lighting
layout(location = 2) in vec4 i_light_position;
layout(location = 3) in vec4 i_light_color;
layout(location = 4) in float i_ambient_strength;
layout(location = 5) in vec4 i_normal;
layout(location = 6) in vec4 i_fragment_position;
layout(location = 7) in vec4 i_view_position;
layout(location = 8) in float i_specular_strength;
layout(location = 9) in float i_is_lighting;

layout(location = 0) out vec4 fragmentColor;

struct fMatrix
{
    int texIndex;
};

layout(std140, binding = 2) uniform fUniformMatrix
{
    fMatrix f_matrix[MAX_TEXTURES];
};

uniform sampler2D tex[MAX_TEXTURES];

void main()
{
    //if (i_is_lighting == 1.0)
    //{
        //Ambient Lighting
        vec4 ambient = i_ambient_strength * i_light_color;

        //Diffuse Lighting
        vec4 normal = normalize(i_normal);
        vec4 light_direction = vec4(normalize(i_light_position.xyz - i_fragment_position.xyz), 1.0);

        float diff = max(dot(normal, light_direction), 0.0);
        vec4 diffuse = diff * i_light_color;

        //Specular Lighting
        vec4 view_direction = vec4(normalize(i_view_position.xyz - i_fragment_position.xyz), 1.0);
        vec4 reflect_direction = reflect(-light_direction, normal);

        float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 32);
        vec4 specular = i_specular_strength * spec * i_light_color;

        fragmentColor = vec4(ambient.xyz + diffuse.xyz + specular.xyz, 1.0) * i_col;
    //}
    //else
        //fragmentColor = i_col;
}