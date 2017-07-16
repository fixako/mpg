//
// Created by fixako on 06.07.2017.
//

#include "mpg_view.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace glm;

MpgView::MpgView(MpgModel* model)
{
    this->model = model;
    paused = false;
    initialize();
}

void MpgView::initialize()
{
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f ); // background color

    std::vector<glm::vec3> circle;
    circle.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    unsigned int fanSize = 100;
    double increment = glm::two_pi<double>() / (double)fanSize;
    for (double t = 0; t <= glm::two_pi<double>(); t += increment)
    {
        circle.push_back(glm::vec3(glm::cos(t), glm::sin(t), 0.0f));
    }

    vbCircleSize = circle.size();

    if( !glfwInit() )
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }


    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing

    window = glfwCreateWindow(mode->width, mode->height, "Mouse practice game", NULL, NULL);
    glfwSetWindowUserPointer(window, this);

    if( window == NULL )
    {
        std::cerr << "Failed to open GLFW window." << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window); // Initialize GLEW
    glewExperimental = true; // Needed in core profile

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW." << std::endl;
        return;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    GLuint VertexArrayID;

    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    glGenBuffers(1, &vbCircle);
    glBindBuffer(GL_ARRAY_BUFFER, vbCircle);
    glBufferData(GL_ARRAY_BUFFER, circle.size() * sizeof(GLfloat) * 3, &circle[0][0], GL_STATIC_DRAW);

    shaderProgramID = loadShaders("../shaders/vertexshader.glsl", "../shaders/fragmentshader.glsl");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return;
}

void MpgView::update()
{


    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    glViewport(0, 0, windowWidth, windowHeight);

    glm::mat4 viewMatrix = glm::lookAt(
            glm::vec3(0, 0, 1),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 projectionMatrix = glm::perspective(
            glm::half_pi<GLfloat>(),
            (GLfloat)windowWidth / windowHeight,
            0.1f,
            100.0f
    );

    GLint colorLocation = glGetUniformLocation(shaderProgramID, "ucolor");
    GLuint mvpLocation = glGetUniformLocation(shaderProgramID, "mvp");

    glUseProgram(shaderProgramID);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbCircle);

    glClearColor(model->bgColor.x, model->bgColor.y, model->bgColor.z, 1.0f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto updateScene = [=](vector<Circle>& circles)
    {
        for (int i = 0; i < circles.size(); ++i)
        {
            glProgramUniform4fv(shaderProgramID, colorLocation, 1, &circles[i].color[0]);
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), circles[i].position);
            double scaler = circles[i].size;
            glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaler, scaler, scaler));
            glm::mat4 modelMatrix = translationMatrix * scalingMatrix;
            glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

            glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glDrawArrays(GL_TRIANGLE_FAN, 0, vbCircleSize);
        }
    };

    updateScene(model->targets);
    updateScene(model->clicks);
    updateScene(model->missclicks);

    glDisableVertexAttribArray(0);
    glfwSwapBuffers(window);
    glfwPollEvents();
}

GLuint MpgView::loadShaders(const char *vertex_file_path, const char *fragment_file_path) {
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ifstream::in);

    if (VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    else
    {
        std::cerr << "Impossible to open " << vertex_file_path << ".Are you in the right directory?\n" << std::endl;
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ifstream::in);
    if(FragmentShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;


    // Compile Vertex Shader
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        std::cerr << &VertexShaderErrorMessage[0] << std::endl;
    }


    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        std::cerr << &FragmentShaderErrorMessage[0] << std::endl;
    }



    // Link the program
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 )
    {
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        std::cerr << &ProgramErrorMessage[0] << std::endl;
    }


    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}


glm::vec2 MpgView::getVisibleCoordinates()
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    double x = (double)width / height;
    double y = 1;
    return glm::vec2(x, y);
}

bool MpgView::shouldExit()
{
    return (glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
}

GLFWwindow* MpgView::getWindow()
{
    return window;
}


