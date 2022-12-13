#pragma once

// imgui [GUI drawer]
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

// opengl [window drawer]
#include "glad/glad.h"
#include "GLFW/glfw3.h"

// std library
#include <cmath>
#include <iostream>

// Windows
#include <windows.h>

namespace utility
{
    GLFWwindow *window = nullptr;

    const char *GLSL_VERSION = "#version 330";

    const int ScreenBaseResolution = 1920;
    const int WindowBaseWidth = 1280;
    const int WindowBaseHeight = 720;
    float WindowScaleFactor = 1.5f;

    void RescaleWindow()
    {
        HDC hdc = GetDC(NULL);
        int windowWidth = GetDeviceCaps(hdc, DESKTOPHORZRES);
        int windowHeight = GetDeviceCaps(hdc, DESKTOPVERTRES);
        ReleaseDC(NULL, hdc);
        std::cout << "window width:" << windowWidth << std::endl;
        std::cout << "window height:" << windowHeight << std::endl;

        WindowScaleFactor *= static_cast<float>(windowWidth) / static_cast<float>(ScreenBaseResolution);
        std::cerr << "window scale factor:" << WindowScaleFactor << std::endl;
    }

    // inner function
    static int GLFWInit()
    {
        if (!glfwInit())
            return -1;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(WindowBaseWidth * WindowScaleFactor, WindowBaseHeight * WindowScaleFactor, "Porch 配置助手", NULL, NULL);
        if (window == NULL)
        {
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        return 0;
    }

    // inner function
    static int GLADInit()
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            return -2;

        return 0;
    }

    int WindowInit()
    {
        return GLFWInit() + GLADInit();
    }

    void GUIInit()
    {
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(GLSL_VERSION);
        ImGui::GetStyle().ScaleAllSizes(WindowScaleFactor);
    }

    void GUILoadFont()
    {
        ImGuiIO &io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/simhei.ttf",
                                     13.0f,
                                     nullptr,
                                     io.Fonts->GetGlyphRangesChineseFull());
        io.FontGlobalScale = WindowScaleFactor;
    }

    bool WindowShouldClose()
    {
        return glfwWindowShouldClose(window);
    }

    // inner function
    static void WindowPollEvent()
    {
        glfwPollEvents();
    }

    // inner function
    static void WindowBackgroundUpdate()
    {
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void WindowNewFrame()
    {
        WindowPollEvent();
        WindowBackgroundUpdate();
    }

    void WindowEndFrame()
    {
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glfwSwapBuffers(window);
    }

    void GUINewFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void GUIDrawFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void GUIDestroy()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void WindowDestroy()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    int ceiling(float num, int significance)
    {
        if (num < 0 or significance < 0)
            return -1;
        if (num < significance)
            return significance;

        int integerNum = std::ceil(num);
        int offset = significance - (integerNum % significance);

        return integerNum + offset;
    }
}
