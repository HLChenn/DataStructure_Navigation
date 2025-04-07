#include "Map.h"

// 为generatePlaces函数服务，提供随机种子
unsigned long long Map::betterSeed() const
{
    auto now = chrono::high_resolution_clock::now().time_since_epoch().count();

    // 把 thread id 转为字符串后 hash
    stringstream ss;
    ss << this_thread::get_id();
    string tidStr = ss.str();
    size_t tidHash = hash<string>{}(tidStr);

    return static_cast<unsigned long long>(now ^ tidHash);
}

void Map::generatePlaces(int N, double meanX, double meanY, double stdX, double stdY)
{
    mt19937 gen(betterSeed());
    normal_distribution<> distX(meanX, stdX);
    normal_distribution<> distY(meanY, stdY);
    for (int i = 0; i < N; i++)
    {
        double x = distX(gen);
        double y = distY(gen);

        if (x < 0)
        {
            x = 0;
        }
        if (x > 1000)
        {
            x = 1000;
        }
        if (y < 0)
        {
            y = 0;
        }
        if (y > 1000)
        {
            y = 1000;
        }

        places.push_back(Place(x, y));
    }
}

const vector<Place> &Map::getPlaces() const
{
    return places;
}

void Map::printPlaces() const
{
    for (size_t i = 0; i < places.size(); i++)
    {
        cout << "place:" << i << ":" << places[i].getPositionX() << "," << places[i].getPositionY() << endl;
    }
}