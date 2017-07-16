//
// Created by fixako on 06.07.2017.
//

#ifndef MPG_MPG_MODEL_HPP
#define MPG_MPG_MODEL_HPP

#include <chrono>
#include <vector>
#include <glm/glm.hpp>
#include <boost/property_tree/ptree.hpp>

class Circle
{
public:
    class DistFunction
    {
    public:
        DistFunction();
        DistFunction(double (*dist)(double), double a, double b, double min, double max, bool mirror);

        double a, b;
        double min, max;
        double (*dist)(double);
        bool mirror;

        double operator() (double x);
    };

    Circle();
    Circle(glm::vec3 position);

    std::chrono::time_point<std::chrono::system_clock> creationTime, collisionTime;
    std::chrono::duration<double> deleteDuration;
    glm::vec4 color;
    glm::vec3 position, velocity, collisionPosition;
    DistFunction sizeDist, opaciyDist;
    double speed, size, minSize, maxSize, minOpacity, maxOpacity;

    void update();
};

class MpgModel
{
public:
    std::vector<Circle> targets, clicks, missclicks;
    glm::vec2 vc;
    glm::vec3 bgColor;
    boost::property_tree::ptree* appData;

    MpgModel();
    void addTarget();
    void addClick(glm::vec3 position);
    void addMissclick(glm::vec3 position);
    void wasPaused(std::chrono::duration<double> pauseDuration);
    void update();
};


#endif //MPG_MPG_MODEL_HPP
