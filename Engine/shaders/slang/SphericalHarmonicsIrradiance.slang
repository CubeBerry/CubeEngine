#define PI 3.14159265358979323846f

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 uvw : TEXCOORD0;
};

struct SHCoefficients
{
    float4 E_lm[9];
};
ConstantBuffer<SHCoefficients> shBuffer : register(b1, space0);

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target
{
    float3 N = normalize(float3(input.uvw.x, input.uvw.y, input.uvw.z));
    float x = N.x;
    float y = N.y;
    float z = N.z;

    // Z-axis points up, Y-axis points forward, X-axis points right
    // float Y00 = 0.5 * sqrt(1.0 / PI);
    // float Y1_1 = (0.5 * sqrt(3.0 / PI)) * y;
    // float Y10 = (0.5 * sqrt(3.0 / PI)) * z;
    // float Y11 = (0.5 * sqrt(3.0 / PI)) * x;
    // float Y2_2 = (0.5 * sqrt(15.0 / PI)) * x * y;
    // float Y2_1 = (0.5 * sqrt(15.0 / PI)) * y * z;
    // float Y20 = (0.25 * sqrt(5.0 / PI)) * (3.0 * z * z - 1.0);
    // float Y21 = (0.5 * sqrt(15.0 / PI)) * x * z;
    // float Y22 = (0.25 * sqrt(15.0 / PI)) * (x * x - y * y);

    // Reorder to match the cubemap's coordinate system where Y-axis points up, Z-axis points forward, X-axis points right
    float Y00 = 0.5 * sqrt(1.0 / PI);
    float Y1_1 = (0.5 * sqrt(3.0 / PI)) * z;
    float Y10 = (0.5 * sqrt(3.0 / PI)) * y;
    float Y11 = (0.5 * sqrt(3.0 / PI)) * x;
    float Y2_2 = (0.5 * sqrt(15.0 / PI)) * x * z;
    float Y2_1 = (0.5 * sqrt(15.0 / PI)) * z * y;
    float Y20 = (0.25 * sqrt(5.0 / PI)) * (3.0 * y * y - 1.0);
    float Y21 = (0.5 * sqrt(15.0 / PI)) * x * y;
    float Y22 = (0.25 * sqrt(15.0 / PI)) * (x * x - z * z);

    float3 irradiance =
        shBuffer.E_lm[0].rgb * Y00 +
        shBuffer.E_lm[1].rgb * Y1_1 +
        shBuffer.E_lm[2].rgb * Y10 +
        shBuffer.E_lm[3].rgb * Y11 +
        shBuffer.E_lm[4].rgb * Y2_2 +
        shBuffer.E_lm[5].rgb * Y2_1 +
        shBuffer.E_lm[6].rgb * Y20 +
        shBuffer.E_lm[7].rgb * Y21 +
        shBuffer.E_lm[8].rgb * Y22;

    return float4(irradiance, 1.0);
}
