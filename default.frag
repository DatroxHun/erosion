/*#version 460 core
out vec4 FragColor;
void main()
{
   FragColor = vec4(0.8f, 0.3f, 0.02f, 1.0f);
}*/

#version 460 core
out vec4 glFragColor;

layout(rgba32f, binding = 1) uniform image2D imgInput;
	
uniform sampler2D img;
uniform ivec2 img_dim;
	
void main()
{             
    vec3 texCol = imageLoad(imgInput, ivec2(gl_FragCoord.xy)).xyz;     
    glFragColor = vec4(texCol, 1.0);
}