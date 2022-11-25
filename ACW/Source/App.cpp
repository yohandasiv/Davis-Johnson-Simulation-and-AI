#pragma region includes
/*
GLEW is the OpenGL Extension Wrangler Library which is a simple tool that helps C/C++ developers
initialize extensions and write portable applications.
http://glew.sourceforge.net/
*/
#include <GL/glew.h>

/*
Dear ImGui is a bloat-free graphical user interface library for C++.
https://github.com/ocornut/imgui
*/
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "glm/ext.hpp"

/*
GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan development on the desktop.
It provides a simple API for creating windows, contexts and surfaces, receiving input and events.
https://www.glfw.org/
*/
#include <GLFW/glfw3.h>

/*
OpenGL Mathematics (GLM) is a header only C++ mathematics library for graphics software based on the
OpenGL Shading Language (GLSL) specifications.
https://github.com/g-truc/glm
*/
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
//#include <string>
#include <time.h>

#include "helpers/GuiHelpers.h"
#include "helpers/RenderHelpers.h"

#include "Camera.h"
#include "Floor.h"
#include "Pipe.h"
#include "Ball.h"

#pragma endregion includes

using namespace std;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 15.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

GLFWwindow* window;
IRenderHelpers* renderHelper;
IGuiHelpers* guiHelper;

Floor ground;
Pipe pipe;
Ball ball1;
Ball ball2;
bool isPaused;

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, Camera& pCamera, float pDeltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        pCamera.ProcessKeyboard(FORWARD, pDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        pCamera.ProcessKeyboard(BACKWARD, pDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            pCamera.ProcessKeyboard(LEFT, pDeltaTime);
        else
            pCamera.ProcessKeyboard(ROTATE_LEFT, pDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            pCamera.ProcessKeyboard(RIGHT, pDeltaTime);
        else
            pCamera.ProcessKeyboard(ROTATE_RIGHT, pDeltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    // camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

GLFWwindow* setupWindow()
{
    GLFWwindow* window;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    /* Initialize the library */
    if (!glfwInit())
        return 0;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hello, ACW!", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return 0;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return window;
}

void renderImGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Hello, IMGUI!");

    if (isPaused)
    {
        if (ImGui::Button("Start"))
        {
            isPaused = false;
        }
    }
    else
    {
        if (ImGui::Button("Stop"))
        {
            isPaused = true;
        }
    }

    static glm::vec3 ball1Position;
    static glm::vec3 ball1Velocity;
    static float ball1Radius;

    camera.RenderGui(*guiHelper);

    //pipe.RenderGui(*guiHelper);
    if (!isPaused)
    {
        ball1.RenderGui(*guiHelper);
        ball1Position = ball1.GetPosition();
        ball1Velocity = ball1.GetVelocity();
        ball1Radius = ball1.GetRadius();
    }
    else
    {
        ImGui::InputFloat3("Position: ", &ball1Position[0]);
        ImGui::InputFloat3("Velocity: ", &ball1Velocity[0]);
        ImGui::InputFloat("Radius: ", &ball1Radius);

        if (ImGui::Button("Set Ball 1"))
        {
            ball1.SetPosition(ball1Position);
            ball1.SetVelocity(ball1Velocity);
            ball1.SetRadius(ball1Radius);
        }
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static void renderOpenGL()
{
    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.Render(*renderHelper);
    ground.Render(*renderHelper);
    //pipe.Render(*renderHelper);
    ball1.Render(*renderHelper);
}

int main(void)
{
    window = setupWindow();
    renderHelper = new OpenGLRenderHelpers();
    guiHelper = new ImGuiHelpers();
    renderHelper->Init();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    renderHelper->SetSize(display_w, display_h);

    double lastTime = glfwGetTime();
    double timeNow = lastTime;
    double renderTime = 1.0 / 60.0;
    double updateTime = 1.0 / 100.0f;

    isPaused = false;

    ground.Translate(glm::vec3(0.0f, -0.5f, 0.0f));
    ground.Scale(glm::vec3(10.0f, 10.0f, 1.0f));

    pipe.Translate(glm::vec3(2.0f, 0.0f, 2.0f));

    ball1.Translate(glm::vec3(0.0f, 0.0f, 1.0f));

    while (!glfwWindowShouldClose(window))
    {
        timeNow = glfwGetTime();
        double elaspedTime = timeNow - lastTime;
        timeNow = lastTime;

        renderTime -= elaspedTime;

        // render 60 times a second
        if (renderTime < 0)
        {
            renderTime = 1.0 / 60.0;
            renderOpenGL();
            renderImGui();
            glfwSwapBuffers(window);
        }

        glm::vec3 Y(0.0f, 1.0f, 0.0f);
        float rotationSpeed = 2 * glm::pi<float>() / 5.f;

        glm::vec3 X(1.0f, 0.0f, 0.0f);

        if (!isPaused)
        {
            updateTime -= elaspedTime;
            // update physics 100 times a second
            if (updateTime < 0)
            {
                updateTime = 1.0 / 100.0f;
                ball1.Update(updateTime);
                cout << glm::to_string(ground.GetPosition()) << endl;
                //ball edge R = ball pos + radius
                //ball edge L = ball pos - radius
                //ball edge T = ball pos + radius
                //ball edge B = ball pos - radius
                //horizontal ball edge collide => (ground.GetScale() - ball1.GetRadius())^2 - (ball1.GetPosition() - ground.GetPosition())^2 > 0
                //vertical ball edge collide => (balledget < wall edge t && balledgeb > wall edge b)
            }
        }
        else
        {
            processInput(window, camera, updateTime);
        }

        /* Poll for and process events */
        glfwPollEvents();
    }

    delete renderHelper;
    delete guiHelper;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}
