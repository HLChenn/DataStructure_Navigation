#pragma once
#include <QPointF>
#include <Qpen>
#include <windows.h>
using namespace std;

//顶点类，记录了每个顶点的坐标以及连接的边的信息
class Vertex
{
private:
    int x;           //x坐标
    int y;           //y坐标
    int Weight;      //顶点权重
    int EdgeNum;     //边的数量
    int TrafficFlow; //模拟车流
    int Mark;        //访问标记
    int *EdgeRecord; //连边的记录。如Edge[3]=5表示这个顶点第4条边，连接了序号为5的点。遍历所有边依赖于EdgeNum
    int *TrafficMaxVolume;  //最大车容量
    int *TrafficCurrentVolume; //当前车流量

public:
    Vertex();
    ~Vertex();
    void setVertexProperties(int X,
                             int Y,
                             int weight,
                             int edgenum,
                             int trafficflow,
                             int mark,
                             int *edge); //设置点的信息，包括坐标等
    virtual void printVertexInfo();      //输出此结点所有信息

    //各类private成员的获取与设置函数
    int getX() const { return x; }
    int getY() const { return y; }
    int getEdgeCount() const { return EdgeNum; }
    int *getEdgeRecord() const { return EdgeRecord;}
    int getOneEdgeRecord(int i) const  { return EdgeRecord[i]; }
    int getTrafficFlow() const { return TrafficFlow;}
    int getMark() const { return Mark;}
    int* getTrafficMaxVolume() const { return TrafficMaxVolume; }
    int getOneTrafficMaxVolume(int i) const { return TrafficMaxVolume[i]; }
    int* getTrafficCurrentVolume() const { return TrafficCurrentVolume; }
    int getOneTrafficCurrentVolume(int i) const {return TrafficCurrentVolume[i]; }

    void setX(int X) { this->x = X; }
    void setY(int Y) { this->y = Y; }
    void setWeight(int weight) { this->Weight=weight; }
    void setEdgeCount(int edgenum) { this->EdgeNum=edgenum; }
    void setTrafficFlow(int trafficflow) { this->TrafficFlow=trafficflow; }
    void setMark(int mark) { this->Mark=mark; }
    void setEdgeRecord(int edgenum,int *edge) {
        this->EdgeRecord = new int[edgenum];
        for (int i = 0; i < edgenum; i++)
        {
            this->EdgeRecord[i] = edge[i];
        }}
    void setTrafficMaxVolume(int maxvolume){
        TrafficMaxVolume=new int[maxvolume];
    }
    void setOneTrafficMaxVolume(int i,int maxvolume){
        TrafficMaxVolume[i]=maxvolume;
    }
    void setTrafficCurrentVolume(int currentvolume){
        TrafficMaxVolume=new int[currentvolume];
    }
    void setOneTrafficCurrentVolume(int i,int currentvolume){
        TrafficMaxVolume[i]=currentvolume;
    }
};
