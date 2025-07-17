#include<iostream>
#include<iomanip>
#include<cstdlib>
#include<ctime>
#include <windows.h>
#include<QDebug>
#include<QMessageBox>
#include"graphsystem.h"
using namespace std;


//GraphSystem的构造
GraphSystem::GraphSystem()
{
    MP = NULL;
    VertexList = NULL;
    VertexNumber = 0;
    NearestVertex = NULL;
    PathList = NULL;

    ShortestDistanceVertex = NULL;
    ShortestDistanceNumber = 0;

    ShortestTrafficVertex = NULL;
    ShortestTrafficNumber = 0;
}

//Graph的析构
GraphSystem::~GraphSystem()
{
    if (this->VertexList)
    {
        for (int i = 0; i < this->VertexNumber; i++)
        {
            if (this->VertexList[i])
            {
                delete[] this->VertexList[i];
                this->VertexList[i] = NULL;
            }
        }
        delete[] this->VertexList;
        this->VertexList = NULL;
    }

    if (this->PathList)
    {
        for (int i = 0; i < this->VertexNumber; i++)
        {
            if (this->PathList[i])
            {
                delete[] this->PathList[i];
                this->PathList[i] = NULL;
            }
        }
        delete[] this->PathList;
        this->PathList = NULL;
    }

    if (this->NearestVertex)
    {
        delete[] this->NearestVertex;
        this->NearestVertex = NULL;
    }

    if (this->ShortestDistanceVertex)
    {
        delete[] this->ShortestDistanceVertex;
        this->ShortestDistanceVertex = NULL;
    }

    if (this->ShortestTrafficVertex)
    {
        delete[] this->ShortestTrafficVertex;
        this->ShortestTrafficVertex = NULL;
    }

}

// //创建随机图
// void GraphSystem::createRandomGraph(int VNumber)
// {
//     this->VertexNumber = VNumber;

//     this->MP = new Map;
//     this->MP->initializeMap(0, VNumber, "123465.txt");

//     this->VertexList = new Vertex * [VNumber];

//     //将MP中的各顶点值导入Graph类的属性
//     for (int i = 0; i < VNumber; i++)
//     {
//         Vertex* v = new Vertex;
//         v->setVertexProperties(this->MP->getVertexX(i), this->MP->getVertexY(i), this->MP->getVertexWeight(i), this->MP->getEdgeCount(i), this->MP->getEdgeTrafficFlow(i), this->MP->getVertexMark(i), this->MP->getAdjacentVertices(i));
//         this->VertexList[i] = v;

//     }
// }

int GraphSystem::coordinateToVertex(const int&X,const int&Y)const
{return this->MP->findNearestVertex(X,Y);}

// //读取已有图
// void GraphSystem::readExistingGraph(const char* fileName)
// {
//     this->MP = new Map;
//     this->MP->initializeMap(1, 0, "123465.txt");

//     this->VertexNumber = this->MP->getVertexCount();

//     this->VertexList = new Vertex * [this->VertexNumber];

//     //将MP中的各顶点值导入Graph类的属性
//     for (int i = 0; i < this->VertexNumber; i++)
//     {
//         Vertex* v = new Vertex;
//         v->setVertexProperties(this->MP->getVertexX(i), this->MP->getVertexY(i), this->MP->getVertexWeight(i), this->MP->getEdgeCount(i), this->MP->getEdgeTrafficFlow(i), this->MP->getVertexMark(i), this->MP->getAdjacentVertices(i));
//         this->VertexList[i] = v;

//     }
// }

//获取单个边的最大车容量
int GraphSystem::calculateEdgeCapacity(const int& v1, const int& v2)
{
    return (int)this->MP->getEdgeCapacity(v1, v2);
}

//获取单个边的实际车流量
int GraphSystem::calculateEdgeReality(const int& v1, const int& v2)
{
    return (int)this->MP->getEdgeTraffic(v1, v2);
}

//计算每个点对应所有边的最大车流量
void GraphSystem::calculateAllCapacity()
{
    if (!this->VertexList)
    {
        return;
    }
    for (int i = 0; i < this->VertexNumber; i++)
    {
        this->VertexList[i]->setTrafficMaxVolume(this->VertexList[i]->getEdgeCount());
        //计算每条边的车流量
        for (int j = 0; j < this->VertexList[i]->getEdgeCount(); j++)
        {
            this->VertexList[i]->setOneTrafficMaxVolume(j,(int)this->MP->getEdgeCapacity(i,j));
        }
    }

}

//计算每个点对应所有边的实际车流量
void GraphSystem::calculateAllReality()
{
    if (!this->VertexList)
    {
        return;
    }
    for (int i = 0; i < this->VertexNumber; i++)
    {
        this->VertexList[i]->setTrafficCurrentVolume(this->VertexList[i]->getEdgeCount());
        //计算每条边的车流量
        for (int j = 0; j < this->VertexList[i]->getEdgeCount(); j++)
        {
            this->VertexList[i]->setOneTrafficCurrentVolume(j,(int)this->MP->getEdgeTraffic(i, j));

        }
    }

}
//计算最佳通行时间
void GraphSystem::calculateTrafficTime(const vector<int>&pathlist){
    trafficTime=0;
    for(size_t i=0;i<pathlist.size()-1;++i){
        int cap=calculateTrafficCondition(pathlist[i],pathlist[i+1]);
        if(cap<=10000){
            trafficTime+=MP->calculateDistance(pathlist[i],pathlist[i+1],1)*0.18;//通行时间直接加上距离来模拟
        }//表示当前道路畅通无比
        else{
            trafficTime+=MP->calculateDistance(pathlist[i],pathlist[i+1],1)*(1+2.7182*(cap/100000))*0.18;
        }
    }
}
//获取单个边的实际距离
int GraphSystem::calculateEdgeDistance(const int& v1, const int& v2)
{
    return (int)this->MP->calculateDistance(v1, v2, 1);
}

//获取单个边的综合距离
int GraphSystem::calculateTrafficDistance(const int& v1, const int& v2)
{
    return (int)this->MP->calculateDistance(v1, v2, 2);
}

//获取单个边的拥堵情况
int GraphSystem::calculateTrafficCondition(const int& v1, const int& v2)
{
    return (int)this->MP->calculateDistance(v1, v2, 3);
}

//查找最近的100个顶点
void GraphSystem::searchNearestVertex(int V)
{
    delete[] this->NearestVertex;//删去上次可能存在的100个点
    this->NearestVertex = new int[100];
    this->MP->searchVicinity(V);
    int count = 0;
    for (int i = 0; i <this->VertexNumber; i++)
    {
        if (this->MP->getVertexMark(i) >= 20)
        {
            this->NearestVertex[count] = i;
            count++;
        }
    }

}

//查找最短路径
void GraphSystem::findShortestDistancePath(const int& v1, const int& v2)
{
    this->dijkstraAllByDistance(v2);
    this->searchShortestDistance(v1, v2);
}

//查找综合最短路径
void GraphSystem::findShortestTrafficPath(const int& v1, const int& v2)
{
    this->dijkstraAllByTraffic(v2);
    this->searchShortestTraffic(v1, v2);
}

//根据边长度查找所有点到v1最短路径，并更新PathList
void GraphSystem::dijkstraAllByDistance(const int& v1)
{
    //首先获取v1的所有点的最短路径列表（模式1）
    this->MP->findDijkstraPath(v1, 1);

    //初始化PathList
    if (PathList)
    {
        for (int i = 0; i < this->VertexNumber; i++)
        {
            delete[] PathList[i];
            PathList[i] = NULL;
        }
        delete[] PathList;
        PathList = NULL;
    }
    //将MP的pathList导入
    this->PathList = new int* [this->VertexNumber];
    for (int i = 0; i < this->VertexNumber; i++)
    {
        PathList[i] = new int[3];

        for (int j = 0; j < 3; j++)
        {
            PathList[i][j] = this->MP->getPathNode(i, j);
        }
    }
}

//根据交通情况查找所有点到v1最短综合路径，更新PathList
void GraphSystem::dijkstraAllByTraffic(const int& v1)
{
    //首先获取v1的所有点的最短综合路径列表（模式2）
    this->MP->findDijkstraPath(v1, 2);

    //初始化PathList
    if (PathList)
    {
        for (int i = 0; i < this->VertexNumber; i++)
        {
            delete[] PathList[i];
            PathList[i] = NULL;
        }
        delete[] PathList;
        PathList = NULL;
    }
    //将MP的pathList导入
    this->PathList = new int* [this->VertexNumber];
    for (int i = 0; i < this->VertexNumber; i++)
    {
        PathList[i] = new int[3];

        for (int j = 0; j < 3; j++)
        {
            PathList[i][j] = this->MP->getPathNode(i, j);
        }
    }
}

//在PathList中查找v1到v2的最短路径，记录到ShortestDistanceVertex中
void GraphSystem::searchShortestDistance(const int& v1, const int& v2)
{
    //初始化ShortestVertex
    if (ShortestDistanceVertex)
    {
        delete[] ShortestDistanceVertex;
        ShortestDistanceVertex = NULL;
    }
    ShortestDistanceVertex = new int[this->VertexNumber + 1];

    //从V1循环查找前驱点，直到找到V2。记录经过的路径的数量
    int p = v1;
    int count = 0;
    while (p != v2)
    {
        ShortestDistanceVertex[count] = p;
        p = PathList[p][1];
        count++;
    }
    if (count <= this->VertexNumber)//检查，其实不管用担心
    {
        ShortestDistanceVertex[count] = p;//最后把v2存进来
        count++;
        ShortestDistanceNumber = count;
    }
}

//在PathList中查找v1到v2的最短路径,并记录到ShortestTrafficVertex中，其思路与最短路径的是一样的
void GraphSystem::searchShortestTraffic(const int& v1,const int& v2)
{

    //初始化ShortestVertex
    if (ShortestTrafficVertex)
    {
        delete[] ShortestTrafficVertex;
        ShortestTrafficVertex = NULL;
    }

    ShortestTrafficVertex = new int[this->VertexNumber + 1];
    //从v2循环查找前驱点，直到找到V1。记录经过的路径的数量
    int p = v1;
    int count = 0;
    while (p != v2)
    {
        ShortestTrafficVertex[count] = p;
        p = PathList[p][1];
        count++;
    }
    if (count <= this->VertexNumber)
    {
        ShortestTrafficVertex[count] = p;
        count++;
        ShortestTrafficNumber = count;
    }
}

//查询函数，获取两点最短路径的总长度
int GraphSystem::getShortestDistanceLength(const int& v1, const int& v2)
{
    this->MP->findDijkstraPath(v1, 1);
    return (int)this->MP->markShortestPath(v1, v2);
}

//查询函数，获取两点最短综合路径的总长度
int GraphSystem::getShortestTrafficLength(const int& v1, const int& v2)
{
    this->MP->findDijkstraPath(v1, 2);
    return (int)this->MP->markShortestPath(v1, v2);
}

//刷新车流
void GraphSystem::refreshTrafficFlow(const int& degree)
{
    this->MP->refreshAllTraffic(degree);
    for (int i = 0; i < this->VertexNumber; i++)
    {
        this->VertexList[i]->setTrafficFlow(this->MP->getEdgeTrafficFlow(i));
    }
}

//清空最近100点的PathList标记
void GraphSystem::refreshDisplayNearest()
{
    this->MP->refreshDisplayPriority(1);
}

//清空最短路径的PathList
void GraphSystem::refreshDisplayShortest()
{
    this->MP->refreshDisplayPriority(2);
}

//检查点的优先级
int GraphSystem::checkVertexVisibility(const int& v, const int& degree)
{
    return this->MP->isVertexVisible(v, degree);
}

//检查边的优先级
int GraphSystem::checkEdgeVisibility(const int& vA, const int& vB, const int& degree)
{
    return this->MP->isEdgeVisible(vA, vB, degree);
}

//输出每个顶点对应的信息
void GraphSystem::printGraphSystemInfo()
{
    for (int i = 0; i < this->VertexNumber; i++)
    {
        cout << "序号" << setw(5) << i << "  ";
        this->VertexList[i]->printVertexInfo();
    }
}

QPointF* GraphSystem::createPointsArray() {
    QPointF* points = new QPointF[this->VertexNumber];  // 动态分配内存

    for (int i = 0; i < this->VertexNumber; i++) {
        points[i] = QPointF(this->VertexList[i]->getX(), this->VertexList[i]->getY());
    }

    return points;  // 返回指向动态分配数组的指针
}
