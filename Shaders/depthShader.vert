#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
out vec3 FragPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    vec4 worldPosition = model * vec4(pos, 1.0);
    FragPos = worldPosition.xyz;
    TexCoord = aTexCoord;
    gl_Position = projection * view * worldPosition;
}
