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

void MpgModel::addTarget()
{
    default_random_engine generator(system_clock::now().time_since_epoch().count());

    double minSize = appData->get<double>("target.min_size");

    std::uniform_real_distribution<double> xDistribution(-vc.x + minSize, vc.x - minSize);
    std::uniform_real_distribution<double> yDistribution(-vc.y + minSize, vc.y - minSize);

    std::uniform_real_distribution<double> posDistribution(0, two_pi<double>());

    double x = xDistribution(generator);
    double y = yDistribution(generator);

    startAgain:
    for (int i = 0; i < targets.size(); ++i)
    {
        double d = glm::distance(targets[i].position, glm::vec3(x, y, 0.0f));

        if (d < 2.0f * minSize)
        {
            x = xDistribution(generator);
            y = yDistribution(generator);
            goto startAgain;
        }
    }

    Circle c(vec3(x, y, 0));
    double angle = posDistribution(generator);
    c.velocity = vec3(cos(angle), sin(angle), 0);

    c.minSize = appData->get<double>("target.min_size");
    c.maxSize = appData->get<double>("target.max_size");
    c.minOpacity = appData->get<double>("target.min_opacity");
    c.maxOpacity = appData->get<double>("target.max_opacity");

    c.deleteDuration = duration<double>(appData->get<double>("target.duration"));
    c.color = vec4(
            appData->get<double>("target.color.x") / 255.0,
            appData->get<double>("target.color.y") / 255.0,
            appData->get<double>("target.color.z") / 255.0, 0);
    c.speed = appData->get<double>("target.speed");

    double (*f)(double) = [](double x) { return x; };
    double a = 0, b = 1;
    c.sizeDist = Circle::DistFunction(f, a, b, 0, 1, true);

    f = [](double x) { return 1.0; };
    a = 0, b = 1;
    c.opaciyDist = Circle::DistFunction(f, a, b, 0, 1, true);

    targets.push_back(c);
}

void MpgModel::addClick(vec3 position)
{
    Circle c(position);
    c.minSize = appData->get<double>("click_target.min_size");
    c.maxSize = appData->get<double>("click_target.max_size");
    c.minOpacity = appData->get<double>("click_target.min_opacity");
    c.maxOpacity = appData->get<double>("click_target.max_opacity");
    c.deleteDuration = duration<double>(appData->get<double>("click_target.duration"));
    c.color = vec4(
            appData->get<double>("click_target.color.x") / 255.0,
            appData->get<double>("click_target.color.y") / 255.0,
            appData->get<double>("click_target.color.z") / 255.0, 0);

    double (*f)(double) = [](double x) { return log(x); };
    double a = 0.2, b = 2;
    c.sizeDist = Circle::DistFunction(f, a, b, f(a), f(b), false);

    f = [](double x) { return -log(x); };
    a = 0.05, b = 2;
    c.opaciyDist = Circle::DistFunction(f, a, b, f(b), f(a), false);

    clicks.push_back(c);
}

void MpgModel::addMissclick(vec3 position)
{
    Circle c(position);
    c.maxSize = appData->get<double>("click_background.max_size");
    c.minSize = appData->get<double>("click_background.min_size");
    c.minOpacity = appData->get<double>("click_background.min_opacity");
    c.maxOpacity = appData->get<double>("click_background.max_opacity");

    c.deleteDuration = duration<double>(appData->get<double>("click_background.duration"));
    c.color = vec4(
            appData->get<double>("click_background.color.x") / 255.0,
            appData->get<double>("click_background.color.y") / 255.0,
            appData->get<double>("click_background.color.z") / 255.0, 0);

    double (*f)(double) = [](double x) { return log(x); };
    double a = 0.2, b = 2;
    c.sizeDist = Circle::DistFunction(f, a, b, f(a), f(b), false);

    f = [](double x) { return -log(x); };
    a = 0.05, b = 2;
    c.opaciyDist = Circle::DistFunction(f, a, b, f(b), f(a), false);

    missclicks.push_back(c);
}

void MpgModel::update()
{
    auto u = [](vector<Circle>& circles)
    {
        for (int i = 0; i < circles.size(); ++i)
        {
            circles[i].update();
            if (circles[i].size == 0)
            {
                circles.erase(circles.begin() + i);
                i--;
            }
        }
    };

    u(targets);
    u(clicks);
    u(missclicks);

    auto now = system_clock::now();

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
    auto u = [pauseDuration](vector<Circle>& circles)
    {
        for (int i = 0; i < circles.size(); ++i)
        {
            circles[i].creationTime += duration_cast<system_clock::duration>(pauseDuration);
            circles[i].collisionTime += duration_cast<system_clock::duration>(pauseDuration);
        }
    };

    u(targets);
    u(clicks);
    u(missclicks);
}


Circle::Circle(glm::vec3 position)
{
    creationTime = collisionTime = system_clock::now();
    this->position = collisionPosition = position;
}

void Circle::update()
{
    duration<double> tscr = (system_clock::now() - creationTime); // time since creation
    duration<double> tsc = system_clock::now() - collisionTime; // time since collision

    double x = tscr.count();

    if (x > this->deleteDuration.count())
    {
        size = 0;
        color[3] = 0;
        return;
    }

    size = minSize + sizeDist(x / deleteDuration.count()) * (maxSize - minSize);
    color[3] = minOpacity + opaciyDist(x / deleteDuration.count()) * (maxOpacity - minOpacity);
    position = collisionPosition + (velocity * (float)(speed * tsc.count()));
}


Circle::DistFunction::DistFunction()
{
    min = max = 0;
}

Circle::DistFunction::DistFunction(double (*dist)(double), double a, double b, double min, double max, bool mirror)
{
    this->dist = dist;
    this->a = a;
    this->b = b;
    this->min = min;
    this->max = max;
    this->mirror = mirror;
}

double Circle::DistFunction::operator()(double x)
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




