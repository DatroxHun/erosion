#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include <stdlib.h>

#include "shaderClass.h"
#include "computeClass.h"
#include "VBO.h"
#include "VAO.h"

using namespace std::chrono;

#define WIDTH 800
#define HEIGHT 800
#define FRAME_TICK_DURATION .2

// vertex coordinates
GLfloat quadVertices[] = {
	-1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,

	-1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 0.0f
};


int main()
{
	// initialize GLFW
	glfwInit();

	// tell GLFW what version of OpenGL we are using: 4.6
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	// tell GLFW that we use the core profile of OpenGl -> only modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create window with dim of WIDTH and HEIGHT, called "erosion" and not fullscreen
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "erosion", NULL, NULL);

	// error checking
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window!" << std::endl;
		glfwTerminate();
		return -1;
	}

	// tell GLFW to open window -> introduce into current condext
	glfwMakeContextCurrent(window);
	
	// turn off vsync
	//glfwSwapInterval(0);



	// load GLAD so it can configure OpenGL
	gladLoadGL();
	// specify OpenGl's viewport in window (from (0; 0) to (WIDTH; HEIGHT))
	glViewport(0, 0, WIDTH, HEIGHT);

	Shader shaderProgram("default.vert", "default.frag");
	ComputeShader computeShader("default.comp");

	// VBO and VAO stuff
	VAO vao;
	vao.Bind();

	VBO vbo(quadVertices, sizeof(quadVertices));

	vao.LinkVBO(vbo, 0);
	vao.Unbind();
	vbo.Unbind();

	// uniform initialization
	GLint tLocation = glGetUniformLocation(computeShader.ID, "t");

	GLint imgLocation = glGetUniformLocation(shaderProgram.ID, "img");
	GLint imgDimLocation = glGetUniformLocation(shaderProgram.ID, "img_dim");


	// initialize Image
	GLuint texture;

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// fps counter initialization
	double dt, currentTime, lastTime = glfwGetTime(), frameTime = glfwGetTime();

	// main while loop
	while (!glfwWindowShouldClose(window))
	{
		// time related calculations
		{
			currentTime = glfwGetTime();
			dt = currentTime - lastTime;
			lastTime = currentTime;

			if (glfwGetTime() - frameTime > FRAME_TICK_DURATION)
			{
				printf("\r%.1f fps     ", 1.0 / dt);
				frameTime = glfwGetTime();
			}
		}

		

		// Compute Shader dispatching
		computeShader.Activate();
		glUniform1f(tLocation, (float)fmod(currentTime, 100.0));

		glDispatchCompute((GLuint)WIDTH, (GLuint)HEIGHT, 1);

		// make sure writing to image has finished before read
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



		// QUAD rendering
		glClear(GL_DEPTH_BUFFER_BIT);

		// set our shaderProgram the current Program
		shaderProgram.Activate();
		vao.Bind();

		glUniform1i(imgLocation, 0);
		glUniform2i(imgDimLocation, WIDTH, HEIGHT);

		// object type, starting index of vertex array, number of vertecies)
		glDrawArrays(GL_TRIANGLES, 0, 6);



		// swap buffers and handle all GLFW events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// freeing up memory
	vao.Delete();
	vbo.Delete();
	shaderProgram.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}