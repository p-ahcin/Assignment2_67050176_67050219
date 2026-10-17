#version 330 core
out vec4 colour;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

const int LIGHT_COUNT = 4;
uniform vec3 lightPositions[LIGHT_COUNT];
uniform float lightStrengths[LIGHT_COUNT];
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float ambientStrength;
uniform float attenuationLinear;
uniform float attenuationQuadratic;
uniform float farPlane;
uniform samplerCube shadowMaps[LIGHT_COUNT];
uniform sampler2D texture_data;
uniform vec3 materialColor;
uniform bool clipAlpha;

// GLSL 3.30 requires constant indices for sampler arrays.
float ReadShadowDepth(int light, vec3 direction)
{
    if (light == 0) return texture(shadowMaps[0], direction).r;
    if (light == 1) return texture(shadowMaps[1], direction).r;
    if (light == 2) return texture(shadowMaps[2], direction).r;
    return texture(shadowMaps[3], direction).r;
}

float CandleShadow(int light, vec3 norm, vec3 lightDir)
{
    vec3 fromLight = FragPos - lightPositions[light];
    float currentDepth = length(fromLight);
    if (currentDepth >= farPlane || currentDepth < 0.8)
        return 0.0;
    float bias = max(0.06 * (1.0 - dot(norm, lightDir)), 0.025);
    vec3 offsets[7] = vec3[](
        vec3(0), vec3(1,0,0), vec3(-1,0,0), vec3(0,1,0),
        vec3(0,-1,0), vec3(0,0,1), vec3(0,0,-1)
    );
    float radius = 0.15 + currentDepth * 0.003;
    float shadow = 0.0;
    for (int sampleIndex = 0; sampleIndex < 7; ++sampleIndex)
    {
        float depth = ReadShadowDepth(light, fromLight + offsets[sampleIndex] * radius) * farPlane;
        shadow += currentDepth - bias > depth ? 1.0 : 0.0;
    }
    return shadow / 7.0;
}

vec3 diffuseLight(vec3 norm, vec3 lightDir)
{
    return lightColor * 0.8 * max(dot(norm, lightDir), 0.0);
}

vec3 specularLight(vec3 norm, vec3 lightDir, vec3 viewDir)
{
    if (dot(norm, lightDir) <= 0.0)
        return vec3(0);
    vec3 halfDir = normalize(lightDir + viewDir);
    return lightColor * 0.18 * pow(max(dot(norm, halfDir), 0.0), 64.0);
}

void main()
{
    vec4 texColor = texture(texture_data, TexCoord);
    if (clipAlpha && texColor.a < 0.5)
        discard;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    // A faint neutral fill keeps details visible away from the candles.
    vec3 lighting = vec3(ambientStrength);
    for (int light = 0; light < LIGHT_COUNT; ++light)
    {
        vec3 toLight = lightPositions[light] - FragPos;
        float distanceToLight = max(length(toLight), 0.0001);
        vec3 lightDir = toLight / distanceToLight;
        float attenuation = 1.0 / (1.0 + attenuationLinear * distanceToLight +
                                    attenuationQuadratic * distanceToLight * distanceToLight);
        float shadow = CandleShadow(light, norm, lightDir);
        lighting += (1.0 - shadow) * attenuation * lightStrengths[light] *
                    (diffuseLight(norm, lightDir) + specularLight(norm, lightDir, viewDir));
    }
    colour = vec4(texColor.rgb * materialColor * lighting, 1.0);
}
