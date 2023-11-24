#include "resparked.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <concepts>
#include <entt/entity/registry.hpp>
#include <exception>
#include <format>
#include <iostream>
#include <random>

#include <btBulletDynamicsCommon.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#include "loguru.hpp"

#include "staplegl.hpp"

namespace resparked {

template <typename T, size_t S>
struct ring_buffer {
    std::array<T, S> buf;
    size_t curr_idx { 0 };

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

engine::~engine()
{
    LOG_F(INFO, "Shutting down ImGUI backend");
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    LOG_F(INFO, "Shutting down GLFW3 Window");

    glfwTerminate();
    LOG_F(INFO, "Engine terminated correctly");
}

void engine::run()
{
    // get current window

    using namespace std::literals::string_literals;
    using namespace staplegl::shader_data_type;

    // data for frametime statistics, 1kb~
    ring_buffer<float, 128> ring;
    std::array<float, 128> fps_arr;

    std::fill(ring.buf.begin(), ring.buf.end(), 0.F);

    LOG_F(INFO, "Starting main loop");

    auto window = glfwGetCurrentContext();
    auto context = entt::registry(); // spawn registry that holds all entities
    auto lastTime = std::chrono::high_resolution_clock::now();

    staplegl::vertex_buffer_layout layout_2P { { u_type::vec2,
        "pos2" } };

    auto str = layout_2P.stride_elements();

    std::array<float, 8> quad_strip {
        -0.2F, 0.2F, -0.2F, -0.2F, 0.2F, 0.2F, 0.2F, -0.2F
    };

    staplegl::vertex_buffer vb_2P { quad_strip };
    vb_2P.set_layout(layout_2P);

    staplegl::index_buffer ib_2P {
        std::array<std::uint32_t, 4> { 0, 1, 2, 3 }
    };

    staplegl::vertex_array va_2P;

    va_2P.add_vertex_buffer(std::move(vb_2P));
    va_2P.set_index_buffer(std::move(ib_2P));

    staplegl::shader_program shader_2P { "basic", "shaders/basic.glsl" };

    shader_2P.bind();
    va_2P.bind();

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
        ImGuiIO& io = ImGui::GetIO();

        // get framebuf size for this frame
        int frameWidth;
        int frameHeight;
        glfwGetWindowSize(window, &frameWidth, &frameHeight);

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*

            Rendercalls BEGIN

        */

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        /*

             Rendercalls END

         */

        /*

            Frametime statistics BEGIN

        */

        ImGui::Begin("Frames per second", nullptr);

        ImGui::SetWindowSize({ 360.f, 80.f });
        ImGui::SetWindowPos({ static_cast<float>(frameWidth) - 360.f - 10.f, 10.f });

        auto currTime = std::chrono::high_resolution_clock::now();

        auto delta = currTime - lastTime;
        auto frametime = std::chrono::duration<float>(delta).count();

        ring.push(frametime);

        auto avg = 0.F;
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