/*
Преобразует координаты вершины из локальной системы координат в Clip Space.
Копирует цвет вершины из вершинного атрибута в выходную переменную color.
*/

#version 330


#if 1
layout(std140) uniform Matrices
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
};
#elif 0
// Uniform dummy is not optimized out:
layout(std140) uniform Matrices
{
    mat4 viewMatrix;
    float dummy;
    mat4 projectionMatrix;
};
#else
// Uniform dummy is optimized out:
layout(packed) uniform Matrices
{
    mat4 viewMatrix;
    float dummy;
    mat4 projectionMatrix;
};
#endif

uniform mat4 modelMatrix;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec4 vertexNormal;

out vec4 color;

void main()
{
    color = vertexNormal * 0.5 + 0.5;

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
}
