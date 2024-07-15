#version 460
precision mediump float;

#define MAX_TEXTURES 20

// layout(location = 0) in vec2 i_uv;
// layout(location = 1) in vec4 i_col;
// layout(location = 2) in float inIsTex;
// layout(location = 3) in flat int inIndex;

layout(location = 0) out vec4 fragmentColor;

// struct fMatrix
// {
//     int texIndex;
// };

// layout(binding = 1) uniform fUniformMatrix
// {
//     fMatrix f_matrix[MAX_TEXTURES];
// };

// uniform sampler2D tex[MAX_TEXTURES];

void main()
{
    //fragmentColor = mix(i_col, i_col * texture(tex[f_matrix[inIndex].texIndex], i_uv).rgba, inIsTex);

    fragmentColor = vec4(1.0, 0.0, 0.0, 1.0);

    // if(inIsTex == 1.0)
    // {
    //     fragmentColor = i_col * texture(tex[inTexIndex], i_uv).rgba;
    // }
    // else
    // {
    //     fragmentColor = i_col;
    // }
}