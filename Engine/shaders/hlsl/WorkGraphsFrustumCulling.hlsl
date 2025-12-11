// dxc WorkGraphsFrustumCulling.hlsl -T lib_6_9 -E CullNode -Fo WorkGraphsFrustumCulling.cso
GlobalRootSignature globalRootSignature =
{
    // Mesh Node
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED),"\
    "CBV(b0),"\
    "DescriptorTable(SRV(t0, numDescriptors = unbounded)),"\
    // Pixel Shader
    "CBV(b1),"\
    "CBV(b2),"\
};

#define MAX_VERTICES_PER_MESHLET 64
#define MAX_PRIMITIVES_PER_MESHLET 128

#define MAX_TEXTURES 500
#define MAX_LIGHTS 10
#define PI 3.14159265358979323846f

// ThreeDimension::StaticQuantizedVertex
struct VSInput
{
    uint position : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    int tex_sub_index : TEXCOORD1;
    uint objectId : TEXCOORD2;
};

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
    float3 viewPosition;
};

struct Meshlet
{
    uint vertexCount;
    uint vertexOffset;
    uint primitiveCount;
    uint primitiveOffset;
};

struct CullingData
{
    float4 frustumPlanes[6];
    uint32_t numMeshlets;
};

struct MeshNodeRecord
{
    uint objectID;
    uint meshletOffset;
    uint meshletCount;
    uint vertexIndexOffset;
    uint primitiveIndexOffset;
};

struct CullEntryRecord
{
    uint gridSize : SV_DispatchGrid;
};

ConstantBuffer<CullingData> cullingData : register(b0);
// uniform uint meshletVisualization : register(b6, space0);

StructuredBuffer<VSInput> globalUniqueVertices : register(t0);
StructuredBuffer<Meshlet> globalMeshlets : register(t1);
StructuredBuffer<vMatrix> globalVertexUniforms : register(t2);
StructuredBuffer<uint> globalUniqueVertexIndices : register(t3);
StructuredBuffer<uint> globalPrimitiveIndices : register(t4);

struct MeshletBounds
{
    float3 center;
    float3 extents; // bounding box half-size
};
StructuredBuffer<MeshletBounds> globalMeshletBounds : register(t5);

bool IsVisible(float3 center, float3 extents)
{
    for (int i = 0; i < 6; ++i)
    {
        float4 plane = cullingData.frustumPlanes[i];
        float d = dot(center, plane.xyz) + plane.w;
        float r = dot(extents, abs(plane.xyz));
        if (d + r < 0.0f) return false;
    }
    return true;
}

// Seems Slang Shading Language supports Work Graphs but no documentation is shown yet.
[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
// Handle 32 meshlets per thread group
[NumThreads(32, 1, 1)]
// first node = broadcastNode
void CullNode(
    uint dispatchThreadID: SV_DispatchThreadID,
    DispatchNodeInputRecord<CullEntryRecord> inputRecord,
    [MaxRecords(32)] NodeOutput<MeshNodeRecord> meshNodeOutput
)
{
    uint globalMeshletIndex = dispatchThreadID;
    if (globalMeshletIndex >= cullingData.numMeshlets) return;

    MeshletBounds bounds = globalMeshletBounds[globalMeshletIndex];
    uint objectID = 0; // @TODO get object ID from a buffer
    vMatrix vertexUniform = globalVertexUniforms[objectID];

    float3 worldCenter = mul(vertexUniform.model, float4(bounds.center, 1.0)).xyz;
    // float3 worldExtents = mul(vertexUniform.model, float4(bounds.extents, 0.0)).xyz;
    float3 worldExtents = bounds.extents;

    if (IsVisible(worldCenter, worldExtents))
    {
        ThreadNodeOutputRecords<MeshNodeRecord> outRecord = meshNodeOutput.GetThreadNodeOutputRecords(1);

        outRecord.Get().objectID = objectID;
        outRecord.Get().meshletOffset = globalMeshletIndex;
        outRecord.Get().meshletCount = 1;
        // @TODO Remove offsets later
        outRecord.Get().vertexIndexOffset = 0;
        outRecord.Get().primitiveIndexOffset = 0;

        outRecord.OutputComplete();
    }
}

[Shader("node")]
[NodeLaunch("mesh")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void MeshNode(
    DispatchNodeInputRecord<MeshNodeRecord> inputRecord,
    uint groupThreadID: SV_GroupThreadID,
    uint dispatchThreadID: SV_DispatchThreadID,
    out vertices VSOutput verts[MAX_VERTICES_PER_MESHLET],
    out indices uint3 tris[MAX_PRIMITIVES_PER_MESHLET]
)
{
    MeshNodeRecord record = inputRecord.Get();

    Meshlet meshlet = globalMeshlets[record.meshletOffset];
    vMatrix matrix = globalVertexUniforms[record.objectID];

    SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);

    // bool meshletVisualizationEnabled = meshletVisualization > 0;
    // MAX_VERTICES_PER_MESHLET == 64, MAX_PRIMITIVES_PER_MESHLET == 128
    if (groupThreadID < meshlet.vertexCount)
    {
        uint vertexIndex = globalUniqueVertexIndices[meshlet.vertexOffset + groupThreadID];
        VSInput input = globalUniqueVertices[vertexIndex];

        // 11, 11, 10
        float x = float(input.position & 0x7FFu);
        float y = float((input.position >> 11) & 0x7FFu);
        float z = float((input.position >> 22) & 0x3FFu);
        float3 decoded_position = mul(matrix.decode, float4(float3(x, y, z), 1.0)).xyz;

        verts[groupThreadID].uv = input.uv;
        // verts[groupThreadID].meshletVisualization = meshletVisualizationEnabled;
        // if (meshletVisualizationEnabled)
        // {
        //     // Mesh Shader Debug Color
        //     uint hash = groupID.x;
        //     hash = (hash ^ 61) ^ (hash >> 16);
        //     hash = hash + (hash << 3);
        //     hash = hash ^ (hash >> 4);
        //     hash = hash * 0x27d4eb2d;
        //     hash = hash ^ (hash >> 15);
        //     float3 debugColor = float3(
        //         float(hash & 0xFF) / 255.0f,
        //         float((hash >> 8) & 0xFF) / 255.0f,
        //         float((hash >> 16) & 0xFF) / 255.0f
        //     );
        //     verts[groupThreadID].color = float4(debugColor, 1.0f);
        // }
        // else verts[groupThreadID].color = matrix.color;
        verts[groupThreadID].tex_sub_index = input.tex_sub_index;

        // Lighting
        verts[groupThreadID].normal = mul((float3x3)matrix.transposeInverseModel, input.normal);
        verts[groupThreadID].fragmentPosition = mul(matrix.model, float4(decoded_position, 1.0f)).xyz;
        verts[groupThreadID].viewPosition = matrix.viewPosition;

        verts[groupThreadID].position = mul(matrix.projection, mul(matrix.view, mul(matrix.model, float4(decoded_position, 1.0))));
    }

    if (groupThreadID < meshlet.primitiveCount)
    {
        uint i0 = globalPrimitiveIndices[meshlet.primitiveOffset + groupThreadID * 3 + 0];
        uint i1 = globalPrimitiveIndices[meshlet.primitiveOffset + groupThreadID * 3 + 1];
        uint i2 = globalPrimitiveIndices[meshlet.primitiveOffset + groupThreadID * 3 + 2];
        tris[groupThreadID] = uint3(i0, i1, i2);
    }
}

// Fragment Shader
struct fMatrix
{
    int isTex;
    int texIndex;
}
StructuredBuffer<fMatrix> globalFragmentUniforms : register(t2);

SamplerState smp : register(s0, space1);
Texture2D tex[MAX_TEXTURES] : register(t0, space0);

struct fMaterial
{
    // Blinn-Phong lighting
    float3 specularColor;
    float shininess;

    // PBR
    float metallic;
    float roughness;
}
StructuredBuffer<fMaterial> globalMaterialUniforms : register(t2);

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
ConstantBuffer<fDirectionalLightList> directionalLightList : register(b3, space0);

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
ConstantBuffer<fPointLightList> pointLightList : register(b4, space0);

SamplerState iblSmp : register(s1, space2);
TextureCube irradianceMap : register(t0, space2);
TextureCube prefilterMap : register(t1, space2);
Texture2D brdfLUT : register(t2, space2);

struct PushConstants
{
    int activeDirectionalLights;
    int activePointLights;
}
ConstantBuffer<PushConstants> pushConstants : register(b5, space0);

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
    fMaterial material = globalMaterialUniforms[0];

    float ao = 1.0;
    float distance = isPointLight ? length(lightPosition - mainVectors.fragmentPosition) : 1.0;

    float3 L = float3(0.0);
    float attenuation = 0.0;
    if (isPointLight)
    {
        L = normalize(lightPosition - mainVectors.fragmentPosition);
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
    if (globalFragmentUniforms[0].isTex > 0) albedo = tex[globalFragmentUniforms[0].texIndex + input.tex_sub_index].Sample(smp, input.uv).rgb;
    else albedo = input.color.rgb;

    // float3 F0 = mix(float3(0.04), albedo, f_material.metallic);
    float3 F0 = lerp(float3(0.04), albedo, globalMaterialUniforms[0].metallic);
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
    float3 Kd = (1.0 - globalMaterialUniforms[0].metallic) * (float3(1.0) - Ks);
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

    // Mesh Shader Debug Color
    return input.meshletVisualization ? float4(input.color.rgb, 1.0) : float4(resultColor, 1.0);
}
