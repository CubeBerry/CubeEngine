#version 450
precision mediump float;

layout(location = 1) in vec2 i_uv;

layout(location = 0) out vec4 framgentColor;

//layout(set = 0, binding = 0) uniform Material
//{
//  mat3 matrix;  
//};

layout(set = 0, binding = 0) uniform uniformMatrix
{       
    mat4 model;
    mat4 view;
    mat4 projection;
} material[2];

layout(set = 1, binding = 1) uniform sampler2D tex[2];

void main()
{
    for(int i = 0; i < 2; ++i)
    {
        //vec3 col = texture(tex[0], i_uv).rgb;
        //vec3 col2 = texture(tex[1], i_uv).rgb;
        //framgentColor = vec4(col * col2, 1.0);

        vec3 col = texture(tex[i], i_uv).rgb;
        framgentColor = vec4(col, 1.0);
    }
}