#define M_2_PI 1.5707963267948965580
#define M_PI  3.1415926535897931160
#define M_PI2 6.2831853071795862320

vec3 fresnel(float cosTheta, vec3 F0)
{
    return F0 + (1 - F0) * pow(clamp(1 - cosTheta, 0, 1), 5);
}  

float GGX(float cosNH, float alpha)
{
    float alpha2 = alpha*alpha;
    float cos2 = cosNH;
    float denom = cos2 * (alpha2 - 1) + 1;
    return alpha2 / (M_PI * denom * denom);
}

float GeometrySchlickGGX(float cosNV, float roughness)
{
    float r = roughness + 1.0;
    float k = r*r / 8.0;
    return cosNV / (cosNV * (1.0 - k) + k);
}
float GeometrySmith(float cosNV, float cosNL, float roughness)
{
    return GeometrySchlickGGX(cosNV, roughness) * GeometrySchlickGGX(cosNL, roughness);
}

vec3 BRDF(vec3 H, vec3 V, vec3 N, vec3 L, float roughness, vec3 albedo, vec3 radiance)
{
        float alpha = roughness*roughness;
        float cosHV = max(0, dot(H,V));
        float cosNH = max(0, dot(N,H));
        float cosNL = max(0, dot(N,L));
        float cosNV = max(0, dot(N,V));
        vec3 F = fresnel(cosHV, vec3(0.04));
        float D = GGX(cosNH, alpha);
        float G = GeometrySmith(cosNV, cosNL, roughness); 
        vec3 specular = D*G*F / (4.0 * cosNV * cosNL  + 0.0001);  
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        return (kD * albedo / M_PI + specular) * radiance * cosNL;
}