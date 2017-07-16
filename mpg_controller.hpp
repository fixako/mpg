//
// Created by fixako on 06.07.2017.
//

#ifndef MPG_MPG_CONTROLLER_HPP
#define MPG_MPG_CONTROLLER_HPP

#include "mpg_model.hpp"
#include "mpg_view.hpp"
#include <boost/property_tree/ptree.hpp>

using namespace glm;
using namespace std;
using namespace chrono;
using namespace chrono_literals;


class MpgController
{
private:
    MpgModel* model;
    MpgView* view;
    boost::property_tree::ptree* appData;
    bool paused;

public:
    MpgController();
    void startGame();

    friend void mouseClick(GLFWwindow* window, int button, int action, int mods);
    friend void keyPress(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void windowSizeChanged(GLFWwindow* window, int width, int height);
    friend void windowPositionChanged(GLFWwindow* window, int xpos, int ypos);
};


#endif //MPG_MPG_CONTROLLER_HPP
