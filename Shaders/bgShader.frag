#version 330 core
// Lab13 reference only. The platform now uses shader.frag with the whole scene.

in vec4 FragPosLightSpace;
in vec2 TexCoord;

out vec4 colour;

uniform vec3 bgColor;
uniform sampler2D shadowMap;
uniform sampler2D texture_data;

float ShadowCalculatoin()
{
    // perspective
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projCoords = projCoords * 0.5f + 0.5f;

    if (projCoords.z < 0.0f || projCoords.z > 1.0f)
        return 0.0f;

    float closesDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = 0.005f;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
    }
    shadow /= 9.0;

    // float shadow = currentDepth > closesDepth ? 1.0f : 0.0f;
    return shadow; 
}

void main()
{
    float shadow = ShadowCalculatoin();
    colour = texture(texture_data, TexCoord) * vec4((1.0 - shadow) * bgColor, 1.0f);
}
