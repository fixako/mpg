//
// Created by fixako on 06.07.2017.
//

#include <random>
#include <glm/gtc/constants.hpp>
#include <iostream>

#include "mpg_model.hpp"

using namespace std;
using namespace glm;
using namespace chrono;
using namespace chrono_literals;
using namespace boost::property_tree;


MpgModel::MpgModel()
{
}

void MpgModel::addCircle(glm::vec3* position, CircleType ctype)
{
	Circle c;
	if (position == NULL)
	{
		vec3 randomPosition;
		default_random_engine generator(system_clock::now().time_since_epoch().count());

		double minSize = circleTypes[TARGET].minSize;

		std::uniform_real_distribution<double> xDistribution(-vc.x + minSize, vc.x - minSize);
		std::uniform_real_distribution<double> yDistribution(-vc.y + minSize, vc.y - minSize);
		std::uniform_real_distribution<double> posDistribution(0, two_pi<double>());

		double x = xDistribution(generator);
		double y = yDistribution(generator);

		startAgain:
		for (int i = 0; i < circles[TARGET].size(); ++i)
		{
			double d = glm::distance(circles[TARGET][i].position, glm::vec3(x, y, 0.0f));

			if (d < 2.0f * minSize)
			{
				x = xDistribution(generator);
				y = yDistribution(generator);
				goto startAgain;
			}
		}

		randomPosition.x = x;
		randomPosition.y = y;

		double angle = posDistribution(generator);
		c.velocity = vec3(cos(angle), sin(angle), 0.0);
		c.position = c.collisionPosition = randomPosition;
	}
	else
	{
		c.velocity = vec3(0, 0, 0);
		c.position = c.collisionPosition = *position;
	}
	
	c.typeData = &circleTypes[ctype];
	circles[ctype].push_back(c);
}


void MpgModel::update()
{
	for (size_t i = 0; i < CIRCLE_TYPE_SIZE; i++)
	{
		for (size_t j = 0; j < circles[i].size(); j++)
		{
			circles[i][j].update();
			if (circles[i][j].size == 0)
			{
				circles[i].erase(circles[i].begin() + j);
				j--;
			}
		}
	}

    auto now = system_clock::now();
	vector<Circle>& targets = circles[TARGET];

    for (int i = 0; i < targets.size(); i++)
    {
        if (vc.x - abs(targets[i].position.x) <= targets[i].size)
        {
            targets[i].position.x -= sign(targets[i].position.x) *
                    (targets[i].size - (vc.x - abs(targets[i].position.x)));
            targets[i].collisionTime = now;
            targets[i].velocity.x = -targets[i].velocity.x;
            targets[i].collisionPosition = targets[i].position;
        }

        if (vc.y - abs(targets[i].position.y) <= targets[i].size)
        {
            targets[i].position.y -= sign(targets[i].position.y) *
                                     (targets[i].size - (vc.y - abs(targets[i].position.y)));
            targets[i].collisionTime = now;
            targets[i].velocity.y = -targets[i].velocity.y;
            targets[i].collisionPosition = targets[i].position;
        }

        for (int j = i + 1; j < targets.size(); j++)
        {
            double d = distance(targets[i].position, targets[j].position);

            if (d <= targets[i].size + targets[j].size)
            {
                targets[i].position += normalize(targets[i].position - targets[j].position) *
                        (float)(targets[i].size + targets[j].size - d);

                vec3 v1 = targets[i].velocity, v2 = targets[j].velocity;
                vec3 x1 = targets[i].position, x2 = targets[j].position;
                double m1 = targets[i].size, m2 = targets[j].size;
                targets[i].velocity =
                        v1 - (float)(2 * m2 / (m1 + m2) * dot(v1 - v2, x1 - x2) / pow(length(x1 - x2), 2)) * (x1 - x2);
                targets[j].velocity =
                        v2 - (float)(2 * m1 / (m1 + m2) * dot(v2 - v1, x2 - x1) / pow(length(x2 - x1), 2)) * (x2 - x1);
                targets[i].collisionTime = now;
                targets[j].collisionTime = now;
                targets[i].collisionPosition = targets[i].position;
                targets[j].collisionPosition = targets[j].position;
            }
        }
    }
}

void MpgModel::wasPaused(std::chrono::duration<double> pauseDuration)
{
	for (size_t i = 0; i < CIRCLE_TYPE_SIZE; i++)
	{
		for (int j = 0; j < circles[i].size(); ++j)
		{
			circles[i][j].creationTime += duration_cast<system_clock::duration>(pauseDuration);
			circles[i][j].collisionTime += duration_cast<system_clock::duration>(pauseDuration);
		}
	}
}


Circle::Circle()
{
	creationTime = collisionTime = system_clock::now();
}

Circle::~Circle()
{
}

Circle::Circle(glm::vec3 position) : Circle()
{
    this->position = collisionPosition = position;
}

void Circle::update()
{
    duration<double> tscr = (system_clock::now() - creationTime); // time since creation
    duration<double> tsc = system_clock::now() - collisionTime; // time since collision

    double x = tscr.count();

	double d = this->typeData->deleteDuration.count();
	double minSize = this->typeData->minSize, minOpacity = this->typeData->minOpacity;
	double maxSize = this->typeData->maxSize, maxOpacity = this->typeData->maxOpacity;
	double speed = this->typeData->speed;

	DistFunction sizeDist = this->typeData->sizeDist;
	DistFunction opacityDist = this->typeData->opaciyDist;

    if (x > d)
    {
        size = 0;
        opacity = 0;
        return;
    }

    size = minSize + sizeDist(x / d) * (maxSize - minSize);
    opacity = minOpacity + opacityDist(x / d) * (maxOpacity - minOpacity);
    position = collisionPosition + (velocity * (float)(speed * tsc.count()));
}


DistFunction::DistFunction()
{
    min = max = 0;
}

DistFunction::DistFunction(double (*dist)(double), double a, double b, double min, double max, bool mirror)
{
    this->dist = dist;
    this->a = a;
    this->b = b;
    this->min = min;
    this->max = max;
    this->mirror = mirror;
}

double DistFunction::operator()(double x)
{
    if (min == max)
    {
        return 1;
    }

    if (mirror)
    {
        if (x > 0.5)
        {
            x = 1 - x;
        }
        x = x * 2;
    }

    double xp = x * (b - a) + a; // convert to [a, b]
    return (dist(xp) - min) / (max - min); // convert dist(xp) to [0, 1]
}




