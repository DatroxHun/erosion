#include "UI.h"

bool is_imgui_initialized = false;

UIWindow::UIWindow(GLFWwindow* window, std::string win_name)
{
	this->window = window;
	this->win_name = win_name;

	if (!is_imgui_initialized)
	{
		is_imgui_initialized = true;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 430");
	}
}

void UIWindow::set_layout(std::function<void()> f)
{
	layout_elements.push_back(f);
}

void UIWindow::load_layout()
{
	ImGui::Begin(win_name.c_str());
	
	std::for_each(layout_elements.begin(), layout_elements.end(), [](std::function<void()> func) {
		func();
	});

	ImGui::End();
}

UIWindow::~UIWindow()
{

}