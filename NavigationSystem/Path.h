#include <iostream>
#include <cmath>
using namespace std;

class Path
{
private:
    int id;
    int from;
    int to;
    double length;
    double capacity;
    double currentCars;

public:
    Path(int _id, int _from, int _to, double _length, double _capacity) : id(_id), from(_from), to(_to), length(_length), capacity(_capacity), currentCars(0) {};
    int getId() const;
    int getFrom() const;
    int getTo() const;
    double getLength() const;
    double getCapacity() const;
    double getCurrentCars() const;

    void setCurrentCars(double n);

    double travelTime(double c) const;
};