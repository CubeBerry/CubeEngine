// dxc WorkGraphsFrustumCulling.frag.hlsl -T ps_6_6 -E pixelMain -Fo WorkGraphsFrustumCulling.frag.cso
/*
    Mesh:
    CBV - b0 ~ b1
    SRV - t0 ~ t5
    Pixel:
    CBV - b2 ~ b4
    SRV - t6 ~ t7
    Sampler - (s0, space1), (s1, space2)
    Texture - (t0, space1), (t0 ~ t2, space2)
*/
GlobalRootSignature globalRootSignature =
    {
        "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED),"\
    "CBV(b0),"\
    "RootConstants(num32BitConstants=1, b1),"\
    "CBV(b2),"\
    "CBV(b3),"\
    "RootConstants(num32BitConstants=2, b4),"\
    "SRV(t0),"\
    "SRV(t1),"\
    "SRV(t2),"
    "SRV(t3),"\
    "SRV(t4),"\
    "SRV(t5),"\
    "SRV(t6),"\
    "SRV(t7),"\
    "StaticSampler(s0, space=1, filter=FILTER_MIN_MAG_MIP_POINT, addressU=TEXTURE_ADDRESS_CLAMP, addressV=TEXTURE_ADDRESS_CLAMP),"\
    "DescriptorTable(SRV(t0, numDescriptors = unbounded, space=1, flags = DESCRIPTORS_VOLATILE)),"\
    "StaticSampler(s1, space=2, filter=FILTER_MIN_MAG_MIP_LINEAR, addressU=TEXTURE_ADDRESS_CLAMP, addressV=TEXTURE_ADDRESS_CLAMP),"\
    "DescriptorTable(SRV(t0, numDescriptors = 3, space=2)),"\
};

#define MAX_TEXTURES 500
#define MAX_LIGHTS 10
#define PI 3.14159265358979323846f

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    int tex_sub_index : TEXCOORD1;
    // Lighting
    float3 normal : NORMAL0;
    float3 fragmentPosition : TEXCOORD2;
    float3 viewPosition : TEXCOORD3;
    bool meshletVisualization : TEXCOORD4;
};

// Fragment Shader
struct fMatrix
{
    int isTex;
    int texIndex;
};

struct fMaterial
{
    // Blinn-Phong lighting
    float3 specularColor;
    float shininess;

    // PBR
    float metallic;
    float roughness;
};

struct fDirectionalLight
{
    float3 lightDirection;
    float ambientStrength;
    float3 lightColor;
    float specularStrength;
};

struct fDirectionalLightList
{
    fDirectionalLight lights[MAX_LIGHTS];
};

struct fPointLight
{
    float3 lightPosition;
    float ambientStrength;
    float3 lightColor;
    float specularStrength;

    float constantAtt;
    float linearAtt;
    float quadraticAtt;
};

struct fPointLightList
{
    fPointLight lights[MAX_LIGHTS];
};

struct PushConstants
{
    int activeDirectionalLights;
    int activePointLights;
};

ConstantBuffer<fDirectionalLightList> directionalLightList : register(b2);
ConstantBuffer<fPointLightList> pointLightList : register(b3);
ConstantBuffer<PushConstants> pushConstants : register(b4);

StructuredBuffer<fMatrix> globalFragmentUniforms : register(t6);
StructuredBuffer<fMaterial> globalMaterialUniforms : register(t7);

SamplerState smp : register(s0, space1);
Texture2D tex[MAX_TEXTURES] : register(t0, space1);

SamplerState iblSmp : register(s1, space2);
TextureCube irradianceMap : register(t0, space2);
TextureCube prefilterMap : register(t1, space2);
Texture2D brdfLUT : register(t2, space2);


// GGX/Trowbridge-Reitz Normal Distribution Function
float D(float alpha, float3 N, float3 H)
{
    float numerator = pow(alpha, 2.0);

    float NdotH = max(dot(N, H), 0.0);
    float denominator = PI * pow(pow(NdotH, 2.0) * (pow(alpha, 2.0) - 1.0) + 1.0, 2.0);
    denominator = max(denominator, 0.000001);

    return numerator / denominator;
}

// Schlick-Beckmann Geometry Shadowing Function
float G1(float alpha, float3 N, float3 X)
{
    float numerator = max(dot(N, X), 0.0);

    float k = alpha / 2.0;
    float denominator = max(dot(N, X), 0.0) * (1.0 - k) + k;
    denominator = max(denominator, 0.000001);

    return numerator / denominator;
}

// Switch Model
float G(float alpha, float3 N, float3 V, float3 L)
{
    return G1(alpha, N, V) * G1(alpha, N, L);
}

// Fresnel-Schlick Function
float3 F(float3 F0, float3 V, float3 H)
{
    return F0 + ((float3)1.0 - F0) * pow(1 - max(dot(V, H), 0.0), 5.0);
}

float3 Froughness(float3 F0, float3 V, float3 H, float roughness)
{
    return F0 + (max((float3)(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - max(dot(H, V), 0.0), 0.0, 1.0), 5.0);
}

struct MainVectors
{
    float3 fragmentPosition;
    float3 albedo;
    float3 V;
    float3 N;
    float3 F0;
};

// Rendering Equation for one light source
float3 PBR(MainVectors mainVectors, float3 lightPosition, float3 lightColor, bool isPointLight, int lightIndex)
{
    fMaterial material = globalMaterialUniforms[0];

    float ao = 1.0;
    float distance = isPointLight ? length(lightPosition - mainVectors.fragmentPosition) : 1.0;

    float3 L = (float3)0.0;
    float attenuation = 0.0;
    if (isPointLight)
    {
        L = normalize(lightPosition - mainVectors.fragmentPosition);
        attenuation = 1.0 / (pointLightList.lights[lightIndex].constantAtt + pointLightList.lights[lightIndex].linearAtt * distance + pointLightList.lights[lightIndex].quadraticAtt * (distance * distance));
    }
    else
    {
        L = normalize(-lightPosition);
        attenuation = 1.0;
    }

    // Half Vector
    float3 H = normalize(mainVectors.V + L);
    float3 radiance = lightColor * attenuation;

    float3 Ks = F(mainVectors.F0, mainVectors.V, H);
    float3 Kd = (1.0 - material.metallic) * ((float3)1.0 - Ks);

    float3 lambert = mainVectors.albedo / PI;

    // Cook-Torrance BRDF
    float alpha = material.roughness * material.roughness;
    float3 cookTorranceNumerator = D(alpha, mainVectors.N, H) * G(alpha, mainVectors.N, mainVectors.V, L) * F(mainVectors.F0, mainVectors.V, H);
    float cookTorranceDenominator = 4.0 * max(dot(mainVectors.V, mainVectors.N), 0.0) * max(dot(L, mainVectors.N), 0.0);
    cookTorranceDenominator = max(cookTorranceDenominator, 0.1);
    float3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;

    float3 BRDF = Kd * lambert + cookTorrance;
    float3 outgoingLight = ao * BRDF * radiance * max(dot(L, mainVectors.N), 0.0);

    return outgoingLight;
}

[shader("pixel")]
float4 pixelMain(VSOutput input) : SV_Target
{
    float3 resultColor = (float3)0.0;

    float3 albedo = (float3)0.0;
    if (globalFragmentUniforms[0].isTex > 0) albedo = tex[globalFragmentUniforms[0].texIndex + input.tex_sub_index].Sample(smp, input.uv).rgb;
    else albedo = input.color.rgb;

    // float3 F0 = mix(float3(0.04), albedo, f_material.metallic);
    float3 F0 = lerp((float3)0.04, albedo, globalMaterialUniforms[0].metallic);
    float3 V = normalize(input.viewPosition - input.fragmentPosition);
    float3 N = normalize(input.normal);
    MainVectors mainVectors = { input.fragmentPosition, albedo, V, N, F0 };

    // Calculate Directional Lights
    for (int l = 0; l < pushConstants.activeDirectionalLights; ++l)
    {
        fDirectionalLight currentLight = directionalLightList.lights[l];
        // resultColor += clamp(BlinnPhong(currentLight.lightDirection, currentLight.lightColor, currentLight.ambientStrength, currentLight.specularStrength, false, l), 0.0, 1.0);
        resultColor += PBR(mainVectors, currentLight.lightDirection, currentLight.lightColor, false, l);
    }

    // Calculate Point Lights
    for (int l = 0; l < pushConstants.activePointLights; ++l)
    {
        fPointLight currentLight = pointLightList.lights[l];
        // resultColor += clamp(BlinnPhong(currentLight.lightPosition, currentLight.lightColor, currentLight.ambientStrength, currentLight.specularStrength, true, l), 0.0, 1.0);
        resultColor += PBR(mainVectors, currentLight.lightPosition, currentLight.lightColor, true, l);
    }

    // PBR IBL Ambient Lighting
    float3 R = reflect(-V, N);
    float3 F = Froughness(F0, V, N, globalMaterialUniforms[0].roughness);

    float3 Ks = F;
    float3 Kd = (1.0 - globalMaterialUniforms[0].metallic) * ((float3)1.0 - Ks);
    float3 irradiance = irradianceMap.Sample(iblSmp, N).rgb;
    float3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    float3 prefilteredColor = prefilterMap.SampleLevel(iblSmp, R, globalMaterialUniforms[0].roughness * MAX_REFLECTION_LOD).rgb;
    float2 brdf = brdfLUT.Sample(iblSmp, float2(max(dot(N, V), 0.0), globalMaterialUniforms[0].roughness)).rg;
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    // 1.0 == ao
    float3 ambient = (Kd * diffuse + specular) * 1.0;
    resultColor = ambient + resultColor;

    // PBR Gamma Correction
    resultColor = resultColor / (resultColor + (float3)1.0);
    resultColor = pow(resultColor, (float3)(1.0 / 2.2));

    // Blinn-Phong Result Color
    // if (f_matrix.isTex)
    // {
    //     vec4 textureColor = texture(tex[f_matrix.texIndex  + i_tex_sub_index], i_uv);
    //     fragmentColor = vec4(resultColor, 1.0) * textureColor;
    // }
    // else fragmentColor = vec4(resultColor, 1.0) * (i_col + 0.5);

    // PBR Result Color
    // return float4(resultColor, 1.0);

    // Mesh Shader Debug Color
    return input.meshletVisualization ? float4(input.color.rgb, 1.0) : float4(resultColor, 1.0);
}
