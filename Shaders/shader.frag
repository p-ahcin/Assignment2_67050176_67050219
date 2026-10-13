#version 330 core
out vec4 colour;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
uniform sampler2D texture_data;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool clipAlpha;
// Ambient, diffuse and Blinn-Phong specular follow Lab12.
void main()
{
    vec4 texColor = texture(texture_data, TexCoord);
    if (clipAlpha && texColor.a < 0.1) discard;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    vec3 ambient = 0.3 * lightColor;
    vec3 diffuse = 0.5 * max(dot(norm, lightDir), 0.0) * lightColor;
    vec3 specular = 0.2 * pow(max(dot(norm, halfDir), 0.0), 64.0) * lightColor;
    colour = vec4(texColor.rgb * objectColor * (ambient + diffuse + specular), 1.0);
}
