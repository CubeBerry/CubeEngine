#version 460 core

#if VULKAN
#define MAX_TEXTURES 500
#else
#define MAX_TEXTURES 20
#endif

layout(location = 0) in vec4 i_col;

layout(location = 0) out vec4 fragmentColor;

void main()
{
    fragmentColor = i_col;
}