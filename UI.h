#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <functional>
#include <vector>
#include <algorithm>

class UIWindow
{
public:
	std::string win_name;

private:
	GLFWwindow* window;
	std::vector<std::function<void()>> layout_elements;

public:
	template<typename... Object>
	static void render(Object&&... uiwins)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		(uiwins.load_layout(),...);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	UIWindow(GLFWwindow* window, std::string name);
	~UIWindow();

	void set_layout(std::function<void()> f);

public:
	void load_layout();
};