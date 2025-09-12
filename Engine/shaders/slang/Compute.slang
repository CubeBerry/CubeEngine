#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_TEXTURES 500
#define MAX_LIGHTS 10
#define PI 3.14159265358979323846f

#if defined(__hlsl__)
Texture2DMS<float4> g_input : register(t0);
RWTexture2D<float4> g_output : register(u0);
#else
Texture2D<float4> g_input : register(t0);
RWTexture2D<float4> g_output : register(u0);
#endif

[numthreads(8, 8, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // float4 originalColor = g_input.Load(dispatchThreadID.xy, 0);
    // float4 invertedColor = float4(1.f - originalColor.rgb, 1.f);
    // g_output[dispatchThreadID.xy] = invertedColor;

    g_output[dispatchThreadID.xy] = float4(1.f,0.f,0.f,1.f);
}
