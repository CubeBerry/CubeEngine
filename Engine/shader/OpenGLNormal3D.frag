#version 460

#define MAX_TEXTURES 20

layout(location = 0) in vec4 i_col;

layout(location = 0) out vec4 fragmentColor;

void main()
{
    fragmentColor = i_col;
}