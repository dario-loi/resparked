#include "resparked.hpp"
#include <entt/entity/registry.hpp>
#include <iostream>
#include <format>
#include <random>
#include <exception>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "loguru.hpp"

namespace resparked {

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

engine::engine()
{

    LOG_F(INFO, "Initializing Engine");
    LOG_F(INFO, "Initializing GLFW3 Window");

    if (!glfwInit()) {
        LOG_F(ERROR, "Failed to initialize GLFW3");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

        if (window == nullptr) {
            LOG_F(ERROR, "Failed to create GLFW3 window");
            glfwTerminate();
            std::terminate();
        }

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            LOG_F(ERROR, "Failed to initialize GLAD");
        }

        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        LOG_F(INFO, "GLFW3 Window initialized");
        LOG_F(INFO, "Initializing ImGUI backend");

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param
        ImGui_ImplOpenGL3_Init(); 

        LOG_F(INFO, "ImGUI backend initialized");
    }

    engine::~engine() {
        LOG_F(INFO, "Shutting down ImGUI backend");
        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        LOG_F(INFO, "Shutting down GLFW3 Window");

        glfwTerminate();
        LOG_F(INFO, "Engine terminated correctly");
    }


    void engine::run() {
        // get current window

        LOG_F(INFO, "Starting main loop");

        auto window = glfwGetCurrentContext();

        
    }

    void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
    {
        // make sure the viewport matches the new window dimensions; note that width
        // and height will be significantly larger than specified on retina displays.
        glViewport(0, 0, width, height);
    }

} // namespace resparked