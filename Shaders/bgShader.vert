#version 330 core
// Lab13 reference only. The platform now uses shader.vert with the whole scene.

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;

out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 lightProjection;
uniform mat4 lightView;

void main()
{
    TexCoord = aTexCoord;
    vec3 FragPos = vec3(model * vec4(pos, 1.0));
    FragPosLightSpace = lightProjection * lightView * vec4(FragPos,1.0f);
    gl_Position = projection * view * model * vec4(pos, 1.0);
}
