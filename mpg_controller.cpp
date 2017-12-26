//
// Created by fixako on 06.07.2017.
//

#include "mpg_controller.hpp"
#include <random>
#include <fstream>
#include <boost/property_tree/json_parser.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>

using namespace glm;
using namespace chrono;
using namespace chrono_literals;
using namespace boost::property_tree;
using namespace std;


void mouseClick(GLFWwindow* window, int button, int action, int mods) 
{
    double xpos, ypos;
    int width, height;

    glfwGetWindowSize(window, &width, &height);
    glfwGetCursorPos(window, &xpos, &ypos);
    MpgController* c = (MpgController*)glfwGetWindowUserPointer(window);
    MpgModel* model = c->model;

    ypos = height - ypos;

    double maxX = (double) width / height;
    double maxY = 1;

    xpos = (xpos / width - 0.5) * 2 * maxX;
    ypos = (ypos / height - 0.5) * 2 * maxY;
	
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
		auto& targets = c->model->circles[TARGET];
        for (int i = 0; i < targets.size(); ++i)
        {
            if (glm::distance(glm::vec3(xpos, ypos, 0), targets[i].position) < targets[i].size)
            {
                targets.erase(targets.begin() + i);
                model->addCircle(&vec3(xpos, ypos, 0), TARGET_CLICK);
                return;
            }

        }
        model->addCircle(&vec3(xpos, ypos, 0), BG_CLICK);
    }
}

void keyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    MpgController* c = (MpgController*)glfwGetWindowUserPointer(window);
    MpgModel* model = c->model;

	duration<double> frequencyStep(0.05), targetDurationStep(0.25);
	double targetSizeStep = 0.01, targetSpeedStep = 0.025;

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        c->paused = !c->paused;
    }


    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
		c->appData.targetFrequency = c->appData.targetFrequency + frequencyStep;
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
		c->appData.targetFrequency = std::max(frequencyStep, (c->appData.targetFrequency - frequencyStep));  
    }


    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
		c->appData.defaultCircleData[TARGET].maxSize = std::max(targetSizeStep,
			c->appData.defaultCircleData[TARGET].maxSize - targetSizeStep);
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
		c->appData.defaultCircleData[TARGET].maxSize = c->appData.defaultCircleData[TARGET].maxSize + targetSizeStep;
    }


    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
		c->appData.defaultCircleData[TARGET].deleteDuration = c->appData.defaultCircleData[TARGET].deleteDuration + targetDurationStep;
    }
    if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		c->appData.defaultCircleData[TARGET].deleteDuration =
			std::max(targetDurationStep, c->appData.defaultCircleData[TARGET].deleteDuration - targetDurationStep);
    }


	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		c->appData.defaultCircleData[TARGET].speed = std::max(0.0, c->appData.defaultCircleData[TARGET].speed - targetSpeedStep);
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
		c->appData.defaultCircleData[TARGET].speed = c->appData.defaultCircleData[TARGET].speed + targetSpeedStep;
    }


	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		c->loadDefaultAppData();
	}
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		c->loadSavedAppData();
	}


    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
    {
		c->saveAppData();
    }
}

void windowSizeChanged(GLFWwindow* window, int width, int height)
{
	if (width == 0 || height == 0)
	{
		return;
	}
    MpgController* c = (MpgController*)glfwGetWindowUserPointer(window);
    MpgModel* model = c->model;

	bool maximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED);

	model->vc = c->view->getVisibleCoordinates();
	c->appData.windowSize.x = width;
	c->appData.windowSize.y = height;

	c->appData.maximizeWindow = maximized;
}

void windowPositionChanged(GLFWwindow* window, int xpos, int ypos)
{
    MpgController* c = (MpgController*)glfwGetWindowUserPointer(window);
	c->appData.windowPosition.x = xpos;
	c->appData.windowPosition.y = ypos;
}

void windowIconified(GLFWwindow* window, int iconified)
{
	MpgController* c = (MpgController*)glfwGetWindowUserPointer(window);
	if (iconified)
	{
		c->iconified = true;
	}
	else
	{
		c->iconified = false;;
		c->view->update();
	}
}


MpgController::MpgController()
{
	paused = false;
	iconified = false;

	appData.defaultCircleData = new CircleTypeData[CIRCLE_TYPE_SIZE];

    if (!fstream("config.json"))
    {
		loadDefaultAppData();
		saveAppData();
    }
	else
	{
		loadSavedAppData();
	}

    this->model = new MpgModel();
    this->view = new MpgView(model);
    this->model->circleTypes = appData.defaultCircleData;
    this->model->vc = view->getVisibleCoordinates();
	this->model->bgColor = appData.bgColor;

    GLFWwindow* w = view->getWindow();
	glfwSetWindowUserPointer(w, this);
    glfwSetMouseButtonCallback(w, mouseClick);
    glfwSetKeyCallback(w, keyPress);
    glfwSetWindowSizeCallback(w, windowSizeChanged);
    glfwSetWindowPosCallback(w, windowPositionChanged);
	glfwSetWindowIconifyCallback(w, windowIconified);

	if (appData.maximizeWindow)
	{
		glfwMaximizeWindow(w);
	}
	else
	{
		glfwSetWindowSize(w, appData.windowSize.x, appData.windowSize.y);
		glfwSetWindowPos(w, appData.windowPosition.x, appData.windowPosition.y);
	}
}

MpgController::~MpgController()
{
	delete appData.defaultCircleData;
	delete model;
	delete view;
}


void MpgController::loadDefaultAppData()
{
	appData.bgColor = vec3(50.0 / 255.0, 76.0 / 255.0, 117.0 / 255.0);
	appData.maximizeWindow = true;
	appData.windowPosition = vec2(0, 30);
	appData.windowSize = vec2(1920, 1017);
	appData.targetFrequency = 0.5s;
	CircleTypeData* defaultCircleData = appData.defaultCircleData;

	double(*f)(double) = [](double x) { return x; };
	double a = 0, b = 1;
	defaultCircleData[TARGET].sizeDist = DistFunction(f, a, b, 0, 1, true);
	f = [](double x) { return 1.0; };
	a = 0, b = 1;
	defaultCircleData[TARGET].opaciyDist = DistFunction(f, a, b, 0, 1, true);;

	defaultCircleData[TARGET].color = vec3(255.0 / 255.0, 182.0 / 255.0, 10.0  / 255.0);
	defaultCircleData[TARGET].deleteDuration = duration<double>(2.5);
	defaultCircleData[TARGET].speed = 0.2;
	defaultCircleData[TARGET].minSize = 0.0;
	defaultCircleData[TARGET].maxSize = 0.05;
	defaultCircleData[TARGET].minOpacity = 0.0;
	defaultCircleData[TARGET].maxOpacity = 1.0;


	f = [](double x) { return log(x); };
	a = 0.2, b = 2;
	defaultCircleData[TARGET_CLICK].sizeDist = DistFunction(f, a, b, f(a), f(b), false);

	f = [](double x) { return -log(x); };
	a = 0.05, b = 2;
	defaultCircleData[TARGET_CLICK].opaciyDist = DistFunction(f, a, b, f(b), f(a), false);

	defaultCircleData[TARGET_CLICK].color = vec3(0.0, 1.0, 0.0);
	defaultCircleData[TARGET_CLICK].deleteDuration = duration<double>(1);
	defaultCircleData[TARGET_CLICK].speed = 0;
	defaultCircleData[TARGET_CLICK].minSize = 0.0;
	defaultCircleData[TARGET_CLICK].maxSize = 0.05;
	defaultCircleData[TARGET_CLICK].minOpacity = 0.05;
	defaultCircleData[TARGET_CLICK].maxOpacity = 1.0;

	defaultCircleData[BG_CLICK] = defaultCircleData[TARGET_CLICK];
	defaultCircleData[BG_CLICK].color = vec3(1.0, 0.0, 0.0);
}

void MpgController::loadSavedAppData()
{
	ptree savedAppData;
	read_json("config.json", savedAppData);

	appData.bgColor = vec3(
		savedAppData.get<double>("background_color.x") / 255.0, 
		savedAppData.get<double>("background_color.y") / 255.0,
		savedAppData.get<double>("background_color.z") / 255.0);

	appData.maximizeWindow = savedAppData.get<bool>("window_maximize");
	appData.windowPosition = vec2(savedAppData.get<double>("window_position.x"), 
							 savedAppData.get<double>("window_position.y"));
	appData.windowSize = vec2(savedAppData.get<double>("window_size.width"), 
						      savedAppData.get<double>("window_size.height"));
	appData.targetFrequency = duration<double>(savedAppData.get<double>("target.frequency"));

	CircleTypeData* defaultCircleData = appData.defaultCircleData;

	double(*f)(double) = [](double x) { return x; };
	double a = 0, b = 1;
	defaultCircleData[TARGET].sizeDist = DistFunction(f, a, b, 0, 1, true);
	f = [](double x) { return 1.0; };
	a = 0, b = 1;
	defaultCircleData[TARGET].opaciyDist = DistFunction(f, a, b, 0, 1, true);

	defaultCircleData[TARGET].color = vec3(
		savedAppData.get<double>("target.color.x") / 255.0,
		savedAppData.get<double>("target.color.y") / 255.0,
		savedAppData.get<double>("target.color.z") / 255.0);

	defaultCircleData[TARGET].deleteDuration = duration<double>(
		savedAppData.get<double>("target.duration"));
	defaultCircleData[TARGET].speed = 
		savedAppData.get<double>("target.speed");
	defaultCircleData[TARGET].minSize = 
		savedAppData.get<double>("target.min_size");
	defaultCircleData[TARGET].maxSize = 
		savedAppData.get<double>("target.max_size");
	defaultCircleData[TARGET].minOpacity = 
		savedAppData.get<double>("target.min_opacity");
	defaultCircleData[TARGET].maxOpacity = 
		savedAppData.get<double>("target.max_opacity");


	f = [](double x) { return log(x); };
	a = 0.2, b = 2;
	defaultCircleData[TARGET_CLICK].sizeDist = DistFunction(f, a, b, f(a), f(b), false);

	f = [](double x) { return -log(x); };
	a = 0.05, b = 2;
	defaultCircleData[TARGET_CLICK].opaciyDist = DistFunction(f, a, b, f(b), f(a), false);

	defaultCircleData[TARGET_CLICK].speed = 0.0;
	defaultCircleData[TARGET_CLICK].color = vec3(
		savedAppData.get<double>("click_target.color.x") / 255.0,
		savedAppData.get<double>("click_target.color.y") / 255.0,
		savedAppData.get<double>("click_target.color.z") / 255.0);
	defaultCircleData[TARGET_CLICK].deleteDuration = duration<double>(
		savedAppData.get<double>("click_target.duration"));
	defaultCircleData[TARGET_CLICK].minSize =
		savedAppData.get<double>("click_target.min_size");
	defaultCircleData[TARGET_CLICK].maxSize =
		savedAppData.get<double>("click_target.max_size");
	defaultCircleData[TARGET_CLICK].minOpacity =
		savedAppData.get<double>("click_target.min_opacity");
	defaultCircleData[TARGET_CLICK].maxOpacity =
		savedAppData.get<double>("click_target.max_opacity");


	f = [](double x) { return log(x); };
	a = 0.2, b = 2;
	defaultCircleData[BG_CLICK].sizeDist = DistFunction(f, a, b, f(a), f(b), false);
	f = [](double x) { return -log(x); };
	a = 0.05, b = 2;
	defaultCircleData[BG_CLICK].opaciyDist = DistFunction(f, a, b, f(b), f(a), false);

	defaultCircleData[BG_CLICK].speed = 0.0;
	defaultCircleData[BG_CLICK].color = vec3(
		savedAppData.get<double>("click_background.color.x") / 255.0,
		savedAppData.get<double>("click_background.color.y") / 255.0,
		savedAppData.get<double>("click_background.color.z") / 255.0);
	defaultCircleData[BG_CLICK].deleteDuration = duration<double>(
		savedAppData.get<double>("click_background.duration"));
	defaultCircleData[BG_CLICK].minSize =
		savedAppData.get<double>("click_background.min_size");
	defaultCircleData[BG_CLICK].maxSize =
		savedAppData.get<double>("click_background.max_size");
	defaultCircleData[BG_CLICK].minOpacity =
		savedAppData.get<double>("click_background.min_opacity");
	defaultCircleData[BG_CLICK].maxOpacity =
		savedAppData.get<double>("click_background.max_opacity");
}

void MpgController::saveAppData()
{
	ptree currentAppData;

	currentAppData.put("background_color.x", appData.bgColor.x * 255);
	currentAppData.put("background_color.y", appData.bgColor.y * 255);
	currentAppData.put("background_color.z", appData.bgColor.z * 255);

	currentAppData.put("window_maximize", appData.maximizeWindow);
	currentAppData.put("window_position.x", appData.windowPosition.x);
	currentAppData.put("window_position.y", appData.windowPosition.y);
	currentAppData.put("window_size.width", appData.windowSize.x);
	currentAppData.put("window_size.height", appData.windowSize.y);

	currentAppData.put("target.frequency", appData.targetFrequency.count());
	currentAppData.put("target.color.x", appData.defaultCircleData[TARGET].color.x * 255.0);
	currentAppData.put("target.color.y", appData.defaultCircleData[TARGET].color.y * 255.0);
	currentAppData.put("target.color.z", appData.defaultCircleData[TARGET].color.z * 255.0);
	currentAppData.put("target.speed", appData.defaultCircleData[TARGET].speed);
	currentAppData.put("target.duration", appData.defaultCircleData[TARGET].deleteDuration.count());
	currentAppData.put("target.min_size", appData.defaultCircleData[TARGET].minSize);
	currentAppData.put("target.max_size", appData.defaultCircleData[TARGET].maxSize);
	currentAppData.put("target.min_opacity", appData.defaultCircleData[TARGET].minOpacity);
	currentAppData.put("target.max_opacity", appData.defaultCircleData[TARGET].maxOpacity);

	currentAppData.put("click_target.color.x", appData.defaultCircleData[TARGET_CLICK].color.x * 255.0);
	currentAppData.put("click_target.color.y", appData.defaultCircleData[TARGET_CLICK].color.y * 255.0);
	currentAppData.put("click_target.color.z", appData.defaultCircleData[TARGET_CLICK].color.z * 255.0);
	currentAppData.put("click_target.duration", appData.defaultCircleData[TARGET_CLICK].deleteDuration.count());
	currentAppData.put("click_target.min_size", appData.defaultCircleData[TARGET_CLICK].minSize);
	currentAppData.put("click_target.max_size", appData.defaultCircleData[TARGET_CLICK].maxSize);
	currentAppData.put("click_target.min_opacity", appData.defaultCircleData[TARGET_CLICK].minOpacity);
	currentAppData.put("click_target.max_opacity", appData.defaultCircleData[TARGET_CLICK].maxOpacity);

	currentAppData.put("click_background.color.x", appData.defaultCircleData[BG_CLICK].color.x * 255.0);
	currentAppData.put("click_background.color.y", appData.defaultCircleData[BG_CLICK].color.y * 255.0);
	currentAppData.put("click_background.color.z", appData.defaultCircleData[BG_CLICK].color.z * 255.0);
	currentAppData.put("click_background.duration", appData.defaultCircleData[BG_CLICK].deleteDuration.count());
	currentAppData.put("click_background.min_size", appData.defaultCircleData[BG_CLICK].minSize);
	currentAppData.put("click_background.max_size", appData.defaultCircleData[BG_CLICK].maxSize);
	currentAppData.put("click_background.min_opacity", appData.defaultCircleData[BG_CLICK].minOpacity);
	currentAppData.put("click_background.max_opacity", appData.defaultCircleData[BG_CLICK].maxOpacity);

	write_json("config.json", currentAppData);
}


void MpgController::startGame()
{
    time_point<system_clock, duration<double>> nextTargetTime = system_clock::now();

    do
    {
        auto targetFrequency = appData.targetFrequency;

        auto pauseBegin = system_clock::now();
        bool wasPaused = false;
        while (paused == true || iconified == true)
        {
            wasPaused = true;
            this_thread::sleep_for(0.1s);
            glfwPollEvents();
			if (view->shouldExit())
			{
				return;
			}
        }

        if (wasPaused)
        {
            duration<double> pauseDuration = system_clock::now() - pauseBegin;
            nextTargetTime += pauseDuration;
            model->wasPaused(pauseDuration);
        }

        if (system_clock::now() > nextTargetTime)
        {
            model->addCircle(NULL, TARGET);
            nextTargetTime = nextTargetTime + targetFrequency;
        }

        model->update();
        view->update();
    }
    while (!view->shouldExit());
}