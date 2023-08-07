#include "resparked.hpp"
#include <entt/entity/registry.hpp>
#include <iostream>
#include <format>
#include <random>
#include <exception>
#include <chrono>
#include <concepts>
#include <array>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#include "loguru.hpp"

namespace resparked {

template <typename T, size_t S>
struct ring_buffer {
  std::array<T, S> buf;
  size_t curr_idx{0};

  void push(T element) { buf[(curr_idx++) % S] = element; }
};

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

        using namespace std::literals::string_literals;

        // data for frametime statistics, 1kb~
        ring_buffer<float, 128> ring;
        std::array<float, 128> fps_arr;

        LOG_F(INFO, "Starting main loop");

        auto window = glfwGetCurrentContext();
        auto context = entt::registry(); // spawn registry that holds all entities
        auto lastTime = std::chrono::high_resolution_clock::now();

        for (;;) // continuously draw frames,
        {
            if (glfwWindowShouldClose(window)) {
                break;
            }

            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // get ImGui io context to query stuff such as resolution
            ImGuiIO &io = ImGui::GetIO();

            // get framebuf size for this frame
            int frameWidth;
            int frameHeight;
            glfwGetWindowSize(window, &frameWidth, &frameHeight);

            /*
            
                Frametime statistics BEGIN
            
            */

            ImGui::Begin("Frames per second", nullptr);

            ImGui::SetWindowSize({360, 80});
            ImGui::SetWindowPos({frameWidth - 360 - 10, 10});

            auto currTime = std::chrono::high_resolution_clock::now();

            auto delta = currTime - lastTime;
            auto frametime = std::chrono::duration<float>(delta).count();

            ring.push(frametime);

            auto avg = 0;
            for (auto t : ring.buf) {
                avg += t;
            }
            avg /= ring.buf.size();
            lastTime = currTime;
            std::transform(ring.buf.begin(), ring.buf.end(), fps_arr.begin(),
                           [](float ft) { return 1 / (ft + 1e-9); });

            ImGui::PlotLines(std::format("frametime {:.2f}ms\nfps: {:.2f}",
                                         frametime, 1 / frametime)
                                 .c_str(),
                             fps_arr.data(), fps_arr.size(), 0, nullptr, 0.f,
                             120.f);
            ImGui::End();

            /*
            
                Frametime statistics END
            
            */

            ImGui::Render();

            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }
    }

    void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
    {
        // make sure the viewport matches the new window dimensions; note that width
        // and height will be significantly larger than specified on retina displays.
        glViewport(0, 0, width, height);
    }

} // namespace resparked