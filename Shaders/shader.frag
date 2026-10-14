#version 330 core
out vec4 colour;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
uniform sampler2D texture_data;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 candleLightPos[2];
uniform vec3 candleLightColor;
uniform vec3 viewPos;
uniform bool clipAlpha;
uniform bool emissive;

// Ambient, diffuse and Blinn-Phong specular follow Lab12.
vec3 ambientLight()
{
    return 0.3 * lightColor;
}

vec3 diffuseLight(vec3 norm, vec3 lightDir, vec3 color, float strength)
{
    float diffuse = max(dot(norm, lightDir), 0.0);
    return strength * diffuse * color;
}

vec3 specularLight(vec3 norm, vec3 lightDir, vec3 viewDir, vec3 color,
                   float strength, float shininess)
{
    vec3 halfDir = normalize(lightDir + viewDir);
    float specular = pow(max(dot(norm, halfDir), 0.0), shininess);
    return strength * specular * color;
}

vec3 candleLight(vec3 norm, vec3 viewDir, int index)
{
    vec3 candleDir = candleLightPos[index] - FragPos;
    float distanceToCandle = length(candleDir);
    candleDir = normalize(candleDir);
    float attenuation = 1.0 / (1.0 + 0.09 * distanceToCandle
                                + 0.032 * distanceToCandle * distanceToCandle);

    vec3 diffuse = diffuseLight(norm, candleDir, candleLightColor, 0.9);
    vec3 specular = specularLight(
        norm, candleDir, viewDir, candleLightColor, 0.15, 64.0
    );
    return attenuation * (diffuse + specular);
}

vec3 candleLights(vec3 norm, vec3 viewDir)
{
    vec3 lighting = vec3(0.0);
    for (int i = 0; i < 2; ++i) {
        lighting += candleLight(norm, viewDir, i);
    }
    return lighting;
}

void main()
{
    vec4 texColor = texture(texture_data, TexCoord);
    if (clipAlpha && texColor.a < 0.1) discard;

    vec3 baseColor = texColor.rgb * objectColor;
    if (emissive) {
        colour = vec4(baseColor, 1.0);
        return;
    }

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lighting = ambientLight()
                  + diffuseLight(norm, lightDir, lightColor, 0.5)
                  + specularLight(norm, lightDir, viewDir, lightColor, 0.2, 64.0)
                  + candleLights(norm, viewDir);

    colour = vec4(baseColor * lighting, 1.0);
}
