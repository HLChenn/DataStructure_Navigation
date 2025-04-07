#include "Map.h"

int main()
{
    Map map;
    int N = 10000;
    double meanX = 500.0, meanY = 500.0;     // 坐标中心（城市中心区域）
    double stddevX = 200.0, stddevY = 200.0; // 标准差（分布的密集程度）

    // 生成点
    map.generatePlaces(N, meanX, meanY, stddevX, stddevY);

    // 打印一些生成的点
    map.printPlaces();

    return 0;
}