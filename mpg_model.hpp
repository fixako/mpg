//
// Created by fixako on 06.07.2017.
//

#ifndef MPG_MPG_MODEL_HPP
#define MPG_MPG_MODEL_HPP

#include <chrono>
#include <vector>
#include <glm/glm.hpp>
#include <boost/property_tree/ptree.hpp>

enum CircleType
{
	TARGET, TARGET_CLICK, BG_CLICK, CIRCLE_TYPE_SIZE
};

class DistFunction
{
public:
	DistFunction();
	DistFunction(double(*dist)(double), double a, double b, double min, double max, bool mirror);

	double a, b;
	double min, max;
	double (*dist)(double);
	bool mirror;

	double operator() (double x);
};

struct CircleTypeData
{
	std::chrono::duration<double> deleteDuration;
	glm::vec3 color;
	double speed, minSize, maxSize, minOpacity, maxOpacity;
	DistFunction sizeDist, opaciyDist;
};

class Circle
{
public:
	~Circle();
	Circle();
    Circle(glm::vec3 position);

    std::chrono::time_point<std::chrono::system_clock> creationTime, collisionTime;
	double opacity, size;
    glm::vec3 position, velocity, collisionPosition;
	CircleTypeData* typeData;

    void update();
};

class MpgModel
{
public:
	CircleTypeData* circleTypes;
	std::vector<Circle> circles[CIRCLE_TYPE_SIZE];
    glm::vec3 bgColor;
	glm::vec2 vc;

    MpgModel();
	void addCircle(glm::vec3* position, CircleType ctype);
    void wasPaused(std::chrono::duration<double> pauseDuration);
    void update();
};


#endif //MPG_MPG_MODEL_HPP
