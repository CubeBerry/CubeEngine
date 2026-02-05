#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 21 "slang/LocalLightingPass.slang"
struct fPointLight_0
{
    float3 lightPosition_0;
    float ambientStrength_0;
    float3 lightColor_0;
    float specularStrength_0;
    float intensity_0;
    float constant_0;
    float linear_0;
    float quadratic_0;
    float radius_0;
};


struct fPointLightList_0
{
    fPointLight_0  lights_0[int(10)];
};


#line 39
cbuffer pointLightList_0 : register(b0)
{
    fPointLightList_0 pointLightList_0;
}

#line 41
struct PushConstants_0
{
    float4x4 viewProjection_0;
    float3 viewPosition_0;
    float padding_0;
    float2 screenSize_0;
};

cbuffer pushConstants_0 : register(b1)
{
    PushConstants_0 pushConstants_0;
}

#line 15
struct VSOutput_0
{
    float4 position_0 : SV_POSITION;
    nointerpolation int lightIndex_0 : TEXCOORD1;
};


#line 10
struct VSInput_0
{
    float3 position_1 : POSITION0;
};


#line 55
VSOutput_0 vertexMain(VSInput_0 input_0, uint instanceID_0 : SV_InstanceID)
{
    VSOutput_0 output_0;

#line 62
    output_0.position_0 = mul(pushConstants_0.viewProjection_0, float4(input_0.position_1 * pointLightList_0.lights_0[instanceID_0].radius_0 + pointLightList_0.lights_0[instanceID_0].lightPosition_0, 1.0f));
    output_0.lightIndex_0 = int(instanceID_0);
    return output_0;
}

