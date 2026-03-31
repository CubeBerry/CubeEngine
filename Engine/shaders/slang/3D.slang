#extension GL_EXT_nonuniform_qualifier : enable

#include "Shadow.slang"

// #pragma pack_matrix(row_major)

#define MAX_TEXTURES 500
#define MAX_LIGHTS 500
#define PI 3.14159265358979323846f

// Vertex Shader
struct VSInput
{
    [[vk::location(0)]] uint position : POSITION0;
    [[vk::location(1)]] float3 normal : NORMAL0;
    [[vk::location(2)]] float2 uv : TEXCOORD0;
    [[vk::location(3)]] int tex_sub_index : TEXCOORD1;
    [[vk::location(7)]] int4 boneIDs : BLENDINDICES0;
    [[vk::location(8)]] float4 weights : BLENDWEIGHTS0;
}

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    int tex_sub_index : TEXCOORD1;
    int4 boneIDs : BLENDINDICES0;
    float4 weights : BLENDWEIGHTS0;
    // Lighting
    float3 normal : NORMAL0;
    float3 fragmentPosition : TEXCOORD2;
    float3 viewPosition : TEXCOORD3;
    bool meshletVisualization : TEXCOORD4;
};

struct vMatrix
{
    float4x4 model;
    // @TODO move to push constants
    float4x4 transposeInverseModel;
    float4x4 view;
    float4x4 projection;
    float4x4 decode;
    float4 color;
    // @TODO move to push constants
    float4 viewPosition;

    // Bone Matrices for Skeletal Animation
    float4x4 finalBones[128];
}
[[vk::binding(0, 0)]] ConstantBuffer<vMatrix> matrix : register(b0, space0);

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    // 11, 11, 10
    float x = float(input.position & 0x7FFu);
    float y = float((input.position >> 11) & 0x7FFu);
    float z = float((input.position >> 22) & 0x3FFu);
    float3 decoded_position = mul(matrix.decode, float4(float3(x, y, z), 1.0)).xyz;

    output.uv = input.uv;
    output.color = matrix.color;
    output.tex_sub_index = input.tex_sub_index;

    output.boneIDs = input.boneIDs;
    output.weights = input.weights;
    output.meshletVisualization = false;

    // Skinning Logic
    float4 totalPosition = float4(0.0, 0.0, 0.0, 0.0);
    float3 totalNormal   = float3(0.0, 0.0, 0.0);

    float totalWeight = input.weights[0] + input.weights[1] + input.weights[2] + input.weights[3];

    if (totalWeight > 0.01f)
    {
        // Skinned mesh: apply bone transformations
        float weightNormalizationScale = 1.0 / totalWeight;

        for (int i = 0; i < 4; i++)
        {
            if (input.boneIDs[i] < 0 || input.boneIDs[i] >= 128)
                continue;
            if (input.weights[i] <= 0.0f)
                continue;

            float normalizedWeight = input.weights[i] * weightNormalizationScale;

            float4 localPosition = mul(matrix.finalBones[input.boneIDs[i]], float4(decoded_position, 1.0f));
            totalPosition += localPosition * normalizedWeight;

            float3 localNormal = mul((float3x3)matrix.finalBones[input.boneIDs[i]], input.normal);
            totalNormal += localNormal * normalizedWeight;
        }
    }
    else
    {
        // Non-skinned mesh: use vertex position/normal directly
        totalPosition = float4(decoded_position, 1.0f);
        totalNormal   = input.normal;
    }

    // Lighting
    //output.normal = mul((float3x3)matrix.transposeInverseModel, input.normal);
    //output.fragmentPosition = mul(matrix.model, float4(decoded_position, 1.0f)).xyz;
    output.normal = mul((float3x3)matrix.transposeInverseModel, totalNormal);
    output.fragmentPosition = mul(matrix.model, totalPosition).xyz;
    output.viewPosition = matrix.viewPosition.xyz;

    //output.position = mul(matrix.projection, mul(matrix.view, mul(matrix.model, float4(decoded_position, 1.0))));
    output.position = mul(matrix.projection, mul(matrix.view, mul(matrix.model, totalPosition)));
    
    return output;
}

// Mesh Shader
#define MAX_VERTICES_PER_MESHLET 64
#define MAX_PRIMITIVES_PER_MESHLET 128

struct Meshlet
{
    uint vertexCount;
    uint vertexOffset;
    uint primitiveCount;
    uint primitiveOffset;
};

StructuredBuffer<VSInput> uniqueVertices : register(t8);
StructuredBuffer<Meshlet> meshlets : register(t9);
StructuredBuffer<uint> uniqueVertexIndices : register(t10);
StructuredBuffer<uint> primitiveIndices : register(t11);
uniform uint meshletVisualization : register(b6, space0);

[numthreads(128, 1, 1)]
[outputtopology("triangle")]
[shader("mesh")]
void meshMain(
    uint groupThreadID: SV_GroupThreadID,
    uint groupID: SV_GroupID,
    OutputVertices<VSOutput, MAX_VERTICES_PER_MESHLET> verts,
    OutputIndices<uint3, MAX_PRIMITIVES_PER_MESHLET> tris
)
{
    Meshlet meshlet = meshlets[groupID.x];
    SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);

    bool meshletVisualizationEnabled = meshletVisualization > 0;
    // MAX_VERTICES_PER_MESHLET == 64, MAX_PRIMITIVES_PER_MESHLET == 128
    if (groupThreadID < meshlet.vertexCount)
    {
        uint vertexIndex = uniqueVertexIndices[meshlet.vertexOffset + groupThreadID];
        VSInput input = uniqueVertices[vertexIndex];

        // 11, 11, 10
        float x = float(input.position & 0x7FFu);
        float y = float((input.position >> 11) & 0x7FFu);
        float z = float((input.position >> 22) & 0x3FFu);
        float3 decoded_position = mul(matrix.decode, float4(float3(x, y, z), 1.0)).xyz;

        // Skinning Logic
        float4 totalPosition = float4(0.0, 0.0, 0.0, 0.0);
        float3 totalNormal   = float3(0.0, 0.0, 0.0);

        float totalWeight = input.weights[0] + input.weights[1] + input.weights[2] + input.weights[3];

        if (totalWeight > 0.01f)
        {
            float weightNormalizationScale = 1.0 / totalWeight;

            for (int i = 0; i < 4; i++)
            {
                if (input.boneIDs[i] < 0 || input.boneIDs[i] >= 128)
                    continue;
                if (input.weights[i] <= 0.0f)
                    continue;

                float normalizedWeight = input.weights[i] * weightNormalizationScale;

                float4 localPosition = mul(matrix.finalBones[input.boneIDs[i]], float4(decoded_position, 1.0f));
                totalPosition += localPosition * normalizedWeight;

                float3 localNormal = mul((float3x3)matrix.finalBones[input.boneIDs[i]], input.normal);
                totalNormal += localNormal * normalizedWeight;
            }
        }
        else
        {
            totalPosition = float4(decoded_position, 1.0f);
            totalNormal   = input.normal;
        }

        verts[groupThreadID].uv = input.uv;
        verts[groupThreadID].meshletVisualization = meshletVisualizationEnabled;
        if (meshletVisualizationEnabled)
        {
            // Mesh Shader Debug Color
            uint hash = groupID.x;
            hash = (hash ^ 61) ^ (hash >> 16);
            hash = hash + (hash << 3);
            hash = hash ^ (hash >> 4);
            hash = hash * 0x27d4eb2d;
            hash = hash ^ (hash >> 15);
            float3 debugColor = float3(
                float(hash & 0xFF) / 255.0f,
                float((hash >> 8) & 0xFF) / 255.0f,
                float((hash >> 16) & 0xFF) / 255.0f
            );
            verts[groupThreadID].color = float4(debugColor, 1.0f);
        }
        else verts[groupThreadID].color = matrix.color;
        verts[groupThreadID].tex_sub_index = input.tex_sub_index;

        // Lighting
        verts[groupThreadID].normal = mul((float3x3)matrix.transposeInverseModel, totalNormal);
        verts[groupThreadID].fragmentPosition = mul(matrix.model, totalPosition).xyz;
        verts[groupThreadID].viewPosition = matrix.viewPosition.xyz;

        verts[groupThreadID].position = mul(matrix.projection, mul(matrix.view, mul(matrix.model, totalPosition)));
    }

    if (groupThreadID < meshlet.primitiveCount)
    {
        uint i0 = primitiveIndices[meshlet.primitiveOffset + groupThreadID * 3 + 0];
        uint i1 = primitiveIndices[meshlet.primitiveOffset + groupThreadID * 3 + 1];
        uint i2 = primitiveIndices[meshlet.primitiveOffset + groupThreadID * 3 + 2];
        tris[groupThreadID] = uint3(i0, i1, i2);
    }

    // https://gpuopen.com/learn/mesh_shaders/mesh_shaders-optimization_and_best_practices/
    // It differs based on GPU, max 128 vertices and 256 primitives are recommended to Radeon
    // MAX_VERTICES_PER_MESHLET == 128, MAX_PRIMITIVES_PER_MESHLET == 256
    // for (uint i = 0; i < 2; ++i)
    // {
    //     const uint primitiveId = groupThreadID + i * 128;

    //     if (primitiveId < meshlet.vertexCount)
    //     {
    //         uint vertexIndex = uniqueVertexIndices[meshlet.vertexOffset + primitiveId];
    //         VSInput input = uniqueVertices[vertexIndex];

    //         // 11, 11, 10
    //         float x = float(input.position & 0x7FFu);
    //         float y = float((input.position >> 11) & 0x7FFu);
    //         float z = float((input.position >> 22) & 0x3FFu);
    //         float3 decoded_position = mul(matrix.decode, float4(float3(x, y, z), 1.0)).xyz;

    //         // Skinning Logic
    //         float4 totalPosition = float4(0.0, 0.0, 0.0, 0.0);
    //         float3 totalNormal   = float3(0.0, 0.0, 0.0);

    //         float totalWeight = input.weights[0] + input.weights[1] + input.weights[2] + input.weights[3];

    //         if (totalWeight > 0.01f)
    //         {
    //             float weightNormalizationScale = 1.0 / totalWeight;

    //             for (int j = 0; j < 4; j++)
    //             {
    //                 if (input.boneIDs[j] < 0 || input.boneIDs[j] >= 128)
    //                     continue;
    //                 if (input.weights[j] <= 0.0f)
    //                     continue;

    //                 float normalizedWeight = input.weights[j] * weightNormalizationScale;

    //                 float4 localPosition = mul(matrix.finalBones[input.boneIDs[j]], float4(decoded_position, 1.0f));
    //                 totalPosition += localPosition * normalizedWeight;

    //                 float3 localNormal = mul((float3x3)matrix.finalBones[input.boneIDs[j]], input.normal);
    //                 totalNormal += localNormal * normalizedWeight;
    //             }
    //         }
    //         else
    //         {
    //             totalPosition = float4(decoded_position, 1.0f);
    //             totalNormal   = input.normal;
    //         }

    //         verts[primitiveId].uv = input.uv;
    //         verts[primitiveId].color = matrix.color;
    //         verts[primitiveId].tex_sub_index = input.tex_sub_index;

    //         // Lighting
    //         verts[primitiveId].normal = mul((float3x3)matrix.transposeInverseModel, totalNormal);
    //         verts[primitiveId].fragmentPosition = mul(matrix.model, totalPosition).xyz;
    //         verts[primitiveId].viewPosition = matrix.viewPosition.xyz;

    //         verts[primitiveId].position = mul(matrix.projection, mul(matrix.view, mul(matrix.model, totalPosition)));
    //     }

    //     if (primitiveId < meshlet.primitiveCount)
    //     {
    //         uint i0 = primitiveIndices[meshlet.primitiveOffset + primitiveId * 3 + 0];
    //         uint i1 = primitiveIndices[meshlet.primitiveOffset + primitiveId * 3 + 1];
    //         uint i2 = primitiveIndices[meshlet.primitiveOffset + primitiveId * 3 + 2];
    //         tris[primitiveId] = uint3(i0, i1, i2);
    //     }
    // }
}

// Fragment Shader
struct fMatrix
{
    int isTex;
    int texIndex;
}
[[vk::binding(0, 1)]] ConstantBuffer<fMatrix> f_matrix : register(b1, space0);

#if defined(__hlsl__)
SamplerState smp : register(s0, space1);
Texture2D tex[MAX_TEXTURES] : register(t0, space1);
#else
[[vk::binding(1, 1)]] Sampler2D tex[MAX_TEXTURES];
#endif

struct fMaterial
{
    // Blinn-Phong lighting
    float3 specularColor;
    float shininess;

    // PBR
    float metallic;
    float roughness;
}
[[vk::binding(2, 1)]] ConstantBuffer<fMaterial> f_material : register(b2, space0);

struct fDirectionalLight
{
    float3 lightDirection;
    float ambientStrength;
    float3 lightColor;
    float specularStrength;
    float intensity;
}

struct fDirectionalLightList
{
    fDirectionalLight lights[MAX_LIGHTS];
}
[[vk::binding(3, 1)]] ConstantBuffer<fDirectionalLightList> directionalLightList : register(b3, space0);

struct fPointLight
{
    float3 lightPosition;
    float ambientStrength;
    float3 lightColor;
    float specularStrength;
    float intensity;

    float constant;
    float linear;
    float quadratic;
    float radius;
}

struct fPointLightList
{
    fPointLight lights[MAX_LIGHTS];
}
[[vk::binding(4, 1)]] ConstantBuffer<fPointLightList> pointLightList : register(b4, space0);

#if defined(__hlsl__)
TextureCube irradianceMap : register(t0, space2);
TextureCube prefilterMap : register(t1, space2);
Texture2D brdfLUT : register(t2, space2);
#else
[[vk::binding(5, 1)]] SamplerCube irradianceMap;
[[vk::binding(6, 1)]] SamplerCube prefilterMap;
[[vk::binding(7, 1)]] Sampler2D brdfLUT;
#endif
#if defined(__hlsl__)
Texture2D shadowMap : register(t3, space2);
SamplerState iblSmp : register(s1, space2);
#endif

struct PushConstants
{
    int activeDirectionalLights;
    int activePointLights;
#if defined(__hlsl__)
    int useShadow;
    float orthoSize;
    float4x4 lightViewProjection;
#endif
}
#if defined(__hlsl__)
ConstantBuffer<PushConstants> pushConstants : register(b5, space0);
#else
[[vk::push_constant]] PushConstants pushConstants;
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

// @TODO Remove this function after post-process tone mapping is implemented in Vulkan
// Reinhard Tone Mapping
float3 ReinhardToneMapping(float3 color)
{
    return color / (color + float3(1.0));
}

// @TODO Remove this function after post-process tone mapping is implemented in Vulkan
// Filimic/ACES Tone Mapping
float3 FilmicToneMapping(float3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

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
    float3 fragmentPosition;
    float3 albedo;
    float3 V;
    float3 N;
    float3 F0;
};

// Rendering Equation for one light source
float3 PBR(MainVectors mainVectors, float3 lightPosition, float3 lightColor, bool isPointLight, int lightIndex)
{
    fMaterial material = f_material;

    float ao = 1.0;
    float3 L = isPointLight ? normalize(lightPosition - mainVectors.fragmentPosition) : normalize(-lightPosition);
    // Half Vector
    float3 H = normalize(mainVectors.V + L);
    float3 radiance = lightColor;

    float3 Ks = F(mainVectors.F0, mainVectors.V, H);
    float3 Kd = (1.0 - material.metallic) * (float3(1.0) - Ks);

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

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    float3 resultColor = float3(0.0);

    float3 albedo = float3(0.0);
#if defined(__hlsl__)
    if (f_matrix.isTex > 0) albedo = tex[f_matrix.texIndex + input.tex_sub_index].Sample(smp, input.uv).rgb;
#else
    if (f_matrix.isTex > 0) albedo = tex[f_matrix.texIndex + input.tex_sub_index].Sample(input.uv).rgb;
#endif
    else albedo = input.color.rgb;

    // float3 F0 = mix(float3(0.04), albedo, f_material.metallic);
    float3 F0 = lerp(float3(0.04), albedo, f_material.metallic);
    float3 V = normalize(input.viewPosition - input.fragmentPosition);
    float3 N = normalize(input.normal);
    MainVectors mainVectors = { input.fragmentPosition, albedo, V, N, F0 };

    // Shadow Mapping: Slope-Scaled Normal Offset Bias
    // =========================================================================
    // This technique is a modern standard for hard shadow mapping, designed to
    // simultaneously mitigate both Shadow Acne (surface noise) and Peter Panning.
    //
    // 1. Calculate the angle (NdotL) between the light direction (L) and surface normal (N).
    // 2. Surfaces receiving light at grazing angles are offset more strongly along their normal.
    // 3. Surfaces facing the light directly (e.g., floors) receive minimal offset,
    //    preventing the shadow from artificially shrinking or detaching.
    // =========================================================================
    float shadow = 0.0;
#if defined(__hlsl__)
    if (pushConstants.useShadow > 0)
    {
        // Transform to light space
        // Normal Offset Bias
        // fragmentPosition == worldPosition
        float3 offsetPos = input.fragmentPosition + N * 0.05;
        float4 lightSpacePosition = mul(pushConstants.lightViewProjection, float4(offsetPos, 1.0));
        float3 projectionCoords = lightSpacePosition.xyz / lightSpacePosition.w;

        // Convert to UV coordinates (Handles both DirectX and Vulkan coordinate systems)
        float2 shadowUV;
        shadowUV.x = projectionCoords.x * 0.5 + 0.5;
#if defined(__hlsl__)
        shadowUV.y = -projectionCoords.y * 0.5 + 0.5;
#else
        shadowUV.y = projectionCoords.y * 0.5 + 0.5;
#endif

        float4 b = shadowMap.Sample(iblSmp, shadowUV);
        float z_f = projectionCoords.z;
        float G = Hamburger4MSM(b, z_f);

        // // Depth Comparison
        // float closestDepth = shadowMap.Sample(gSampler, shadowUV).r;
        // float currentDepth = projectionCoords.z;
        // // shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

        // // Apply a very small constant bias as a baseline defense against micro-acne
        // float constantBias = 0.0001;
        // float isShadow = currentDepth - constantBias > closestDepth ? 1.0 : 0.0;

        // Frustum bounds check (Prevents clamping smearing)
        // Branchless optimization using step() instead of if-statements
        float bounds = step(0.0, shadowUV.x) * step(shadowUV.x, 1.0) *
                       step(0.0, shadowUV.y) * step(shadowUV.y, 1.0) *
                       step(0.0, z_f) * step(z_f, 1.0);
        shadow = G * bounds;
    }
    else shadow = 0.0;
#endif // defined(__hlsl__)

    // Calculate Directional Lights
    for (int l = 0; l < pushConstants.activeDirectionalLights; ++l)
    {
        fDirectionalLight currentLight = directionalLightList.lights[l];
        // resultColor += clamp(BlinnPhong(currentLight.lightDirection, currentLight.lightColor, currentLight.ambientStrength, currentLight.specularStrength, false, l), 0.0, 1.0);
        resultColor += PBR(mainVectors, currentLight.lightDirection, currentLight.lightColor * currentLight.intensity, false, l) * (1.0 - shadow);
    }

    // Calculate Point Lights
    for (int l = 0; l < pushConstants.activePointLights; ++l)
    {
        fPointLight currentLight = pointLightList.lights[l];
        float distance = length(currentLight.lightPosition - input.fragmentPosition);
        if (distance > currentLight.radius) continue;
        // float attenuation = 1.0 / (pointLightList.lights[lightIndex].constant + pointLightList.lights[lightIndex].linear * distance + pointLightList.lights[lightIndex].quadratic * (distance * distance));
        float attenuation = max(0.0, (1.0 / (distance * distance)) - (1.0 / (currentLight.radius * currentLight.radius)));
        // resultColor += clamp(BlinnPhong(currentLight.lightPosition, currentLight.lightColor, currentLight.ambientStrength, currentLight.specularStrength, true, l), 0.0, 1.0);
        resultColor += PBR(mainVectors, currentLight.lightPosition, currentLight.lightColor * currentLight.intensity, true, l) * attenuation;
    }

    // PBR IBL Ambient Lighting
    float3 R = reflect(-V, N);
    float3 F = Froughness(F0, V, N, f_material.roughness);

    float3 Ks = F;
    float3 Kd = (1.0 - f_material.metallic) * (float3(1.0) - Ks);
#if defined(__hlsl__)
    float3 irradiance = irradianceMap.Sample(iblSmp, N).rgb;
#else
    float3 irradiance = irradianceMap.Sample(N).rgb;
#endif
    float3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
#if defined(__hlsl__)
    float3 prefilteredColor = prefilterMap.SampleLevel(iblSmp, R, f_material.roughness * MAX_REFLECTION_LOD).rgb;
    float2 brdf = brdfLUT.Sample(iblSmp, float2(max(dot(N, V), 0.0), f_material.roughness)).rg;
#else
    float3 prefilteredColor = prefilterMap.SampleLevel(R, f_material.roughness * MAX_REFLECTION_LOD).rgb;
    float2 brdf = brdfLUT.Sample(float2(max(dot(N, V), 0.0), f_material.roughness)).rg;
#endif
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    // 1.0 == ao
    float3 ambient = (Kd * diffuse + specular) * 1.0;
    resultColor = ambient + resultColor;

    // Blinn-Phong Result Color
    // if (f_matrix.isTex)
    // {
    //     vec4 textureColor = texture(tex[f_matrix.texIndex  + i_tex_sub_index], i_uv);
    //     fragmentColor = vec4(resultColor, 1.0) * textureColor;
    // }
    // else fragmentColor = vec4(resultColor, 1.0) * (i_col + 0.5);

    // PBR Result Color
    // return float4(resultColor, 1.0);

#if defined(__hlsl__)
    // Mesh Shader Debug Color
    return input.meshletVisualization ? float4(input.color.rgb, 1.0) : float4(resultColor, 1.0);
#else
    // @TODO Remove tone mapping after post-process tone mapping is implemented in Vulkan
    // Convert HDR to LDR
    resultColor = FilmicToneMapping(resultColor);
    // 2.2 Gamma Correction
    resultColor = pow(resultColor, 1.0 / 2.2);
    return float4(resultColor, 1.0);
#endif
}
