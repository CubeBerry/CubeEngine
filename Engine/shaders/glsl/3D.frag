#version 460 core

#define MAX_TEXTURES 29
#define MAX_LIGHTS 500
const float PI = 3.14159265359;

layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec4 i_col;
layout(location = 2) in flat int i_tex_sub_index;
//Lighting
layout(location = 4) in vec3 i_normal;
layout(location = 5) in vec3 i_fragment_position;
layout(location = 6) in vec3 i_view_position;

layout(location = 0) out vec4 fragmentColor;

struct fMatrix
{
    int isTex;
    int texIndex;
};

struct fMaterial
{
    //Blinn-Phong
    vec3 specularColor;
    float shininess;

    //PBR
    float metallic;
    float roughness;
};

struct fDirectionalLight
{
    vec3 lightDirection;
    float ambientStrength;
    vec3 lightColor;
    float specularStrength;
    float intensity;
};

struct fPointLight
{
    vec3 lightPosition;
    float ambientStrength;
    vec3 lightColor;
    float specularStrength;
    float intensity;

    float constant;
    float linear;
    float quadratic;
    float radius;
};

#if VULKAN
layout(set = 1, binding = 0) uniform fUniformMatrix
#else
layout(std140, binding = 3) uniform fUniformMatrix
#endif
{
    fMatrix f_matrix;
};

#if VULKAN
layout(set = 1, binding = 1) uniform sampler2D tex[MAX_TEXTURES];
#else
//Unit 0 ~ 28
uniform sampler2D tex[MAX_TEXTURES];
#endif

#if VULKAN
layout(set = 1, binding = 2) uniform fUniformMaterial
#else
layout(std140, binding = 4) uniform fUniformMaterial
#endif
{
    fMaterial f_material;
};

#if VULKAN
layout(set = 1, binding = 3) uniform fDirectionalLightList
#else
layout(std140, binding = 5) uniform fDirectionalLightList
#endif
{
    fDirectionalLight directionalLightList[MAX_LIGHTS];
};

#if VULKAN
layout(set = 1, binding = 4) uniform fPointLightList
#else
layout(std140, binding = 6) uniform fPointLightList
#endif
{
    fPointLight pointLightList[MAX_LIGHTS];
};

#if VULKAN
layout(set = 1, binding = 5) uniform samplerCube irradianceMap;
layout(set = 1, binding = 6) uniform samplerCube prefilterMap;
layout(set = 1, binding = 7) uniform sampler2D brdfLUT;
#else
//Unit 29
uniform samplerCube irradianceMap;
//Unit 30
uniform samplerCube prefilterMap;
//Unit 31
uniform sampler2D brdfLUT;
#endif

#if VULKAN
layout(push_constant) uniform ActiveLights
{
    int activePointLights;
    int activeDirectionalLights;
} activeLights;
#else
uniform int activePointLights;
uniform int activeDirectionalLights;
#endif

vec3 BlinnPhong(vec3 lightPosition, vec3 lightColor, float ambientStrength, float specularStrength, bool isPointLight, int lightIndex)
{
    vec3 light_direction = vec3(0.0);
    float attenuation = 0.0;
    if (isPointLight)
    {
        light_direction = normalize(lightPosition - i_fragment_position);
        float distance = length(lightPosition - i_fragment_position);
        attenuation = 1.0 / (pointLightList[lightIndex].constant + pointLightList[lightIndex].linear * distance + pointLightList[lightIndex].quadratic * (distance * distance));
    }
    else
        light_direction = normalize(-lightPosition);

    //Ambient Lighting
    vec3 ambient = ambientStrength * lightColor;

    //Diffuse Lighting
    vec3 normal = normalize(i_normal);
    float diff = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = diff * lightColor;

    //Specular Lighting
    vec3 specular = vec3(0.0);
    if (diff > 0.0)
    {
        vec3 view_direction = normalize(i_view_position - i_fragment_position);
        // vec3 reflect_direction = reflect(-light_direction, normal);
        vec3 halfway_vector = normalize(view_direction + light_direction);

        // float spec = pow(max(dot(view_direction, reflect_direction), 0.0), f_material[i_index].shininess);
        float spec = pow(max(dot(halfway_vector, normal), 0.0), f_material.shininess);
        specular = specularStrength * spec * lightColor * f_material.specularColor;
    }

    if (isPointLight)
    {
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
    }

    return ambient + diffuse + specular;
}

// @TODO Remove this function after post-process tone mapping is implemented in OpenGL
// Reinhard Tone Mapping
vec3 ReinhardToneMapping(vec3 color)
{
    return color / (color + vec3(1.0));
}

// @TODO Remove this function after post-process tone mapping is implemented in OpenGL
// Filimic/ACES Tone Mapping
vec3 FilmicToneMapping(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

// GGX/Trowbridge-Reitz Normal Distribution Function
float D(float alpha, vec3 N, vec3 H)
{
    float numerator = pow(alpha, 2.0);

    float NdotH = max(dot(N, H), 0.0);
    float denominator = PI * pow(pow(NdotH, 2.0) * (pow(alpha, 2.0) - 1.0) + 1.0, 2.0);
    denominator = max(denominator, 0.000001);

    return numerator / denominator;
}

// Schlick-Beckmann Geometry Shadowing Function
float G1(float alpha, vec3 N, vec3 X)
{
    float numerator = max(dot(N, X), 0.0);

    float k = alpha / 2.0;
    float denominator = max(dot(N, X), 0.0) * (1.0 - k) + k;
    denominator = max(denominator, 0.000001);

    return numerator / denominator;
}

// Switch Model
float G(float alpha, vec3 N, vec3 V, vec3 L)
{
    return G1(alpha, N, V) * G1(alpha, N, L);
}

// Fresnel-Schlick Function
vec3 F(vec3 F0, vec3 V, vec3 H)
{
    return F0 + (vec3(1.0) - F0) * pow(1 - max(dot(V, H), 0.0), 5.0);
}

vec3 Froughness(vec3 F0, vec3 V, vec3 H, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - max(dot(H, V), 0.0), 0.0, 1.0), 5.0);
}

struct MainVectors
{
    vec3 albedo;
    vec3 V;
    vec3 N;
    vec3 F0;
};

// Rendering Equation for one light source
vec3 PBR(MainVectors mainVectors, vec3 lightPosition, vec3 lightColor, bool isPointLight, int lightIndex)
{
    fMaterial material = f_material;

    float ao = 1.0;
    vec3 L = isPointLight ? normalize(lightPosition - i_fragment_position) : normalize(-lightPosition);
    // Half Vector
    vec3 H = normalize(mainVectors.V + L);
    vec3 radiance = lightColor;

    vec3 Ks = F(mainVectors.F0, mainVectors.V, H);
    vec3 Kd = (1.0 - material.metallic) * (vec3(1.0) - Ks);

    vec3 lambert = mainVectors.albedo / PI;

    // Cook-Torrance BRDF
    float alpha = material.roughness * material.roughness;
    vec3 cookTorranceNumerator = D(alpha, mainVectors.N, H) * G(alpha, mainVectors.N, mainVectors.V, L) * F(mainVectors.F0, mainVectors.V, H);
    float cookTorranceDenominator = 4.0 * max(dot(mainVectors.V, mainVectors.N), 0.0) * max(dot(L, mainVectors.N), 0.0);
    cookTorranceDenominator = max(cookTorranceDenominator, 0.1);
    vec3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;

    vec3 BRDF = Kd * lambert + cookTorrance;
    vec3 outgoingLight = ao * BRDF * radiance * max(dot(L, mainVectors.N), 0.0);

    return outgoingLight;
}

void main()
{
    vec3 resultColor = vec3(0.0);

    vec3 albedo = vec3(0.0);
    if (f_matrix.isTex > 0) albedo = texture(tex[f_matrix.texIndex + i_tex_sub_index], i_uv).rgb;
    else albedo = i_col.rgb;

    vec3 F0 = mix(vec3(0.04), albedo, f_material.metallic);
    vec3 V = normalize(i_view_position - i_fragment_position);
    vec3 N = normalize(i_normal);
    MainVectors mainVectors = { albedo, V, N, F0 };

    //Calculate Directional Lights
#if VULKAN
    for (int l = 0; l < activeLights.activeDirectionalLights; ++l)
#else
    for (int l = 0; l < activeDirectionalLights; ++l)
#endif
    {
        fDirectionalLight currentLight = directionalLightList[l];
        // resultColor += clamp(BlinnPhong(currentLight.lightDirection, currentLight.lightColor, currentLight.ambientStrength, currentLight.specularStrength, false, l), 0.0, 1.0);
        resultColor += PBR(mainVectors, currentLight.lightDirection, currentLight.lightColor * currentLight.intensity, false, l);
    }

    //Calculate Point Lights
#if VULKAN
    for (int l = 0; l < activeLights.activePointLights; ++l)
#else
    for (int l = 0; l < activePointLights; ++l)
#endif
    {
        fPointLight currentLight = pointLightList[l];
        float distance = length(currentLight.lightPosition - i_fragment_position);
        if (distance > currentLight.radius) continue;
        float attenuation = max(0.0, (1.0 / (distance * distance)) - (1.0 / (currentLight.radius * currentLight.radius)));
        // resultColor += clamp(BlinnPhong(currentLight.lightPosition, currentLight.lightColor, currentLight.ambientStrength, currentLight.specularStrength, true, l), 0.0, 1.0);
        resultColor += PBR(mainVectors, currentLight.lightPosition, currentLight.lightColor * currentLight.intensity, true, l) * attenuation;
    }

    //PBR IBL Ambient Lighting
    vec3 R = reflect(-V, N);
    vec3 F = Froughness(F0, V, N, f_material.roughness);

    vec3 Ks = F;
    vec3 Kd = (1.0 - f_material.metallic) * (vec3(1.0) - Ks);
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, f_material.roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), f_material.roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    //1.0 == ao
    vec3 ambient = (Kd * diffuse + specular) * 1.0;
    resultColor = ambient + resultColor;

    // Blinn-Phong Result Color
    // if (f_matrix.isTex)
    // {
    //     vec4 textureColor = texture(tex[f_matrix.texIndex  + i_tex_sub_index], i_uv);
    //     fragmentColor = vec4(resultColor, 1.0) * textureColor;
    // }
    // else fragmentColor = vec4(resultColor, 1.0) * (i_col + 0.5);

    // PBR Result Color
    // fragmentColor = vec4(resultColor, 1.0);
    // @TODO Remove tone mapping after post-process tone mapping is implemented in Vulkan
    // Convert HDR to LDR
    resultColor = FilmicToneMapping(resultColor);
    // 2.2 Gamma Correction
    resultColor = pow(resultColor, vec3(1.0 / 2.2));
    fragmentColor = vec4(resultColor, 1.0);
}
