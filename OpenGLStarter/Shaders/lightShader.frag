// สีของ obj แสงนี้
#version 330 core

out vec4 colour;

uniform vec3 lightColor;

void main()
{
    colour = vec4(lightColor, 1.0f);
}
