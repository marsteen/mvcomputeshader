#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <CShaderTool.h>


using namespace std;

int main()
{
    if (!glfwInit())
    {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello computeshader world", nullptr, nullptr);

    if (window == nullptr)
    {
        return -1;
    }

    cout << "glfwCreateWindow ok" << endl;

    glfwMakeContextCurrent(window);
    glewInit();

    CShaderTool* ShaderTool = new CShaderTool;

    ShaderTool->init();

    cout << "ShaderTool init ok" << endl;

    int frame = 0;

    while (!glfwWindowShouldClose(window))
    {
        ShaderTool->updateTex(frame++);
        ShaderTool->draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}
