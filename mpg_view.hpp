//
// Created by fixako on 06.07.2017.
//

#ifndef MPG_MPG_VIEW_HPP
#define MPG_MPG_VIEW_HPP

#include "mpg_model.hpp"
#include "mpg_view.hpp"

#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class MpgView
{
private:
    GLFWwindow* window;
    GLuint shaderProgramID;
    GLuint vbCircle, vbCircleSize;
    MpgModel* model;
    bool paused;

    GLuint loadShaders(const char* vertex_file_path, const char* fragment_file_path);

public:
    MpgView(MpgModel* model);
    void initialize();
    void update();
    bool shouldExit();
    GLFWwindow* getWindow();
    glm::vec2 getVisibleCoordinates();

    friend void keyPress(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void mouseClick(GLFWwindow* window, int button, int action, int mods);

    void setBackground(glm::vec3 color);
};


#endif //MPG_MPG_VIEW_HPP
