
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ctime>
#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>
#include <sstream>

std::chrono::time_point<std::chrono::steady_clock> t1, t2;
std::chrono::duration<float> dt;

GLFWwindow* window;
GLFWwindow* window2;
int w1[4] = {30, 30, 640, 480};
bool w1full = false;

void key_callback         (GLFWwindow* wnd, int key, int scancode, int action, int mods);
//void scroll_callback      (GLFWwindow* wnd, double xoffset, double yoffset);
//void cursorpos_callback   (GLFWwindow* wnd, double xpos, double ypos);
//void mousebutton_callback (GLFWwindow* wnd, int button, int action, int mods);
void window_size_callback(GLFWwindow* wnd, int width, int height);




int main ()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    //glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
    
    //window = glfwCreateWindow(WIDTH, HEIGHT, "NewWorld", glfwGetPrimaryMonitor(), nullptr);
    window = glfwCreateWindow(w1[2], w1[3], "GLFW", nullptr, nullptr);
    glfwSetWindowPos(window, w1[0], w1[1]);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    if (window == NULL)
    {
        std::cerr << "ERROR: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -2;
    }
    
    glfwSetKeyCallback        (window, key_callback);
    //glfwSetScrollCallback     (window, scroll_callback);
    //glfwSetCursorPosCallback  (window, cursorpos_callback);
    //glfwSetMouseButtonCallback(window, mousebutton_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    
    
    while (!glfwWindowShouldClose(window))
    {
        t2 = std::chrono::steady_clock::now();
        dt = t2 - t1;
        
        /*
        float slp = 16666.6f - std::chrono::duration_cast<std::chrono::microseconds>(dt).count();
        if (slp > 200.0f)
        {
            //cout << "Sleep: " << slp << endl;
            //std::this_thread::sleep_for(std::chrono::microseconds((int)slp - 100));
        }
        */
    
        t2 = std::chrono::steady_clock::now();
        dt = t2 - t1;
        //std::cout << "dt: " << std::chrono::duration_cast<std::chrono::microseconds>(dt).count() << std::endl;
        
        t1 = t2;
        
        glfwPollEvents();
        //render
        glfwSwapBuffers(window);
    }
    
    
    return 0;
}

void key_callback (GLFWwindow* /*wnd*/, int key, int /*scancode*/, int action, int mods)
{
    if (action != GLFW_PRESS) return;
    
    
    if (key == GLFW_KEY_ESCAPE) { glfwSetWindowShouldClose(window, GL_TRUE); return; }
    
    else if (key == (GLFW_KEY_C) && (mods & GLFW_MOD_CONTROL))
    {
        glfwSetClipboardString(NULL, "clipboard test");
        return;
    }
    
    else if (key == (GLFW_KEY_V) && (mods & GLFW_MOD_CONTROL))
    {
        std::cout << "Pasted: " << glfwGetClipboardString(NULL) << std::endl;
        return;
    }
    
    
    else if (key == GLFW_KEY_F11)
    {
        if (w1full)
        {
            glfwSetWindowMonitor(window, nullptr, 0, 0, w1[2], w1[3], 0);
            //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetWindowPos(window, w1[0], w1[1]);
            w1full = false;
        }
        else
        {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
            //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            w1full = true;
        }
    }
}
//void scroll_callback      (GLFWwindow* wnd, double xoffset, double yoffset);
//void cursorpos_callback   (GLFWwindow* wnd, double xpos, double ypos);
//void mousebutton_callback (GLFWwindow* wnd, int button, int action, int mods);
void window_size_callback(GLFWwindow* /*wnd*/, int width, int height)
{
    std::cout << "SIZE: " << width << " - " << height << std::endl;
}
