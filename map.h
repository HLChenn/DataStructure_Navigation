#pragma once
// #include"GraphImplement.h"
#include <vector>
#include <windows.h>
using namespace std;

class GraphSystem;
class Map // 无向图，邻接表形式
{
    friend GraphSystem;

public:
    int zoneCount;      // 区域数量
    int vertexCount;    // 顶点数量
    int **zoneTable;   // 区域列表
    int **crossEdges;  // 区域间连边列表
    int **vertexTable; // 顶点列表
    int **edgeTable;   // 全图连边列表
    int **pathTable;   // 最短路径列表

    int displayGraphInfo(const int &) const; // 监测输出地图有关信息的辅助函数

    inline int insertionSort(int **&,
                             const int &,
                             const int &,
                             const int &); // 插入排序辅助函数（默认降序排列）
    inline int quickSort(int **&,
                         const int &,
                         const int &,
                         const int &); // 快速排序辅助函数（默认降序排列）
    int searchInHeap(int **&, int, int);
    void deleteFromHeap(int **heap, int &heapSize, int pos, const int &sortMode);
    inline int siftDown(int **&, int, const int &, const int &); // 堆排序下筛辅助函数（默认小顶堆）
    inline int buildHeap(int **&, const int &, const int &);     // 堆创建辅助函数（默认小顶堆）
    inline int deleteMinFromHeap(int **&,
                                 int &,
                                 const int &,
                                 const int &); // 堆顶元素删除辅助函数（默认小顶堆）
    inline int insertElementToHeap(
        int **&, int &, const int *, const int &, const int &); // 堆元素插入辅助函数（默认小顶堆）

    int getEdgeCapacity(const int &, const int &) const;         // 获取连边最大流量的辅助函数
    int getEdgeTraffic(const int &, const int &) const;          // 获取连边当前流量的辅助函数
    int calculateDistance(const int &, const int &, const int &) const; // 获取二顶点间距离的辅助函数
    int checkEdgeIntersection(int **&,
                              const int &,
                              const int &,
                              const int &) const; // 用于连边交叉检查的辅助函数

    int generateVertices(const int &, const int &);   // 给定数量顶点随机生成函数
    int generateLocalEdges(const int &zoneIndex);       // 区域内连边生成函数（保证图的连通性及尽可能无交叉）
    int generateCrossEdges(const int &zoneCount);      // 跨区域连边生成函数

    vector<int> path;

public:
    Map();                                                     // 含参构造函数
    ~Map();                                                    // 析构函数
    int initializeMap(const int &, const int &, const char *); // 地图初始化函数
    int exportGraph(const char *) const;                       // 地图写入文本文件执行函数

    int findNearestVertex(const int &x, const int &y) const;

    int searchVicinity(const int &vertexIndex);                       // 查询与某顶点距离最近的100个顶点编号的函数
    int findDijkstraPath(const int &startVertex, const int &endVertex);            // Dijkstra算法最短路径查找函数
    int markShortestPath(const int &startVertex, const int &endVertex); // 最短路径标记函数

    int findAStarPath(const int &startVertex, const int &endVertex);
    // int Dijkstra(const int &vStart, const int &vEnd);

    int refreshAllTraffic(const int &); // 全图交通流量刷新函数
    int refreshDisplayPriority(const int &); // 显示优先级设置函数

    int isVertexVisible(const int &, const int &) const;            // 顶点显示可见性扫描确认函数
    int isEdgeVisible(const int &, const int &, const int &) const; // 连边显示可见性扫描确认函数

    int getVertexX(int vertexIndex) const;
    int getVertexY(int vertexIndex) const;
    int getEdgeCount(int vertexIndex) const;
    int getVertexWeight(int vertexIndex) const;
    int getEdgeTrafficFlow(int edgeIndex) const;
    int getVertexMark(int vertexIndex) const;
    int *getAdjacentVertices(int vertexIndex) const;
    int getPathNode(int pathIndex, int nodeIndex) const;
    int getVertexCount() const;
};
