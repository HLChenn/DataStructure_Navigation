#include <iostream>
using namespace std;

class Place
{
private:
    double positionX;
    double positionY;

public:
    Place(double x, double y) : positionX(x), positionY(y) {};
    double getPositionX() const;
    double getPositionY() const;
};
