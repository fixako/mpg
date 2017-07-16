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
        for (int i = 0; i < model->targets.size(); ++i)
        {
            if (glm::distance(glm::vec3(xpos, ypos, 0), model->targets[i].position) < model->targets[i].size)
            {
                model->targets.erase(model->targets.begin() + i);
                model->addClick(vec3(xpos, ypos, 0));
                return;
            }

        }
        model->addMissclick(vec3(xpos, ypos, 0));
    }
}

void keyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    MpgController* c = (MpgController*)glfwGetWindowUserPointer(window);
    MpgModel* model = c->model;

    double frequencyStep = 0.05, targetSizeStep = 0.01, targetDurationStep = 0.25, targetSpeedStep = 0.025;

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        c->paused = !c->paused;
    }


    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        auto targetFrequency = std::max(frequencyStep, (c->appData->get<double>("target.frequency") - frequencyStep));
        model->appData->put("target.frequency", targetFrequency);
    }

    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        auto targeFrequency = std::max(frequencyStep, c->appData->get<double>("target.frequency") + frequencyStep);
        c->appData->put("target.frequency", targeFrequency);
    }


    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        auto targetSize = std::max(targetSizeStep, c->appData->get<double>("target.max_size") - targetSizeStep);
        c->appData->put("target.max_size", targetSize);
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        auto targetSize = std::max(targetSizeStep, c->appData->get<double>("target.max_size") + targetSizeStep);
        c->appData->put("target.max_size", targetSize);
    }


    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        auto targetDuration = std::max(targetDurationStep, c->appData->get<double>("target.duration") - targetDurationStep);
        c->appData->put("target.duration", targetDuration);
    }

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        auto targetDuration = std::max(targetDurationStep, c->appData->get<double>("target.duration") + targetDurationStep);
        c->appData->put("target.duration", targetDuration);
    }


    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        auto targetSpeed = std::max(0.0, c->appData->get<double>("target.speed") - targetSpeedStep);
        c->appData->put("target.speed", targetSpeed);
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        auto targetSpeed = std::max(0.0, c->appData->get<double>("target.speed") + targetSpeedStep);
        c->appData->put("target.speed", targetSpeed);
    }


    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
    {
        write_json("config.json", *c->appData);
    }
}

void windowSizeChanged(GLFWwindow* window, int width, int height)
{
    MpgController* c = (MpgController*)glfwGetWindowUserPointer(window);
    MpgModel* model = c->model;
    model->vc = c->view->getVisibleCoordinates();
    c->appData->put("window_size.width", width);
    c->appData->put("window_size.height", height);
}

void windowPositionChanged(GLFWwindow* window, int xpos, int ypos)
{
    MpgController* c = (MpgController*)glfwGetWindowUserPointer(window);
    c->appData->put("window_position.x", xpos);
    c->appData->put("window_position.y", ypos);
}


MpgController::MpgController()
{
    appData = new ptree;

    appData->put("background_color.x", 50.0);
    appData->put("background_color.y", 76.0);
    appData->put("background_color.z", 117.0);

    appData->put("window_position.x", 0);
    appData->put("window_position.y", 0);
    appData->put("window_size.width", 1920);
    appData->put("window_size.height", 1080);

    appData->put("target.color.x", 255);
    appData->put("target.color.y", 182);
    appData->put("target.color.z", 10);
    appData->put("target.speed", 0.2);
    appData->put("target.frequency", 0.55);
    appData->put("target.duration", 2.5);
    appData->put("target.min_size", 0.0);
    appData->put("target.max_size", 0.075);
    appData->put("target.min_opacity", 0.0);
    appData->put("target.max_opacity", 1.0);

    appData->put("click_target.duration", 1);
    appData->put("click_target.color.x", 0);
    appData->put("click_target.color.y", 255);
    appData->put("click_target.color.z", 0);
    appData->put("click_target.min_size", 0.0);
    appData->put("click_target.max_size", 0.05);
    appData->put("click_target.min_opacity", 0.05);
    appData->put("click_target.max_opacity", 1.0);

    appData->put("click_background.duration", 1);
    appData->put("click_background.color.x", 255);
    appData->put("click_background.color.y", 0);
    appData->put("click_background.color.z", 0);
    appData->put("click_background.min_size", 0.0);
    appData->put("click_background.max_size", 0.05);
    appData->put("click_background.min_opacity", 0.05);
    appData->put("click_background.max_opacity", 1.0);


    if (!fstream("config.json"))
    {
        write_json("config.json", *appData);
    }

    read_json("config.json", *appData);

    paused = false;

    this->model = new MpgModel();
    this->view = new MpgView(model);
    this->model->appData = appData;
    this->model->vc = view->getVisibleCoordinates();
    this->model->bgColor = vec3(
            appData->get<double>("background_color.x") / 255,
            appData->get<double>("background_color.y") / 255,
            appData->get<double>("background_color.z") / 255
    );


    GLFWwindow* w = view->getWindow();
    glfwSetMouseButtonCallback(w, mouseClick);
    glfwSetKeyCallback(w, keyPress);
    glfwSetWindowSizeCallback(w, windowSizeChanged);
    glfwSetWindowPosCallback(w, windowPositionChanged);
    glfwSetWindowUserPointer(w, this);

    glfwSetWindowSize(w,
                      appData->get<int>("window_size.width"),
                      appData->get<int>("window_size.height"));


    glfwSetWindowPos(w,
                     appData->get<int>("window_position.x"),
                     appData->get<int>("window_position.y"));
}


void MpgController::startGame()
{
    time_point<system_clock, duration<double>> nextTargetTime = system_clock::now();

    do
    {
        auto freq = appData->get<double>("target.frequency");
        auto targetFrequency = duration<double>(freq);

        auto pauseBegin = system_clock::now();
        bool wasPaused = false;
        while (paused == true)
        {
            wasPaused = true;
            this_thread::sleep_for(0.1s);
            glfwPollEvents();
        }

        if (wasPaused)
        {
            duration<double> pauseDuration = system_clock::now() - pauseBegin;
            nextTargetTime += pauseDuration;
            model->wasPaused(pauseDuration);
        }

        if (system_clock::now() > nextTargetTime)
        {
            model->addTarget();
            nextTargetTime = nextTargetTime + targetFrequency;
        }

        model->update();
        view->update();
    }
    while (view->shouldExit());
}


