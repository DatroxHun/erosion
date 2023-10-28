/*#version 460 core
layout (location = 0) in vec3 aPos;
void main()
{
   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}*/

#version 460 core
layout (location = 0) in vec3 vert_pos;
layout(rgba32f, binding = 1) uniform image2D imgInput;

uniform int resolution;
uniform mat4 cam_mat;

void main()
{
    ivec2 sample_point = ivec2((vert_pos.xz + vec2(1.0)) / 2.0 * resolution);
    vec3 alt_vert_pos = vert_pos + vec3(0.0, imageLoad(imgInput, sample_point).x, 0.0);

    gl_Position = cam_mat * vec4(alt_vert_pos, 1.0);
}