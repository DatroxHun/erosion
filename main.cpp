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
#include "subDivQuadClass.h"

#include "camera.h"

using namespace std::chrono;

// constants
#define WIDTH 800
#define HEIGHT 800
#define RES 512 // should be a power of 2 (128 works pretty well)

// global variables
Camera cam;
double dt;

int blur_intensity = 0;
float frame_tick_duration = .25f;

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

	if (ImGui::GetIO().WantCaptureMouse) return;

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

	Quad *quad = new Quad(RES);
	ComputeShader heightMapShader("height_map.comp");
	ComputeShader blurShader("blur.comp");

	//GLfloat vertices[3 * (RES + 1) * (RES + 1)];
	//GLuint indices[6 * RES * RES];

	// uniform initialization
	GLint seedLocation = glGetUniformLocation(heightMapShader.ID, "seed");
	GLint blurIntensityLocation = glGetUniformLocation(blurShader.ID, "blur_intensity");


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
	heightMapShader.Activate();
	glUniform2i(seedLocation, rand() % 1000, rand() % 1000);
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

			if (glfwGetTime() - frameTime > frame_tick_duration)
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

		// compute shader dispatching
		blurShader.Activate();
		glUniform1i(blurIntensityLocation, blur_intensity);

		glDispatchCompute((GLuint)(RES / 16), (GLuint)(RES / 16), 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


		// subdivided QUAD rendering
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		quad->render(cam);


		// UI rendering
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings");
		ImGui::SliderInt("Blur Kernel Size", &blur_intensity, 0, 10);
		ImGui::InputFloat("Frame Tick Duration (s)", &frame_tick_duration, 0.025, .1);
		frame_tick_duration = clamp(frame_tick_duration, .025f, 1.f);
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

	delete quad;
	heightMapShader.Delete();
	blurShader.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}