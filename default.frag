/*#version 460 core
out vec4 FragColor;
void main()
{
   FragColor = vec4(0.8f, 0.3f, 0.02f, 1.0f);
}*/

#version 460 core

in vec3 worldPos;
out vec4 glFragColor;

uniform int resolution;

layout(rgba32f, binding = 1) uniform image2D imgInput;

void main()
{             
    ivec2 sample_point = ivec2(floor((worldPos.xz + vec2(1.0)) / 2.0 * (resolution - .001)));
    //vec2 interpolation = fract((worldPos.xz + vec2(1.0)) / 2.0 * resolution);
    vec2 interpolation = smoothstep(0.0, 1.0, fract((worldPos.xz + vec2(1.0)) / 2.0 * resolution));

    float texCol00 = imageLoad(imgInput, min(sample_point + ivec2(0, 0), ivec2(resolution - 1))).x;
    float texCol01 = imageLoad(imgInput, min(sample_point + ivec2(0, 1), ivec2(resolution - 1))).x;
    float texCol10 = imageLoad(imgInput, min(sample_point + ivec2(1, 0), ivec2(resolution - 1))).x;
    float texCol11 = imageLoad(imgInput, min(sample_point + ivec2(1, 1), ivec2(resolution - 1))).x;

    float interCol0 = texCol00 * (1. - interpolation.y) + texCol01 * interpolation.y;
    float interCol1 = texCol10 * (1. - interpolation.y) + texCol11 * interpolation.y;

    float out_color = interCol0 * (1. - interpolation.x) + interCol1 * interpolation.x;

    glFragColor = vec4(vec3(out_color), 1.0);
    //glFragColor = vec4(fract((worldPos.xz + vec2(1.0)) / 2.0 * resolution), 0.0, 1.0);
}