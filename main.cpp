#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <chrono>
#include <stdlib.h>

#include "shaderClass.h"
#include "computeClass.h"
#include "VBO.h"
#include "EBO.h"
#include "VAO.h"

#include "camera.h"

using namespace std::chrono;

// constants
#define WIDTH 800
#define HEIGHT 800
#define RES 128 // should be a power of 2 (128 works pretty well)
#define FRAME_TICK_DURATION .25

// vertex coordinates
void get_sub_div_quad_vertices(GLfloat vertices[], int res)
{
	int r_res = res + 1;

	for (int i = 0; i < r_res; i++)
	{
		for (int j = 0; j < r_res; j++)
		{
			vertices[3 * (i + j * r_res) + 0] = -1.0 + 2.0 * i / (GLfloat)res; // x
			vertices[3 * (i + j * r_res) + 1] = 0.0/*(i + j) / (GLfloat)(4.0 * res)*/; // y
			vertices[3 * (i + j * r_res) + 2] = -1.0 + 2.0 * j / (GLfloat)res; // z
		}
	}
}

void get_sub_div_quad_indices(GLuint indices[], int res)
{
	int r_res = res + 1;

	for (GLuint i = 0; i < res; i++)
	{
		for (GLuint j = 0; j < res; j++)
		{
			indices[6 * (i + j * res) + 0] = i + j * r_res;
			indices[6 * (i + j * res) + 1] = (i + 1) + j * r_res;
			indices[6 * (i + j * res) + 2] = (i + 1) + (j + 1) * r_res;
			indices[6 * (i + j * res) + 3] = i + j * r_res;
			indices[6 * (i + j * res) + 4] = (i + 1) + (j + 1) * r_res;
			indices[6 * (i + j * res) + 5] = i + (j + 1) * r_res;
		}
	}
}

GLfloat quadVertices[] = {
	-1.f, 0.0f, 1.0f,
	-1.f, 0.0f, -1.f,
	1.0f, 0.0f, -1.f,
	1.0f, 0.0f, 1.0f
};

GLuint quadIndices[] = {
	0, 1, 2,
	0, 2, 3
};

// global variables
Camera cam;
int blur_intensity = 0;
double dt;

// mouse callbacks
bool left_button_down = false;
double last_mouse_x = WIDTH / 2, last_mouse_y = HEIGHT / 2;
float mouse_sensitivity = 2.;
double mouse_dx, mouse_dy;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

	mouse_dx = xpos - last_mouse_x;
	mouse_dy = last_mouse_y - ypos; // reversed since y-coordinates range from bottom to top
	last_mouse_x = xpos;
	last_mouse_y = ypos;
}
static void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (GLFW_PRESS == action)
			left_button_down = true;
		else if (GLFW_RELEASE == action)
			left_button_down = false;
	}
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//cam.FOV *= 1. - yoffset / 10.;
	cam.update_polar(0.0, 0.0, -yoffset * .25);
	std::cout << cam.pos.x << " " << cam.pos.y << " " << cam.pos.z;
}

int main()
{
	// initialize GLFW
	glfwInit();

	// tell GLFW what version of OpenGL we are using: 4.6
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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

	glEnable(GL_DEPTH_TEST);

	//Cam
	cam = Camera(vec3(3., 2., .0), vec3(.0), vec3(0., 1., 0.));

	Shader shaderProgram("default.vert", "default.frag");
	ComputeShader noiseShader("default.comp");
	ComputeShader blurShader("post.comp");

	GLfloat vertices[3 * (RES + 1) * (RES + 1)];
	GLuint indices[6 * RES * RES];

	get_sub_div_quad_vertices(vertices, RES);
	get_sub_div_quad_indices(indices, RES);

	// VBO and VAO stuff
	VAO vao;
	vao.Bind();

	VBO vbo(vertices, sizeof(vertices));
	EBO ebo(indices, sizeof(indices));

	vao.LinkVBO(vbo, 0);
	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();

	// uniform initialization
	GLint tLocation = glGetUniformLocation(noiseShader.ID, "t");

	GLint blurIntensityLocation = glGetUniformLocation(blurShader.ID, "blur_intensity");

	GLint resolutionLocation = glGetUniformLocation(shaderProgram.ID, "resolution");
	GLint camMatLocation = glGetUniformLocation(shaderProgram.ID, "cam_mat");


	// initialize Image
	GLuint texture0, texture1;

	// id 0 image
	glGenTextures(1, &texture0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, RES, RES, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindImageTexture(0, texture0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// id 1 image
	glGenTextures(1, &texture1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, RES, RES, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindImageTexture(1, texture1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// fps counter initialization
	double currentTime, lastTime = glfwGetTime(), frameTime = glfwGetTime();

	// UI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

	// callbacks
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// terrain generation
	noiseShader.Activate();
	glDispatchCompute((GLuint)(RES / 16), (GLuint)(RES / 16), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

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

		// camera
		if (left_button_down)
		{
			cam.update_polar(mouse_dx / HEIGHT * mouse_sensitivity, -mouse_dy / HEIGHT * mouse_sensitivity);
		}
		else
		{
			cam.update_polar(dt * .025, 0.0);
		}
		mouse_dx = 0;
		mouse_dy = 0;

		// Compute Shader dispatching
		blurShader.Activate();
		glUniform1i(blurIntensityLocation, blur_intensity);

		glDispatchCompute((GLuint)(RES / 16), (GLuint)(RES / 16), 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



		// subdivided QUAD rendering
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// set our shaderProgram the current Program
		shaderProgram.Activate();
		vao.Bind();

		glUniform1i(resolutionLocation, RES);
		glUniformMatrix4fv(camMatLocation, 1, GL_FALSE, value_ptr(cam.get_matrix()));

		// object type, starting index of vertex array, number of vertecies)
		//glDrawArrays(GL_TRIANGLES, 0, 6);

		// object type, indices count, indices data type, index of indices
		glDrawElements(GL_TRIANGLES, 6 * RES * RES, GL_UNSIGNED_INT, 0);


		// UI rendering
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings");
		ImGui::SliderInt("Blur Kernel Size", &blur_intensity, 0, 10);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		// swap buffers and handle all GLFW events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// freeing up memory
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	vao.Delete();
	vbo.Delete();
	ebo.Delete();
	shaderProgram.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}