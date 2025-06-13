#version 460 core
#if VULKAN
#extension GL_EXT_nonuniform_qualifier : enable
#endif

#if VULKAN
#else
#define MAX_MATRICES 20
#endif

layout(location = 0) in vec3 i_pos;

layout(location = 0) out vec2 o_uv;
layout(location = 1) out vec4 o_col;
layout(location = 2) out float outIsTex;

struct vMatrix
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 color;
    vec3 frameSize;
    float isTex;
    vec3 texelPos;
    float isTexel;
};

#if VULKAN
layout(set = 0, binding = 0) uniform vUniformMatrix
#else
layout(std140, binding = 0) uniform vUniformMatrix
#endif
{
    vMatrix matrix;
};

void main()
{
    //o_uv.x = ((i_pos.x + 1) / 2);
    //o_uv.y = ((i_pos.y + 1) / 2);

    //o_uv.x = mix((i_pos.x + 1) / 2, ((i_pos.x + 1) / 2) * matrix[index].frameSize.x + matrix[index].texelPos.x, isTexel);
    //o_uv.y = mix((i_pos.y + 1) / 2, ((i_pos.y + 1) / 2) * matrix[index].frameSize.y + matrix[index].texelPos.y, isTexel);

    if(matrix.isTexel == 1.0)
    {
        o_uv.x = mix((i_pos.x + 1.0) / 2.0, ((i_pos.x + 1.0) / 2.0) * matrix.frameSize.x + matrix.texelPos.x, matrix.isTexel);
        o_uv.y = mix((i_pos.y + 1.0) / 2.0, ((i_pos.y + 1.0) / 2.0) * matrix.frameSize.y + matrix.texelPos.y, matrix.isTexel);
    }
    else
    {
        o_uv.x = ((i_pos.x + 1.0) / 2.0);
        o_uv.y = ((i_pos.y + 1.0) / 2.0);
    }

    outIsTex = matrix.isTex;
    o_col = matrix.color;

    gl_Position = matrix.projection * matrix.view * matrix.model * vec4(i_pos, 1.0);
}