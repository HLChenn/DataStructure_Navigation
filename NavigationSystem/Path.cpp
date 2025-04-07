#include "Path.h"

int Path::getId() const
{
    return id;
};
int Path::getFrom() const
{
    return from;
};
int Path::getTo() const
{
    return to;
};
double Path::getLength() const
{
    return length;
};
double Path::getCapacity() const
{
    return capacity;
};
double Path::getCurrentCars() const
{
    return currentCars;
};

void Path::setCurrentCars(double n)
{
    currentCars = n;
};

double Path::travelTime(double c) const
{
    double ratio = currentCars / capacity;
    if (ratio <= 1.0)
    {
        return c * length;
    }
    return c * length * (1.0 + exp(ratio));
}