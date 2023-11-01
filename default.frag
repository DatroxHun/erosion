/*#version 460 core
out vec4 FragColor;
void main()
{
   FragColor = vec4(0.8f, 0.3f, 0.02f, 1.0f);
}*/

#version 460 core

in vec3 worldPos;
in int frag_resolution;

out vec4 glFragColor;

layout(rgba32f, binding = 1) uniform image2D imgInput;

void main()
{             
    vec3 texCol = imageLoad(imgInput, ivec2((worldPos.xz + vec2(1.0)) / 2.0 * 800.0)).xyz;
    glFragColor = vec4(texCol, 1.0);
}