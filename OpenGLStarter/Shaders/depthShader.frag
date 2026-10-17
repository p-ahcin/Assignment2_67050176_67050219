#version 330 core
in vec2 TexCoord;
in vec3 FragPos;
uniform sampler2D texture_data;
uniform bool clipAlpha;
uniform vec3 pointLightPos;
uniform float farPlane;
void main()
{
    if (clipAlpha && texture(texture_data, TexCoord).a < 0.5)
        discard;
    float lightDistance = length(FragPos - pointLightPos);
    // The flame's own small mesh must not block the light inside it.
    if (lightDistance < 0.8)
        discard;
    gl_FragDepth = lightDistance / farPlane;
}
