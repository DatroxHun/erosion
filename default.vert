/*#version 460 core
layout (location = 0) in vec3 aPos;
void main()
{
   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}*/

#version 460 core
layout (location = 0) in vec3 vert_pos;

void main()
{
    gl_Position = vec4(vert_pos, 1.0);
}