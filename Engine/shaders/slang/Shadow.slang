float Hamburger4MSM(float4 b, float z_f)
{
    float alpha = 1.0e-3;
    float4 b_prime = (1.0 - alpha) * b + alpha * float4(0.5, 0.5, 0.5, 0.5);

    float L22 = sqrt(max(b_prime.y - b_prime.x * b_prime.x, 0.00001));
    float L32 = (b_prime.z - b_prime.y * b_prime.x) / L22;
    float L33 = sqrt(max(b_prime.w - b_prime.y * b_prime.y - L32 * L32, 0.00001));

    float y1 = 1.0;
    float y2 = (z_f - b_prime.x) / L22;
    float y3 = (z_f * z_f - b_prime.y - L32 * y2) / L33;

    float c3 = y3 / L33;
    float c2 = (y2 - L32 * c3) / L22;
    float c1 = 1.0 - b_prime.x * c2 - b_prime.y * c3;

    float discriminant = max(0.0, c2 * c2 - 4.0 * c3 * c1);
    float sqrtDisc = sqrt(discriminant);

    float rootA = (-c2 - sqrtDisc) / (2.0 * c3);
    float rootB = (-c2 + sqrtDisc) / (2.0 * c3);

    float z2 = min(rootA, rootB);
    float z3 = max(rootA, rootB);

    float G = 0.0;
    if (z_f <= z2) G = 0.0;
    else if (z_f <= z3) G = (z_f * z3 - b_prime.x * (z_f + z3) + b_prime.y) / ((z3 - z2) * (z_f - z2));
    else G = 1.0 - (z2 * z3 - b_prime.x * (z2 + z3) + b_prime.y) / ((z_f - z2) * (z_f - z3));

    return G;
}
