#include"vertex.h"
#include<iostream>
#include<cstdlib>
#include<ctime>
#include <windows.h>
#include<QDebug>
#include<QMessageBox>
using namespace std;


//顶点类构造函数
Vertex::Vertex()
{
    this->x = 0;
    this->y = 0;
    this->Weight = 0;
    this->EdgeNum = 0;
    this->TrafficFlow = 0;
    this->Mark = 0;
    this->EdgeRecord = NULL;
    this->TrafficMaxVolume = NULL;
    this->TrafficCurrentVolume = NULL;
}

//顶点析构函数
Vertex::~Vertex()
{
    if (this->getEdgeRecord())
    {
        delete[] this->EdgeRecord;
        this->EdgeRecord = NULL;
    }

    if (this->getTrafficMaxVolume())
    {
        delete[] this->TrafficMaxVolume;
        this->TrafficMaxVolume = NULL;
    }

    if (this->getTrafficCurrentVolume())
    {
        delete[] this->TrafficCurrentVolume;
        this->TrafficCurrentVolume = NULL;
    }

}

//记录某个顶点的对应属性
void Vertex::setVertexProperties(int X, int Y, int weight, int edgenum, int trafficflow, int mark, int* edge)
{
    this->setX(X);
    this->setY(Y);
    this->setWeight(weight);
    this->setEdgeCount(edgenum);
    this->setTrafficFlow(trafficflow);
    this->setMark(mark);
    this->setEdgeRecord(edgenum,edge);
}

void Vertex::printVertexInfo()
{
    cout << "坐标(" << this->x << "," << this->y << ")";
    cout << "  连边：";
    for (int i = 0; i < this->getEdgeCount(); i++)
    {
        cout << this->getOneEdgeRecord(i) << " " << this->getOneTrafficCurrentVolume(i) << " ";
    }
    cout << endl;
}
