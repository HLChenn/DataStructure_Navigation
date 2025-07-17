#pragma once
#include<iostream>
#include<vector>
#include <windows.h>
#include <QPointF>
#include <Qpen>
#include"map.h"
#include"vertex.h"

using namespace std;

class GraphSystem //创造出随机图，且具有图的常用数据结构。之后的函数接口都包装在这个类里面
{
public:
    Map* MP;
    Vertex** VertexList;//顶点指针数组，用于记录mp中每个顶点的信息
    //未重新定义边的类，是因为MapGraph中边的属性记录在了顶点中
    //故把边的所有信息都记录在Vertex中
    int VertexNumber;//记录地图导航系统里的点的数量，实则可以用map里返回点的数量得到
    int* NearestVertex;//记录最近的100个点。每次调用函数都会更新
    int** PathList;

    //用于距离最短路径的数据
    int* ShortestDistanceVertex;//记录最短路径上的所有点
    int ShortestDistanceNumber;//记录共经过多少点

    //用于综合最短路径的数据
    int* ShortestTrafficVertex;//记录最短路径上的所有点
    int ShortestTrafficNumber;//记录共经过多少点
    int trafficTime;//记录通行时间


    GraphSystem();//构造
    ~GraphSystem();//析构

    void createRandomGraph(int VNumber);//根据数目构建随机图,要求顶点数量大于0,小于100000
    void readExistingGraph(const char* fileName);//读取已有图

    int coordinateToVertex(const int&,const int&)const;

    int calculateEdgeCapacity(const int& v1, const int& v2);//获取单个边的最大车容量
    int calculateEdgeReality(const int& v1, const int& v2);//获取单个边的实际车流量
    void calculateAllCapacity();//获得边的最大容量，储存到TrafficVolume中
    void calculateAllReality();//获得边实际车流量，储存到TrafficReality中

    int calculateEdgeDistance(const int& v1, const int& v2);//获取单个边的实际距离
    int calculateTrafficDistance(const int& v1, const int& v2);//考虑车流情况下，获取单个边的综合距离
    int calculateTrafficCondition(const int& v1, const int& v2);//获取单个连边的拥堵情况
    void calculateTrafficTime(const vector<int>&pathlist);//获取最佳通行时间

    void searchNearestVertex(int V);//寻找点V最近的100个点，储存到NearestVertex中

    void findShortestDistancePath(const int& v1, const int& v2);//查找两点间最短路径
    void findShortestTrafficPath(const int& v1, const int& v2);//查找两点间综合最短路径
    void dijkstraAllByDistance(const int& v1);//根据边长度查找所有点到v1最短路径，并更新PathList
    void dijkstraAllByTraffic(const int& v1);//根据综合交通情况查找所有点到v1最短路径，更新PathList
    void searchShortestDistance(const int& v1, const int& v2);//在PathList中查找v1到v2的最短路径
    void searchShortestTraffic(const int& v1, const int& v2);//在PathList中查找v1到v2的最短综合路径

    void refreshTrafficFlow(const int& degree);//刷新TrafficFlow值，以便更新最大容量和实际车流量
    void refreshDisplayNearest();//刷新最近100点的PathList标记
    void refreshDisplayShortest();//刷新最短路径的PathList

    int checkVertexVisibility(const int& v, const int& degree);//检查点的级别（优先级）
    int checkEdgeVisibility(const int& vA, const int& vB, const int& degree);//检查连边的级别（优先级）
    int getShortestDistanceLength(const int& v1, const int& v2);//查询函数，获取两点最短路径的总长度
    int getShortestTrafficLength(const int& v1, const int& v2);//查询函数，获取两点最短综合路径的总长度


    //每个区域中最重要的四个点
    vector<vector<vector<int>>> Zones;//二维数组定位，一维数组标点
    void findImportantVertices(){
        Zones.clear();
        int znum=MP->zoneCount;
        int firstV;
        int tot=0;
        for(int i=0;i<znum;i++){
            vector<vector<int>> _zone;
            for(int j=0;j<znum;j++){
                vector<int> Zone_;
                firstV=MP->zoneTable[tot][1];
                cout<<"zonelist: "<<tot<<"  "<<firstV<<endl;
                for(int k=0;k<4;k++){
                    Zone_.push_back(firstV++);
                }
                _zone.push_back(Zone_);
                tot++;
            }
            Zones.push_back(_zone);
        }

    }



    void printGraphSystemInfo();//输出所有结点的坐标及对应连边信息
    QPointF* createPointsArray();
};
