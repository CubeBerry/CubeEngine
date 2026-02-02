#extension GL_EXT_nonuniform_qualifier : enable

// #pragma pack_matrix(row_major)

#define MAX_TEXTURES 500
#define MAX_LIGHTS 10
#define PI 3.14159265358979323846f

// Vertex Shader
struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

[shader("vertex")]
VSOutput vertexMain(uint vertexID : SV_VertexID)
{
    VSOutput output;

    float2 texcoord = float2((vertexID << 1) & 2, vertexID & 2);
#if defined(__hlsl__)
    output.position = float4(texcoord * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
#else
    output.position = float4(texcoord.x * 2.0 - 1.0, texcoord.y * 2.0 - 1.0, 0.0, 1.0);
#endif
    output.uv = texcoord;
    
    return output;
}

// Fragment Shader
struct fDirectionalLight
{
    float3 lightDirection;
    float ambientStrength;
    float3 lightColor;
    float specularStrength;
}

struct fDirectionalLightList
{
    fDirectionalLight lights[MAX_LIGHTS];
}
[[vk::binding(3, 1)]] ConstantBuffer<fDirectionalLightList> directionalLightList : register(b0, space0);

struct fPointLight
{
    float3 lightPosition;
    float ambientStrength;
    float3 lightColor;
    float specularStrength;

    float constant;
    float linear;
    float quadratic;
}

struct fPointLightList
{
    fPointLight lights[MAX_LIGHTS];
}
[[vk::binding(4, 1)]] ConstantBuffer<fPointLightList> pointLightList : register(b1, space0);

struct PushConstants
{
    float3 viewPosition;
    int meshletVisualization;
    int activeDirectionalLights;
    int activePointLights;
}
#if defined(__hlsl__)
ConstantBuffer<PushConstants> pushConstants : register(b2, space0);
#else
[[vk::push_constant]] PushConstants pushConstants;
#endif

Texture2D gAlbedo : register(t0, space1);
Texture2D gNormal : register(t1, space1);
Texture2D gPosition : register(t2, space1);
Texture2D gMaterial : register(t3, space1);
SamplerState gSampler : register(s0, space1);

#if defined(__hlsl__)
TextureCube irradianceMap : register(t0, space2);
TextureCube prefilterMap : register(t1, space2);
Texture2D brdfLUT : register(t2, space2);
SamplerState iblSmp : register(s1, space2);
#else
[[vk::binding(5, 1)]] SamplerCube irradianceMap;
[[vk::binding(6, 1)]] SamplerCube prefilterMap;
[[vk::binding(7, 1)]] Sampler2D brdfLUT;
#endif

// float3 BlinnPhong(float3 lightPosition, float3 lightColor, float ambientStrength, float specularStrength, bool isPointLight, int lightIndex)
// {
//     float3 light_direction = float3(0.0);
//     float attenuation = 0.0;
//     if (isPointLight)
//     {
//         light_direction = normalize(lightPosition - i_fragment_position);
//         float distance = length(lightPosition - i_fragment_position);
//         attenuation = 1.0 / (pointLightList[lightIndex].constant + pointLightList[lightIndex].linear * distance + pointLightList[lightIndex].quadratic * (distance * distance));
//     }
//     else
//         light_direction = normalize(-lightPosition);

//     // Ambient Lighting
//     float3 ambient = ambientStrength * lightColor;

//     // Diffuse Lighting
//     float3 normal = normalize(i_normal);
//     float diff = max(dot(normal, light_direction), 0.0);
//     float3 diffuse = diff * lightColor;

//     // Specular Lighting
//     float3 specular = float3(0.0);
//     if (diff > 0.0)
//     {
//         float3 view_direction = normalize(i_view_position - i_fragment_position);
//         // float3 reflect_direction = reflect(-light_direction, normal);
//         float3 halfway_vector = normalize(view_direction + light_direction);

//         // float spec = pow(max(dot(view_direction, reflect_direction), 0.0), f_material[i_index].shininess);
//         float spec = pow(max(dot(halfway_vector, normal), 0.0), f_material.shininess);
//         specular = specularStrength * spec * lightColor * f_material.specularColor;
//     }

//     if (isPointLight)
//     {
//         ambient *= attenuation;
//         diffuse *= attenuation;
//         specular *= attenuation;
//     }

//     return ambient + diffuse + specular;
// }

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
    return F0 + (float3(1.0) - F0) * pow(1 - max(dot(V, H), 0.0), 5.0);
}

float3 Froughness(float3 F0, float3 V, float3 H, float roughness)
{
    return F0 + (max(float3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - max(dot(H, V), 0.0), 0.0, 1.0), 5.0);
}

struct MainVectors
{
    float3 worldPosition;
    float3 albedo;
    float4 material;
    float3 V;
    float3 N;
    float3 F0;
};

// Rendering Equation for one light source
float3 PBR(MainVectors mainVectors, float3 lightPosition, float3 lightColor, bool isPointLight, int lightIndex)
{
    float metallic = mainVectors.material.r;
    float roughness = mainVectors.material.g;
    // float ao = mainVectors.material.b;
    float ao = 1.0;
    float distance = isPointLight ? length(lightPosition - mainVectors.worldPosition) : 1.0;

    float3 L = float3(0.0);
    float attenuation = 0.0;
    if (isPointLight)
    {
        L = normalize(lightPosition - mainVectors.worldPosition);
        attenuation = 1.0 / (pointLightList.lights[lightIndex].constant + pointLightList.lights[lightIndex].linear * distance + pointLightList.lights[lightIndex].quadratic * (distance * distance));
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
    float3 Kd = (1.0 - metallic) * (float3(1.0) - Ks);

    float3 lambert = mainVectors.albedo / PI;

    // Cook-Torrance BRDF
    float alpha = roughness * roughness;
    float3 cookTorranceNumerator = D(alpha, mainVectors.N, H) * G(alpha, mainVectors.N, mainVectors.V, L) * F(mainVectors.F0, mainVectors.V, H);
    float cookTorranceDenominator = 4.0 * max(dot(mainVectors.V, mainVectors.N), 0.0) * max(dot(L, mainVectors.N), 0.0);
    cookTorranceDenominator = max(cookTorranceDenominator, 0.1);
    float3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;

    float3 BRDF = Kd * lambert + cookTorrance;
    float3 outgoingLight = ao * BRDF * radiance * max(dot(L, mainVectors.N), 0.0);

    return outgoingLight;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    float3 resultColor = float3(0.0);

    float4 albedoSample = gAlbedo.Sample(gSampler, input.uv);
    if (albedoSample.a < 0.01) discard;
    float3 albedo = albedoSample.rgb;
    if (pushConstants.meshletVisualization > 0) return float4(albedo, 1.0f);
    float3 normal = gNormal.Sample(gSampler, input.uv).rgb;
    float3 worldPosition = gPosition.Sample(gSampler, input.uv).rgb;
    float4 material = gMaterial.Sample(gSampler, input.uv);
    // R == Metallic, G == Roughness, B == AO, A == (Unused)
    float metallic = material.r;
    float roughness = material.g;
    // float ao; = material.b;

    // float3 F0 = mix(float3(0.04), albedo, f_material.metallic);
    float3 F0 = lerp(float3(0.04), albedo, metallic);
    float3 V = normalize(pushConstants.viewPosition - worldPosition);
    float3 N = normalize(normal);
    MainVectors mainVectors = { worldPosition, albedo, material, V, N, F0 };

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
    float3 F = Froughness(F0, V, N, roughness);

    float3 Ks = F;
    float3 Kd = (1.0 - metallic) * (float3(1.0) - Ks);
#if defined(__hlsl__)
    float3 irradiance = irradianceMap.Sample(iblSmp, N).rgb;
#else
    float3 irradiance = irradianceMap.Sample(N).rgb;
#endif
    float3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
#if defined(__hlsl__)
    float3 prefilteredColor = prefilterMap.SampleLevel(iblSmp, R, roughness * MAX_REFLECTION_LOD).rgb;
    float2 brdf = brdfLUT.Sample(iblSmp, float2(max(dot(N, V), 0.0), roughness)).rg;
#else
    float3 prefilteredColor = prefilterMap.SampleLevel(R, roughness * MAX_REFLECTION_LOD).rgb;
    float2 brdf = brdfLUT.Sample(float2(max(dot(N, V), 0.0), roughness)).rg;
#endif
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    // 1.0 == ao
    float3 ambient = (Kd * diffuse + specular) * 1.0;
    resultColor = ambient + resultColor;

    // PBR Gamma Correction
    resultColor = resultColor / (resultColor + float3(1.0));
    resultColor = pow(resultColor, float3(1.0 / 2.2));

    // Blinn-Phong Result Color
    // if (f_matrix.isTex)
    // {
    //     vec4 textureColor = texture(tex[f_matrix.texIndex  + i_tex_sub_index], i_uv);
    //     fragmentColor = vec4(resultColor, 1.0) * textureColor;
    // }
    // else fragmentColor = vec4(resultColor, 1.0) * (i_col + 0.5);

    // PBR Result Color
    // return float4(resultColor, 1.0);

    return float4(resultColor, 1.0);
}
