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

struct AppData
{
	glm::vec3 bgColor;
	bool maximizeWindow;
	glm::vec2 windowPosition, windowSize;
	duration<double> targetFrequency;
	CircleTypeData* defaultCircleData;
};


class MpgController
{
private:
    MpgModel* model;
    MpgView* view;
	AppData appData;
	
    bool paused, iconified;

public:
    MpgController();
	~MpgController();

	void loadDefaultAppData();
	void loadSavedAppData();
	void saveAppData();
    void startGame();

    friend void mouseClick(GLFWwindow* window, int button, int action, int mods);
    friend void keyPress(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void windowSizeChanged(GLFWwindow* window, int width, int height);
    friend void windowPositionChanged(GLFWwindow* window, int xpos, int ypos);
	friend void windowIconified(GLFWwindow* window, int iconified);
};


#endif //MPG_MPG_CONTROLLER_HPP
