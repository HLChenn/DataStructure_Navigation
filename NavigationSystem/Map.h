#include <vector>
#include <random>
#include <chrono>
#include <mingw.thread.h>
#include <functional>
#include <sstream>
#include "Place.h"
#include "Path.h"
using namespace std;

class Map
{
private:
    vector<Place> places;
    unsigned long long betterSeed() const;

public:
    Map() {};
    void generatePlaces(int N, double meanX, double meanY, double stdX, double stdY);
    const vector<Place> &getPlaces() const;
    void printPlaces() const;
};