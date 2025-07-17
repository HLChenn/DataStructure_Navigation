#include "map.h"

#include<iostream>
#include<iomanip>
#include<fstream>
#include<cstdlib>
#include<ctime>
#include<cmath>
#include <windows.h>
#include<QDebug>
#include<QMessageBox>
#include <queue>
using namespace std;

Map::Map() : zoneCount(0), vertexCount(0), zoneTable(nullptr) {}

Map::~Map()
{
    // 释放动态内存
}

int Map::initializeMap // 地图初始化函数
    (const int &initMode, const int &vertexNumInput, const char *file)
{ // 参数initMode指示创建模式，vertexNumInput指示所需的顶点数量,fileName指示需要读入的文本文件名称
    if (this->vertexCount != 0)
    { // 保险机制，若图中当前顶点数量非0则判定其已经初始化，报错并直接退出
        cout << "Initialization Fail: MapGraph Already Initialized" << '\n';
        // QMessageBox::warning(nullptr,"错误","当前已初始化地图");
        return 0;
    }
    else
    {
        switch (initMode)
        {
        case 0:
        { // 模式0：新建地图内容，随机生成一定数量的顶点，按一定规律生成连边
            if (vertexNumInput <= 9999 || vertexNumInput >= 100001)
            { // 保险机制，若传入参数代表的顶点数量超限则报错并直接退出
                cout << "Initialization Fail: Total Vertex Number Out of Range" << '\n';
                // QMessageBox::warning(nullptr,"错误","当前地图顶点数超过限制");
                return 0;
            }
            else
            {
                int i = 0;
                if (vertexNumInput >= 10000 && vertexNumInput <= 15000)
                {
                    this->zoneCount = 16;
                } // 根据待生成的顶点数量动态确定区域数量和大小
                else if (vertexNumInput >= 15001 && vertexNumInput <= 30000)
                {
                    this->zoneCount = 20;
                }
                else if (vertexNumInput >= 30001 && vertexNumInput <= 60000)
                {
                    this->zoneCount = 25;
                }
                else if (vertexNumInput >= 60001 && vertexNumInput <= 100000)
                {
                    this->zoneCount = 40;
                }
                else
                {
                    ;
                }
                this->vertexCount = vertexNumInput;
                this->zoneTable = new int *[this->zoneCount * this->zoneCount + 1]; // 创建区域列表
                this->vertexTable = new int *[vertexNumInput];                            // 创建顶点列表
                this->edgeTable = new int *[vertexNumInput];                              // 创建全图连边列表
                this->pathTable = new int *[vertexNumInput];                              // 创建最短路径存储列表
                for (i = 0; i <= (this->zoneCount * this->zoneCount); i++)
                {                                   // 初始化区域列表
                    this->zoneTable[i] = new int[3]; // 0：区域权值，1：区域顶点范围，2：区域间连边范围
                    this->zoneTable[i][0] = 0;       // 0：区域权值，与区域内顶点数量成正比，与区域位置相对地图中心的距离成反比
                    this->zoneTable[i][1] = 0;       // 1：区域顶点范围，作为顶点列表vertexTable的索引表
                    this->zoneTable[i][2] = 0;       // 2：区域间连边范围，作为区域间连边列表crossEdges的索引表
                }
                for (i = 0; i <= vertexNumInput - 1; i++)
                {                                     // 初始化顶点列表
                    this->vertexTable[i] = new int[5]; // 0：顶点坐标，1：顶点权值，2：顶点边数，3：随机参数权值，4：访问标记
                    this->vertexTable[i][0] = 0;       // 0：顶点坐标(x,y)取值范围0~9999，以整形数据（10000*x+y）形式存储
                    this->vertexTable[i][1] = 0;       // 1：顶点权值，取为该顶点到其所在区域内其它顶点的距离平方和
                    this->vertexTable[i][2] = 0;       // 2：顶点边数，代表以该顶点为起点的连边数量
                    this->vertexTable[i][3] = 0;       // 3：随机参数权值，用于模拟车流时使用
                    this->vertexTable[i][4] = 0;       // 4：访问标记，用于查询显示地图时使用
                    this->pathTable[i] = new int[3];   // 0：最短路径距离，1：前驱顶点，2：访问标记
                    this->pathTable[i][0] = (-1);      // 0：Dijkstra算法中当前每个顶点到起始点的最短路径距离
                    this->pathTable[i][1] = (-1);      // 1：Dijkstra算法中当前顶点最短路径的前驱顶点
                    this->pathTable[i][2] = 0;         // 2：顶点的访问和顺序标记，用于查询显示最短路径时使用
                    this->edgeTable[i] = NULL;         // 以变长二维数组的形式存储全图连边的邻接表
                }
                this->generateVertices(this->zoneCount, this->vertexCount); // 随机生成顶点
                for (i = 0; i <= (this->zoneCount * this->zoneCount - 1); i++)
                { // 生成每个区域内部的连边
                    this->generateLocalEdges(i);
                }
                this->generateCrossEdges(this->zoneCount); // 生成区域之间的跨区域连边
                this->refreshAllTraffic(0);                // 初始化全图的交通流量数据
                this->refreshDisplayPriority(0);
                // this->displayGraphInfo(4);
                // this->displayGraphInfo(0); // 监测输出地图区域数据是否正确生成
                // this->displayGraphInfo(3); // 监测输出跨区域连边数据是否正确生成
                // this->displayGraphInfo(2); // 监测输出顶点及全图连边数据是否正确生成
            }
            break;
        }
        case 1:
        {                                     // 模式1：从记录地图数据的文本文件中读取地图数据以创建地图内容
            ifstream datafile(file, ios::in); // 以只读方式打开数据记录文本文件
            if (!datafile.is_open())
            { // 保险机制，若文本文件打开失败则报错并直接退出
                // cout << "Initialization Fail: Error in Opening Source File" << '\n';
                // QMessageBox::warning(nullptr,"错误","地图文件打开失败！");
                return 0;
            }
            else
            {
                char title[256];
                datafile.getline(title, 256, '\n');           // 读取文本文件的标题（可优化为文件安全性检查）
                datafile >> this->zoneCount >> this->vertexCount; // 读取地图数据中的区域数量和顶点数量
                if (this->zoneCount <= 15 || this->zoneCount >= 41 || (10000 % this->zoneCount) != 0 || this->vertexCount <= 9999 || this->vertexCount >= 100001)
                { // 保险机制，若传入参数代表的区域数量或顶点数量超限则报错并直接退出
                    cout << "Initialization Fail: Total Vertex Number Out of Range" << '\n';
                    // QMessageBox::warning(nullptr,"错误","当前已初始化地图");
                    datafile.close();
                    return 0;
                }
                else
                {
                    int i = 0, j = 0;
                    this->zoneTable = new int *[this->zoneCount * this->zoneCount + 1]; // 创建区域列表
                    this->vertexTable = new int *[this->vertexCount];                 // 创建顶点列表
                    this->edgeTable = new int *[this->vertexCount];                   // 创建全图连边列表
                    this->pathTable = new int *[this->vertexCount];                   // 创建最短路径存储列表
                    for (i = 0; i <= (this->zoneCount * this->zoneCount); i++)
                    { // 初始化区域列表
                        this->zoneTable[i] = new int[3];
                        this->zoneTable[i][0] = 0;
                        this->zoneTable[i][1] = 0;
                        this->zoneTable[i][2] = 0;
                    }
                    for (i = 0; i <= this->vertexCount - 1; i++)
                    { // 初始化顶点列表
                        this->vertexTable[i] = new int[5];
                        this->vertexTable[i][0] = 0;
                        this->vertexTable[i][1] = 0;
                        this->vertexTable[i][2] = 0;
                        this->vertexTable[i][3] = 0;
                        this->vertexTable[i][4] = 0;
                        this->pathTable[i] = new int[3];
                        this->pathTable[i][0] = (-1);
                        this->pathTable[i][1] = (-1);
                        this->pathTable[i][2] = 0;
                        this->edgeTable[i] = NULL;
                    }
                    for (i = 0; i <= (this->zoneCount * this->zoneCount); i++)
                    { // 读取区域列表部分的数据并初始化之
                        datafile >> j >> this->zoneTable[i][0] >> this->zoneTable[i][1] >> this->zoneTable[i][2];
                    }
                    this->crossEdges = new int *[this->zoneTable[this->zoneCount * this->zoneCount][2]]; // 创建区域间连边列表
                    for (i = 0; i <= this->zoneTable[this->zoneCount * this->zoneCount][2] - 1; i++)
                    { // 读取区域间连边列表部分的数据并初始化之
                        this->crossEdges[i] = new int[3];
                        datafile >> j >> this->crossEdges[i][0] >> this->crossEdges[i][1] >> this->crossEdges[i][2];
                    }
                    for (i = 0; i <= this->vertexCount - 1; i++)
                    {
                        datafile >> j >> this->vertexTable[i][0] >> this->vertexTable[i][1] >> this->vertexTable[i][2];
                        this->edgeTable[i] = new int[this->vertexTable[i][2]]; // 创建全图连边列表
                        for (j = 0; j <= this->vertexTable[i][2] - 1; j++)
                        { // 读取全图连边列表部分的数据并初始化之
                            datafile >> this->edgeTable[i][j];
                        }
                    }
                    this->refreshAllTraffic(0); // 初始化全图的交通流量数据
                    this->refreshDisplayPriority(0);
                    this->displayGraphInfo(0); // 监测输出，同上
                    this->displayGraphInfo(3);
                    this->displayGraphInfo(2);
                }
                datafile.close();
            }
            break;
        }
        default:
        {
            break;
        }
        }
    }
    return 1;
}

int Map::generateVertices // 给定数量顶点随机生成函数
    (const int &zoneCountInput, const int &vertexCountInput)
{                                            // 参数zoneCountInput指示抽象地图中划分区域数量的平方根，vertexCountInput指示随机生成的顶点数量
    int i = 0, j = 0, k = 0, repeatFlag = 0; // 重复标记参数repeatFlag用于后续检查随机顶点坐标是否交叉时使用
    srand((unsigned int)time(NULL));         // 生成（伪）随机数种子

    for (i = 0; i <= vertexCountInput - 1; i++)
    { // 复用（即暂时占用）顶点列表第二列，生成各个顶点的随机数横坐标
        this->vertexTable[i][2] = (rand() % 10000) * 10000;
    }
    for (i = 0; i <= vertexCountInput - 1; i++)
    { // 复用（即暂时占用）顶点列表第二列，生成各个顶点的随机数纵坐标
        this->vertexTable[i][2] += (rand() % 10000);
    }

vertexReFlag: // 遍历顶点列表第二列，检查随机生成的顶点坐标是否重复，直至无重复为止
    for (i = 0; i <= vertexCountInput - 1; i++)
    {
        for (j = i + 1; j <= vertexCountInput - 1; j++)
        {
            if (this->vertexTable[i][2] == this->vertexTable[j][2])
            {
                this->vertexTable[j][2] = (rand() * 10000 + rand()); // 重新随机生成顶点的横纵坐标
                this->vertexTable[j][2] %= 100000000;
                // cout << '#' << j << '#' << this->vertexTable[j][2] << '\t' << "repetition" << '\n';
                repeatFlag = 1;
            } // 将重复标记置1，表示当前一趟检查中发现重复坐标
            else
            {
                continue;
            }
        }
    }
    if (repeatFlag != 0)
    {
        repeatFlag = 0;
        goto vertexReFlag;
    } // 上一趟检查中发现重复的坐标，需要再次检查（重复标记重置为0）
    else
    {
        // cout << "Repetition Annihilated" << '\n'
        //      << '\n';
    } // 检查完成，所有顶点的坐标均无重复值

    for (i = 0; i <= vertexCountInput - 1; i++)
    { // 复用顶点列表第三列，其中标记该顶点所属的区域编号
        this->vertexTable[i][3] =
            (((this->vertexTable[i][2] / 10000) % 10000) / (10000 / zoneCountInput)) * zoneCountInput + (this->vertexTable[i][2] % 10000) / (10000 / zoneCountInput); // 分别顶点的横纵坐标所属的区域编号，按照（zoneCountInput）进制叠加得到顶点所属的区域编号
        this->zoneTable[this->vertexTable[i][3] % (zoneCountInput * zoneCountInput)][1]++;
    } // 区域列表第1列统计区域内部包含的顶点数量
    for (i = 0; i <= (zoneCountInput * zoneCountInput - 1); i++)
    { // 累加区域内顶点数量得到顶点存储编号的索引值（参考桶排序原理）
        this->zoneTable[i + 1][1] += this->zoneTable[i][1];
    }
    for (i = vertexCountInput - 1; i >= 0; i--)
    { // 使用桶排序算法将各个顶点坐标按照所属区域顺序分区域放置在顶点列表的第零列
        this->vertexTable[--this->zoneTable[this->vertexTable[i][3]][1]][0] = this->vertexTable[i][2];
    }
    for (i = 0; i <= (zoneCountInput * zoneCountInput - 1); i++)
    { // 遍历各个区域，按照区域内权值大小在区域内排列各个区域的顶点
        for (j = this->zoneTable[i][1]; j <= this->zoneTable[i + 1][1] - 1; j++)
        {
            for (k = this->zoneTable[i][1]; k <= this->zoneTable[i + 1][1] - 1; k++)
            {
                this->vertexTable[j][1] += this->calculateDistance(k, j, 0);
            } // 对于每个顶点，将其与所在区域内其它顶点距离的平方和作为其在区域内的权值倒数
            this->vertexTable[j][1] = (vertexCountInput >= 60001 ? 400000000000 : 1000000000000) / (this->vertexTable[j][1] + 1);
        } // 对上述距离平方和取倒数，得到各个顶点在区域内的权值
        this->quickSort(this->vertexTable, this->zoneTable[i][1], this->zoneTable[i + 1][1], 1); // 使用优化后的快速排序算法对每个区域内部的顶点排序
        this->insertionSort(this->vertexTable, this->zoneTable[i][1], this->zoneTable[i + 1][1], 1);
    }
    for (i = 0; i <= vertexCountInput - 1; i++)
    { // 将先前复用的顶点列表第二，三列重置为0
        this->vertexTable[i][2] = 0;
        this->vertexTable[i][3] = 0;
    }
    for (i = 0; i <= (zoneCountInput * zoneCountInput - 1); i++)
    { // 遍历区域列表，计算区域权值
        this->zoneTable[i][0] =
            (((zoneCountInput / 2) - (i / zoneCountInput)) * ((zoneCountInput / 2) - (i / zoneCountInput)) + ((zoneCountInput / 2) - (i % zoneCountInput)) * ((zoneCountInput / 2) - (i % zoneCountInput))); // 计算区域中心与全图中心之间的相对距离作为其权值倒数的一部分
        this->zoneTable[i][0] =
            (((this->zoneTable[i + 1][1] - this->zoneTable[i][1]) * 10000) / (this->zoneTable[i][0] + 1));
    } // 对上述距离取倒数，再乘以区域内顶点数量，结果作为区域的权值
    return 1;
}

int Map::generateLocalEdges // 区域内连边生成函数（保证图的连通性及尽可能无交叉）
    (const int &zoneIndex)
{ // 参数zoneIndex指示需要建立（区域内）连边的区域编号
    if (zoneIndex <= (-1) || zoneIndex >= (this->zoneCount * this->zoneCount))
    {
        return 0;
    } // 若传入的区域编号参数超限则直接退出函数
    else
    {
        int i = 0, vertexQueueCount = 0, vertexHeapCount = 0, edgeCount = 0, currentVertexPos = 0, currentVertexDest = 0, // 详见下述，currentVertexPos指示当前正在处理的起始顶点，currentVertexDest指示当前正在处理的目标顶点
            zoneVertexCount = (this->zoneTable[zoneIndex + 1][1] - this->zoneTable[zoneIndex][1]);          // zoneVertexCount指示区域内的顶点数量
        int *vertexQueue = new int[zoneVertexCount];                                               // vertexQueue代表当前处理顶点的队列，vertexQueueCount指示其中已经加入队列的顶点数量
        int **vertexHeap = new int *[zoneVertexCount];                                             // vertexHeap代表待处理顶点的堆（非真正堆，仅用排序模拟），vertexHeapCount指示其中剩余顶点的数量
        int **tempEdgeList = new int *[zoneVertexCount * zoneVertexCount];                                  // tempEdgeList为临时连边列表，edgeCount指示其中已经添加的连边数量（经实验，此处申请足够大而冗余的内存空间可显著提升运行速度）
        for (i = 0; i <= zoneVertexCount - 1; i++)
        { // 初始化当前处理顶点的队列和待处理顶点堆（注意已经处理的顶点也可能再次被处理，详见下述）
            vertexQueue[i] = 0;
            vertexHeap[i] = new int[2];
            vertexHeap[i][0] = 0; // 0：顶点编号
            vertexHeap[i][1] = 0;
        } // 1：顶点到当前处理顶点的距离
        for (i = 0; i <= (zoneVertexCount * zoneVertexCount - 1); i++)
        { // 初始化临时连边列表
            tempEdgeList[i] = new int[2];
            tempEdgeList[i][0] = 0; // 0：连边的起始顶点
            tempEdgeList[i][1] = 0;
        } // 1：连边的终止顶点
        vertexQueue[vertexQueueCount++] = this->zoneTable[zoneIndex][1]; // 将区域内第一个顶点（权值最高的顶点）加入当前处理顶点的队列（注意此时currentVertexPos初始化为0）

    addLocalEdgeFlag:
        for (; currentVertexPos <= vertexQueueCount - 1; currentVertexPos++)
        {                                              // 遍历区域内顶点直至当前处理顶点的队列为空（理想情况下可遍历遍历区域内的所有顶点）
            this->vertexTable[vertexQueue[currentVertexPos]][3] = 1; // 当前处理顶点的队列中currentVertexPos位置的顶点将被处理，将其访问标记置为1以表示已访问和处理（复用顶点列表第三列）
            for (i = 0; i <= zoneVertexCount - 1; i++)
            { // 对于区域内每个正在处理的顶点，将区域内其它顶点按照到其的距离由远及近排列，存放于待处理顶点堆中
                vertexHeap[i][0] = i + this->zoneTable[zoneIndex][1];
                vertexHeap[i][1] = this->calculateDistance(vertexQueue[currentVertexPos], i + this->zoneTable[zoneIndex][1], 0);
            }
            this->quickSort(vertexHeap, 0, zoneVertexCount, 1);
            this->insertionSort(vertexHeap, 0, zoneVertexCount, 1); // 此时区域内各个顶点按照其到正在处理的顶点的距离降序排列

            for (i = 0, vertexHeapCount = zoneVertexCount - 1; i <= 3 && vertexHeapCount >= 0; vertexHeapCount--)
            {                                  // 对每个正在处理的顶点添加不多于4条连边
                currentVertexDest = vertexHeap[vertexHeapCount][0]; // vertexHeapCount递减以按照由近及远的顺序取出vertexHeap中的顶点，作为待生成连边终点的考察对象存放在currentVertexDest中
                if (currentVertexDest == vertexQueue[currentVertexPos])
                {
                    continue;
                } // 若待生成连边终点与正在处理的顶点是同一个顶点则直接取出下一个（不需要也不允许出现自连边成环的情况）
                else if (this->checkEdgeIntersection(tempEdgeList, edgeCount, vertexQueue[currentVertexPos], currentVertexDest) == 0)
                { // 检查待生成的连边与已经添加的连边是否发生交叉，若否则添加该连边
                    tempEdgeList[edgeCount][0] = vertexQueue[currentVertexPos];
                    tempEdgeList[edgeCount++][1] = currentVertexDest;
                    tempEdgeList[edgeCount][0] = currentVertexDest;
                    tempEdgeList[edgeCount++][1] = vertexQueue[currentVertexPos]; // 地图为无向图，因此向临时连边列表中同时添加起始顶点和终止顶点互换的两条连边
                    if (this->vertexTable[currentVertexDest][3] == 0)
                    {                                  // 若已添加连边的终止顶点至此尚未被访问，则将其加入队列
                        vertexQueue[vertexQueueCount++] = currentVertexDest; // 将已添加连边的终止顶点加入当前处理顶点的队列
                        this->vertexTable[currentVertexDest][3] = 2;
                    } // 复用顶点列表第三列，将已添加连边的终止顶点置为2以标记其已访问但尚未处理
                    else
                    {
                        ;
                    }
                    this->vertexTable[vertexQueue[currentVertexPos]][2]++; // 在顶点列表中分别将新添加连边两个顶点的边数统计加1（顶点列表第二列存储该顶点对应的连边数量）
                    this->vertexTable[currentVertexDest][2]++;
                    i++;
                }
                else
                {
                    continue;
                }
            }
        } // 当前连边与已有连边发生交叉，不应当添加，则返回取出下一个待考察的连边终止顶点

        if (currentVertexPos <= zoneVertexCount - 1)
        { // 若上述当前一轮循环并未遍历区域内每一个顶点，说明有顶点尚未被访问和添加连边
            for (i = this->zoneTable[zoneIndex][1]; i <= this->zoneTable[zoneIndex + 1][1] - 1 && this->vertexTable[i][3] != 0; i++)
                ;                               // 遍历区域内顶点，找到（按照权值从大到小）第一个尚未访问的顶点
            vertexQueue[vertexQueueCount++] = i;              // 将找到的尚未访问的顶点添加进入当前处理顶点的队列（注意，此处无需标记其为已经访问）
            currentVertexDest = this->zoneTable[zoneIndex][1]; // 需要将该新添加的顶点连接至区域内上述已经生成连边的连通子图（若无更合适的目标顶点，则选择区域内权值最大的顶点）
            for (i = this->zoneTable[zoneIndex][1] + 1; i <= this->zoneTable[zoneIndex + 1][1] - 1; i++)
            { // 遍历区域内顶点，选出与当前顶点距离最近且连边无交叉的属于连通子图中的顶点作为待生成连边的终止顶点
                if (this->vertexTable[i][3] != 0 && this->checkEdgeIntersection(tempEdgeList, edgeCount, vertexQueue[currentVertexPos], i) == 0 && this->calculateDistance(vertexQueue[currentVertexPos], i, 0) <= this->calculateDistance(vertexQueue[currentVertexPos], currentVertexDest, 0) - 1)
                {
                    currentVertexDest = i;
                }
                else
                {
                    continue;
                }
            }
            if (i >= this->zoneTable[zoneIndex + 1][1])
            { // 若上述搜索仍然不能找到合适的目标顶点
                for (i = 0; i <= zoneVertexCount - 1; i++)
                { // 对于区域内每个正在处理的顶点，将区域内其它顶点按照到其的距离由远及近排列，存放于待处理顶点堆中
                    vertexHeap[i][0] = i + this->zoneTable[zoneIndex][1];
                    vertexHeap[i][1] = this->calculateDistance(vertexQueue[currentVertexPos], i + this->zoneTable[zoneIndex][1], 0);
                }
                this->quickSort(vertexHeap, 0, zoneVertexCount, 1);
                this->insertionSort(vertexHeap, 0, zoneVertexCount, 1); // 此时区域内各个顶点按照其到正在处理的顶点的距离降序排列
                for (vertexHeapCount = zoneVertexCount - 1; vertexHeapCount >= 0; vertexHeapCount--)
                {
                    currentVertexDest = vertexHeap[vertexHeapCount][0]; // vertexHeapCount递减以按照由近及远的顺序取出vertexHeap中的顶点，作为待生成连边终点的考察对象存放在currentVertexDest中
                    if (this->vertexTable[currentVertexDest][3] != 0)
                    {
                        break;
                    } // 选择距离该新添加顶点最近的已经被访问的顶点作为待生成连边终点，不再考虑交叉现象
                    else
                    {
                        continue;
                    }
                }
            }
            else
            {
                ;
            }
            tempEdgeList[edgeCount][0] = vertexQueue[currentVertexPos];
            tempEdgeList[edgeCount++][1] = currentVertexDest;
            tempEdgeList[edgeCount][0] = currentVertexDest;
            tempEdgeList[edgeCount++][1] = vertexQueue[currentVertexPos];
            this->vertexTable[vertexQueue[currentVertexPos]][2]++;
            this->vertexTable[currentVertexDest][2]++; // 将新添加的顶点与区域内连通子图中符合上述条件的顶点间建立一条连边，方法同上
            goto addLocalEdgeFlag;
        } // 返回添加连边的循环，以新添加的顶点为起始顶点重启添加连边的搜索过程，如此直至遍历区域内每一个顶点为止

        // cout << '\n'
        //      << '(' << currentVertexPos << ',' << vertexQueueCount << ',' << edgeCount << ')' << '\n';
        // for (i = this->zoneTable[zoneIndex][1]; i <= this->zoneTable[zoneIndex + 1][1] - 1; i++)
        // {
        //     if (this->vertexTable[i][2] == 0)
        //     {
        //         cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << '\n'
        //              << '\n';
        //         break;
        //     }
        //     else
        //     {
        //         continue;
        //     }
        // }

        this->quickSort(tempEdgeList, 0, edgeCount, 1);
        this->insertionSort(tempEdgeList, 0, edgeCount, 1); // 对临时连边列表按照其终止顶点的编号大小降序排序
        for (i = this->zoneTable[zoneIndex][1]; i <= this->zoneTable[zoneIndex + 1][1] - 1; i++)
        { // 对区域内每个顶点按照上述统计得到的连边数量在连边列表中分配对应大小的内存空间
            this->edgeTable[i] = new int[this->vertexTable[i][2]];
        }
        for (i = edgeCount - 1; i >= 0; i--)
        { // 递减遍历临时连边列表，将其中的连边转存至地图的（永久）连边列表
            this->edgeTable[tempEdgeList[i][0]][this->vertexTable[tempEdgeList[i][0]][3] - 1] = tempEdgeList[i][1];
            this->vertexTable[tempEdgeList[i][0]][3]++;
        } // 注意，上述连边添加完成后顶点列表第三列应当均为1，复用之作为对应顶点的连边列表添加连边信息时的临时迭代器

        for (i = 0; i <= zoneVertexCount - 1; i++)
        {
            delete[] vertexHeap[i];
            vertexHeap[i] = NULL;
            this->vertexTable[i + this->zoneTable[zoneIndex][1]][3] = 0;
        } // 将上述复用的顶点列表第三列重置为0
        for (i = 0; i <= zoneVertexCount * zoneVertexCount - 1; i++)
        {
            delete[] tempEdgeList[i];
            tempEdgeList[i] = NULL;
        }
        delete[] vertexQueue;
        vertexQueue = NULL;
        delete[] vertexHeap;
        vertexHeap = NULL;
        delete[] tempEdgeList;
        tempEdgeList = NULL;
        return 1;
    }
}

int Map::generateCrossEdges // 跨区域连边生成函数
    (const int &zoneCountInput)
{                                                                                                          // 参数zoneCountInput指示抽象地图中划分区域数量的平方根
    int i = 0, j = 0, zoneQueueCount = 0, zoneHeapCount = 0, edgeCount = 0, currentZonePos = 0, currentZoneDest = 0, intersectionFlag = 0; // 各同名参数（除v变为z外）意义同上述区域内连边生成函数，intersectionFlag用作连边交叉检查标记
    int *zoneMark = new int[zoneCountInput * zoneCountInput];                                                                     // zoneMark为各个区域的访问标记，意义同上述区域内连边生成函数中复用的顶点列表第三列
    int *zoneQueue = new int[zoneCountInput * zoneCountInput];                                                                    // 各同名数组（除v变为z外）意义同上述区域内连边生成函数
    int **zoneHeap = new int *[zoneCountInput * zoneCountInput];
    int **tempEdgeList = new int *[zoneCountInput * zoneCountInput * 100];
    for (i = 0; i <= (zoneCountInput * zoneCountInput - 1); i++)
    { // 初始化各个临时功能数组
        zoneMark[i] = 0;
        zoneQueue[i] = 0;
        zoneHeap[i] = new int[2];
        zoneHeap[i][0] = 0;
        zoneHeap[i][1] = 0;
    }
    for (i = 0; i <= (zoneCountInput * zoneCountInput * 100 - 1); i++)
    {
        tempEdgeList[i] = new int[2];
        tempEdgeList[i][0] = 0;
        tempEdgeList[i][1] = 0;
    }
    for (i = 0; i <= (zoneCountInput * zoneCountInput - 1); i++)
    { // 遍历区域列表，选出权值最大的区域
        if (this->zoneTable[i][0] > this->zoneTable[j][0])
        {
            j = i;
        }
        else
        {
            continue;
        }
    }
    zoneQueue[zoneQueueCount++] = j; // 将上述选择得到的最大权值区域加入当前处理区域的队列

addCrossEdgeFlag:
    for (; currentZonePos <= zoneQueueCount - 1; currentZonePos++)
    { // 添加区域间连边的处理过程与上述区域内连边生成函数一致，具体不同仅为将顶点编号换成区域编号
        zoneMark[zoneQueue[currentZonePos]] = 1;
        for (i = 0; i <= (zoneCountInput * zoneCountInput - 1); i++)
        {
            zoneHeap[i][0] = i;
            zoneHeap[i][1] = ((i / zoneCountInput - zoneQueue[currentZonePos] / zoneCountInput) * (i / zoneCountInput - zoneQueue[currentZonePos] / zoneCountInput) + (i % zoneCountInput - zoneQueue[currentZonePos] % zoneCountInput) * (i % zoneCountInput - zoneQueue[currentZonePos] % zoneCountInput));
        }
        this->quickSort(zoneHeap, 0, zoneCountInput * zoneCountInput, 1);
        this->insertionSort(zoneHeap, 0, zoneCountInput * zoneCountInput, 1); // 此时区域内各个顶点按照其到正在处理的顶点的距离降序排列
        for (i = 0, zoneHeapCount = (zoneCountInput * zoneCountInput - 1); i <= 3 // 此处规定每个区域连接到其它区域的连边数量最多为4条，且不能超过该区域内包含的顶点数量
                                                                           && this->zoneTable[zoneQueue[currentZonePos]][2] <= (this->zoneTable[currentZonePos + 1][1] - this->zoneTable[currentZonePos][1] - 1) && zoneHeapCount >= 0;
             zoneHeapCount--)
        {
            currentZoneDest = zoneHeap[zoneHeapCount][0];
            if (currentZoneDest == zoneQueue[currentZonePos])
            {
                continue;
            }
            else
            {
                intersectionFlag = 0; // 上述用于检查连边交叉情况的checkEdgeIntersection函数不适配此处区域的情况，因此复写相关处理操作，但仅检查待生成的连边是否与已添加的连边发生重复，而不考虑连边之间的交叉
                for (j = 0; j <= edgeCount - 1; j++)
                {
                    if ((tempEdgeList[j][0] == zoneQueue[currentZonePos]) && (tempEdgeList[j][1] == currentZoneDest))
                    {
                        intersectionFlag = 1;
                        break;
                    }
                    else if ((tempEdgeList[j][1] == zoneQueue[currentZonePos]) && (tempEdgeList[j][0] == currentZoneDest))
                    {
                        intersectionFlag = 1;
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
                if (intersectionFlag == 0)
                {
                    tempEdgeList[edgeCount][0] = zoneQueue[currentZonePos];
                    tempEdgeList[edgeCount++][1] = currentZoneDest;
                    tempEdgeList[edgeCount][0] = currentZoneDest;
                    tempEdgeList[edgeCount++][1] = zoneQueue[currentZonePos];
                    if (zoneMark[currentZoneDest] == 0)
                    {
                        zoneQueue[zoneQueueCount++] = currentZoneDest;
                        zoneMark[currentZoneDest] = 2;
                    }
                    else
                    {
                        ;
                    }
                    this->zoneTable[zoneQueue[currentZonePos]][2]++;
                    this->zoneTable[currentZoneDest][2]++;
                    i++;
                }
                else
                {
                    continue;
                }
            }
        }
    }
    if (currentZonePos <= (zoneCountInput * zoneCountInput - 1))
    {
        for (i = 0; i <= (zoneCountInput * zoneCountInput - 1) && zoneMark[i] != 0; i++)
            ;
        zoneQueue[zoneQueueCount++] = i;
        goto addCrossEdgeFlag;
    }

    // cout << '\n'
    //      << '(' << currentZonePos << ',' << zoneQueueCount << ',' << edgeCount << ')' << '\n';

    this->crossEdges = new int *[edgeCount]; // 创建区域间连边列表
    for (i = 0; i <= edgeCount - 1; i++)
    { // 初始化区域间连边列表，0：列表的目标区域，1：连边的起始顶点，2：连边的终止顶点
        this->crossEdges[i] = new int[3];
    }
    for (i = 0; i <= (zoneCountInput * zoneCountInput - 1); i++)
    { // 累加各个区域的区域间连边数量得到区域间连边存储编号的索引值（参考桶排序原理）
        this->zoneTable[i + 1][2] += this->zoneTable[i][2];
    }
    for (i = edgeCount - 1; i >= 0; i--)
    { // 使用桶排序算法将各条区域间连边存储在区域间连边列表各个区域的对应位置
        this->crossEdges[--this->zoneTable[tempEdgeList[i][0]][2]][0] = tempEdgeList[i][1];
    }
    for (i = 0; i <= edgeCount - 1; i++)
    { // 复用区域间连边列表第一列存放每条连边终止区域的权值
        this->crossEdges[i][1] = this->zoneTable[this->crossEdges[i][0]][0];
    }
    for (i = 0; i <= (zoneCountInput * zoneCountInput - 1); i++)
    { // 在区域间连边列表中对每个区域到其它区域的连边按照终止区域的权值降序排序
        this->insertionSort(this->crossEdges, this->zoneTable[i][2], this->zoneTable[i + 1][2], 1);
    }
    for (i = 0; i <= (zoneCountInput * zoneCountInput - 1); i++)
    { // 根据各个终止区域的权值分配自己区域内相应权值排名的顶点作为区域间连边的起始顶点
        for (j = this->zoneTable[i][2]; j <= this->zoneTable[i + 1][2] - 1; j++)
        {
            this->crossEdges[j][1] = this->zoneTable[i][1] + j - this->zoneTable[i][2];
        }
    }
    for (i = 0; i <= (zoneCountInput * zoneCountInput - 1); i++)
    { // 根据上述区域间连边已经确定完毕的起始顶点确定各条区域间连边的终止顶点
        for (j = this->zoneTable[i][2]; j <= this->zoneTable[i + 1][2] - 1; j++)
        {
            for (currentZoneDest = this->zoneTable[this->crossEdges[j][0]][2];
                 (currentZoneDest <= this->zoneTable[this->crossEdges[j][0] + 1][2] - 1) && this->crossEdges[currentZoneDest][0] != i; currentZoneDest++)
                ; // 找到目标区域对应本区域的连边起始起点作为当前区域间连边的终止顶点
            this->crossEdges[j][2] = this->crossEdges[currentZoneDest][1];
        }
    }

    int *newEdgeList = NULL, *tempEdge = NULL; // 对于含有区域间连边的顶点的待替换的连边列表
    for (i = 0; i <= edgeCount - 1; i++)
    { // 将区域间连边添加进入地图的连边列表
        newEdgeList = new int[++this->vertexTable[this->crossEdges[i][1]][2]];
        for (j = 0; j <= this->vertexTable[this->crossEdges[i][1]][2] - 2; j++)
        { // 将顶点原先连边列表的内容复制到新列表中
            newEdgeList[j] = this->edgeTable[this->crossEdges[i][1]][j];
        }
        newEdgeList[this->vertexTable[this->crossEdges[i][1]][2] - 1] = this->crossEdges[i][2];
        tempEdge = this->edgeTable[this->crossEdges[i][1]];
        this->edgeTable[this->crossEdges[i][1]] = newEdgeList;
        delete[] tempEdge; // 删除释放原先连边列表对应顶点的旧行
        tempEdge = NULL;
        newEdgeList = NULL;
    }

    for (i = 0; i <= (zoneCountInput * zoneCountInput * 100 - 1); i++)
    {
        delete[] tempEdgeList[i];
        tempEdgeList[i] = NULL;
    }
    for (i = 0; i <= (zoneCountInput * zoneCountInput - 1); i++)
    {
        delete[] zoneHeap[i];
        zoneHeap[i] = NULL;
    }
    delete[] zoneMark;
    zoneMark = NULL;
    delete[] zoneQueue;
    zoneQueue = NULL;

    return 1;
}

int Map::findNearestVertex // 根据坐标查找最近的顶点的函数
    (const int &coordX, const int &coordY) const
{ // 参数coordX和coordY分别代表需要查询位置的坐标
    if (coordX <= (-1) || coordX >= 10000 || coordY <= (-1) || coordY >= 10000)
    {
        return (-1);
    }
    else
    {
        int i = 0, currentDistance = 0, nearestVertexPos = 0, minDistance = 200000000;
        for (i = 0; i <= this->vertexCount - 1; i++)
        {
            currentDistance = ((((this->vertexTable[i][0] / 10000) % 10000) - coordX) * (((this->vertexTable[i][0] / 10000) % 10000) - coordX) + ((this->vertexTable[i][0] % 10000) - coordY) * ((this->vertexTable[i][0] % 10000) - coordY));
            if (currentDistance <= minDistance - 1)
            {
                nearestVertexPos = i;
                minDistance = currentDistance;
                cout << currentDistance << endl;
            }
            else
            {
                continue;
            }
        }
        cout << coordX << '\t' << coordY << '\n'
             << ((this->vertexTable[nearestVertexPos][0] / 10000) % 10000) << '\t'
             << (this->vertexTable[nearestVertexPos][0] % 10000) << '\n'
             << nearestVertexPos << '\n';
        return nearestVertexPos;
    }
}

int Map::exportGraph // 地图写入文本文件执行函数
    (const char *fileName) const
{                                             // 参数fileName指示将要写入的文本文件名称
    ofstream storagefile(fileName, ios::out); // 以覆盖写的方式打开存储目标文本文件
    if (!storagefile.is_open())
    { // 保险机制，若目标文本文件打开失败则报错并直接退出
        cout << "Storage Fail: Error in Opening Destination File" << '\n';
        return 0;
    }
    else
    {
        int i = 0;
        storagefile << "----MapGraph----" << '\n';                       // 写入地图标题
        storagefile << this->zoneCount << '\t' << this->vertexCount << '\n'; // 写入地图的顶点数量数据
        for (i = 0; i <= (this->zoneCount * this->zoneCount); i++)
        { // 写入地图区域列表部分的数据
            storagefile << setw(4) << i
                        << setw(10) << this->zoneTable[i][0]
                        << setw(10) << this->zoneTable[i][1]
                        << setw(10) << this->zoneTable[i][2] << '\n';
        }
        storagefile << '\n';
        for (i = 0; i <= this->zoneTable[this->zoneCount * this->zoneCount][2] - 1; i++)
        { // 写入地图区域间连边列表部分的数据
            storagefile << setw(6) << i
                        << setw(8) << this->crossEdges[i][0]
                        << setw(8) << this->crossEdges[i][1]
                        << setw(8) << this->crossEdges[i][2] << '\n';
        }
        storagefile << '\n';
        for (i = 0; i <= this->vertexCount - 1; i++)
        { // 写入地图顶点列表部分的数据
            storagefile << setw(5) << i
                        << setw(9) << this->vertexTable[i][0]
                        << setw(7) << this->vertexTable[i][1]
                        << setw(3) << this->vertexTable[i][2];
            for (int j = 0; j <= this->vertexTable[i][2] - 1; j++)
            { // 写入地图全图连边列表部分的数据
                storagefile << setw(6) << this->edgeTable[i][j];
            }
            storagefile << '\n';
        }
        storagefile.close();
    }
    return 1;
}

int Map::displayGraphInfo // 注意这是在测试中用到的，方便调试
    (const int &outputMode) const
{
    switch (outputMode)
    {
    case 0:
    {
        cout << "------------------------------------------------" << '\n';
        for (int i = 0; i <= this->zoneCount * this->zoneCount - 1; i++)
        {
            cout << '(' << setw(4) << i
                 << setw(8) << this->zoneTable[i][0]
                 << setw(8) << this->zoneTable[i][1]
                 << setw(8) << this->zoneTable[i][2]
                 << ')' << '\n';
        }
        cout << '(' << setw(4) << this->zoneCount * this->zoneCount
             << setw(8) << this->zoneTable[this->zoneCount * this->zoneCount][0]
             << setw(8) << this->zoneTable[this->zoneCount * this->zoneCount][1]
             << setw(8) << this->zoneTable[this->zoneCount * this->zoneCount][2] << ')' << '\n';
        cout << "------------------------------------------------" << '\n'
             << '\n';
        break;
    }
    case 1:
    {
        cout << "################################################" << '\n';
        for (int i = 0; i <= 299; i++)
        {
            cout << '(' << setw(3) << i << ','
                 << setw(9) << this->vertexTable[i][0] << ','
                 << setw(8) << this->vertexTable[i][1] << ','
                 << setw(5) << this->vertexTable[i][2] << ','
                 << setw(5) << this->vertexTable[i][3] << ','
                 << setw(5) << this->vertexTable[i][4] << ')' << '\n';
        }
        cout << "################################################" << '\n'
             << '\n';
        break;
    }
    case 2:
    {
        cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << '\n';
        for (int i = 0; i <= 199; i++)
        {
            cout << '(' << setw(3) << i << ','
                 << setw(9) << this->vertexTable[i][0] << ','
                 << setw(8) << this->vertexTable[i][1] << ','
                 << setw(3) << this->vertexTable[i][2] << ','
                 << setw(7) << this->vertexTable[i][3] << ','
                 << setw(5) << this->vertexTable[i][4] << ')' << ':' << '\n';
            for (int j = 0; j <= this->vertexTable[i][2] - 1; j++)
            {
                cout << setw(5) << this->edgeTable[i][j];
            }
            cout << '\n'
                 << '\n';
        }
        cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << '\n';
        break;
    }
    case 3:
    {
        cout << "************************************************" << '\n';
        for (int i = 0; i <= this->zoneCount * this->zoneCount - 1; i++)
        {
            cout << "Zone " << i << ':' << '\n';
            for (int j = this->zoneTable[i][2]; j <= this->zoneTable[i + 1][2] - 1; j++)
            {
                cout << setw(3) << this->crossEdges[j][0]
                     << setw(6) << this->crossEdges[j][1]
                     << setw(6) << this->crossEdges[j][2] << '\n';
            }
            cout << '\n';
        }
        cout << "************************************************" << '\n';
        break;
    }
    case 4:
    {
        cout << "++++++++++++++++++++++++++++++++++++++++++++++++" << '\n';
        for (int i = 0; i <= 19; i++)
        {
            cout << '(' << setw(3) << i << ','
                 << setw(9) << this->vertexTable[i][0] << ','
                 << setw(8) << this->vertexTable[i][1] << ','
                 << setw(3) << this->vertexTable[i][2] << ','
                 << setw(7) << this->vertexTable[i][3] << ','
                 << setw(5) << this->vertexTable[i][4] << ')' << ':' << '\n';
            for (int j = 0; j <= this->vertexTable[i][2] - 1; j++)
            {
                double ec = (double)this->getEdgeCapacity(i, this->edgeTable[i][j]),
                    et = (double)this->getEdgeTraffic(i, this->edgeTable[i][j]);
                cout << '[' << setw(3) << i << ','
                     << setw(5) << this->edgeTable[i][j] << ']' << ':'
                     << setw(8) << this->vertexTable[i][1] << ','
                     << setw(8) << this->vertexTable[this->edgeTable[i][j]][1] << ':'
                     << setw(8) << this->getEdgeCapacity(i, this->edgeTable[i][j]) << ';'
                     << setw(8) << this->vertexTable[i][3] << ','
                     << setw(8) << this->vertexTable[this->edgeTable[i][j]][3] << ':'
                     << setw(8) << this->getEdgeTraffic(i, this->edgeTable[i][j]) << ';' << '('
                     << setw(10) << setprecision(6) << (et / ec) << ')' << ';'
                     << setw(9) << this->calculateDistance(i, this->edgeTable[i][j], 0) << ','
                     << setw(5) << this->calculateDistance(i, this->edgeTable[i][j], 1) << ','
                     << setw(10) << this->calculateDistance(i, this->edgeTable[i][j], 2) << ','
                     << setw(10) << this->calculateDistance(i, this->edgeTable[i][j], 3) << '\n';
            }
        }
        cout << "++++++++++++++++++++++++++++++++++++++++++++++++" << '\n';
        break;
    }
    default:
    {
        break;
    }
    }
    return 1;
}

int Map::searchVicinity // 查询与某顶点距离最近的100个顶点编号的函数
    (const int &vertexIndex)
{ // 参数vertexIndex指示需要查询的起始顶点
    if (vertexIndex <= (-1) || vertexIndex >= this->vertexCount)
    {
        return 0;
    } // 保险机制，若传入参数代表的顶点编号超限则报错并直接退出
    else
    {
        int i = 0, j = 0, targetVertex = 0, vertexHeapCurrentSize = this->vertexCount; // targetVertex代表目标顶点编号，vertexHeapCurrentSize代表顶点堆当前大小
        int **vertexDistanceHeap = new int *[this->vertexCount];            // 创建顶点堆数组
        for (i = 0; i <= this->vertexCount - 1; i++)
        {                           // 初始化顶点堆数组
            vertexDistanceHeap[i] = new int[2]; // 0：顶点编号，1：该顶点到起始顶点间距离
            vertexDistanceHeap[i][0] = i;
            vertexDistanceHeap[i][1] = this->calculateDistance(vertexIndex, i, 0);
            this->vertexTable[i][4] %= 10;
            // cout << vertexTable[i][4] << endl;
        } // 重置顶点列表中的标记位，防止标记冲突
        this->buildHeap(vertexDistanceHeap, vertexHeapCurrentSize, 1); // 建立顶点堆
        for (i = 0; i <= 99; i++)
        { // 取出堆顶元素100次
            targetVertex = this->deleteMinFromHeap(vertexDistanceHeap, vertexHeapCurrentSize, 1, 0);
            this->vertexTable[targetVertex][4] += 20; // 在顶点列表的标记位列特殊标记这100个顶点（占用标记位的十位数）
            for (j = 0; j <= this->vertexTable[targetVertex][2] - 1; j++)
            {
                if (this->vertexTable[this->edgeTable[targetVertex][j]][4] <= 9)
                {
                    this->vertexTable[this->edgeTable[targetVertex][j]][4] += 10;
                }

                cout << '(' << setw(6) << vertexIndex
                     << setw(6) << targetVertex
                     << setw(8) << this->calculateDistance(vertexIndex, targetVertex, 1)
                     << setw(5) << this->vertexTable[targetVertex][4]
                     << setw(6) << this->edgeTable[targetVertex][j]
                     << setw(5) << this->vertexTable[this->edgeTable[targetVertex][j]][4] << ')' << '\n';
            }
        }
        cout << '\n';

        for (i = 0; i <= this->vertexCount - 1; i++)
        { // 释放函数内部临时动态申请的内存空间，以下各函数结尾部分同此
            delete[] vertexDistanceHeap[i];
            vertexDistanceHeap[i] = NULL;
        }
        delete[] vertexDistanceHeap;
        vertexDistanceHeap = NULL;
    }
    return 1;
}

// 启发式函数，返回从当前顶点到目标顶点的估算距离
int GetHeuristic(int currentVertexPos, int targetVertexPos)
{
    // 这里假设是欧几里得距离作为启发式估计
    // 具体实现需要根据图的结构和坐标来定制
    return abs(currentVertexPos - targetVertexPos); // 简单示例，替换为实际的估算方法
}

int Map::findAStarPath(const int &startVertex, const int &endVertex)
{
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> heap;

    double *distance = new double[vertexCount + 10];
    for (int i = 0; i < vertexCount; i++)
        distance[i] = 999999;
    distance[startVertex] = 0;

    int *predecessorVertex = new int[vertexCount + 10];
    for (int i = 0; i < vertexCount; i++)
        predecessorVertex[i] = -1;

    // 启发函数 h(n): v -> double
    auto heuristic = [&](int v) -> double
    {
        double dx = vertexTable[v][0] - vertexTable[endVertex][0];
        double dy = vertexTable[v][1] - vertexTable[endVertex][1];
        return sqrt(dx * dx + dy * dy); // 欧几里得距离
    };

    heap.push({distance[startVertex] + heuristic(startVertex), startVertex}); // f(startVertex) = g(startVertex) + h(startVertex)

    while (!heap.empty())
    {
        int currentNum = heap.top().second;
        double currentDis = heap.top().first;
        heap.pop();

        if (currentNum == endVertex)
            break;
        if (currentDis > distance[currentNum] + heuristic(currentNum))
            continue;

        for (int i = 0; i < vertexTable[currentNum][2]; i++)
        {
            int v = edgeTable[currentNum][i];
            int cost = calculateDistance(currentNum, v, 1);

            if (distance[currentNum] + cost < distance[v])
            {
                distance[v] = distance[currentNum] + cost;
                predecessorVertex[v] = currentNum;
                heap.push({distance[v] + heuristic(v), v});
            }
        }
    }

    vector<int> tempPath;
    path.clear();
    int k = endVertex;
    while (predecessorVertex[k] != -1)
    {
        tempPath.push_back(k);
        k = predecessorVertex[k];
    }
    int len = tempPath.size();
    for (int i = len - 1; i >= 0; i--)
    {
        path.push_back(tempPath[i]);
    }

    delete[] distance;
    delete[] predecessorVertex;
    return 1;
}

// int Map::Dijkstra(const int& vStart,const int& vEnd)
// {
//     //cout<<"dij\n";
//     priority_queue<pair<double,int>,vector<pair<double,int>>,greater<pair<double,int>>> heap;
//     //cout<<"heap\n";
//     double* dist=new double[vertexCount+10];
//     for(int i=0;i<vertexCount;i++)dist[i]=999999;
//     dist[vStart]=0;
//     int* pre_vtx=new int[vertexCount+10];
//     for(int i=0;i<vertexCount;i++)pre_vtx[i]=-1;
//     //cout<<"array\n";
//     heap.push({0,vStart});
//     while(!heap.empty()){
//         auto num=heap.top().second;
//         auto dis=heap.top().first;
//         heap.pop();
//         if(num==vEnd) break;
//         if (dis > dist[num])continue;
//         for (int i=0;i<vertexTable[num][2];i++) {
//             int v = edgeTable[num][i];
//             int cost = calculateDistance(num,v,1);

//             if (dist[num] + cost < dist[v]) {
//                 dist[v] = dist[num] + cost;
//                 pre_vtx[v] = num;
//                 heap.push({dist[v], v});
//             }
//         }
//     }

//     vector<int> path_;
//     _path.clear();
//     int k=vEnd;
//     while(pre_vtx[k]!=-1){
//         path_.push_back(k);
//         k=pre_vtx[k];
//     }
//     int len=path_.size();
//     for(int i=len-1;i>=0;i--){
//         _path.push_back(path_[i]);
//     }

//     delete[] dist;
//     delete[] pre_vtx;
//     return 1;
// }

int Map::findDijkstraPath // Dijkstra算法最短路径查找函数
    (const int &startVertex, const int &searchMode)
{ // 参数startVertex指示最短路径的起始顶点，searchMode指示查找类别（按照路径距离或者路径间通行时间）
    if (startVertex <= (-1) || startVertex >= this->vertexCount)
    {
        return 0;
    } // 保险机制，若传入参数代表的顶点编号超限则报错并直接退出
    else
    {
        int i = 0, j = 0, pathQueueCount = 0, currentVertexPos = (-1); // pathQueueCount指示最短路径堆当前大小，currentVertexPos指示当前正在处理的顶点
        int **pathQueue = new int *[this->vertexCount];      // 创建最短路径堆数组
        int *currentNext = new int[2];                      // currentNext代表即将插入最短路径堆的元素([0]:顶点 [1]:距离)
        for (i = 0; i <= this->vertexCount - 1; i++)
        {                                // 初始化最短路径列表
            this->pathTable[i][0] = (-1); // 0：该顶点到起始顶点当前的最短距离，初始化为无穷大（-1）
            this->pathTable[i][1] = (-1); // 1：该顶点到起始顶点当前的前驱顶点，初始化为不存在（-1）
            this->pathTable[i][2] = 0;    // 2：该顶点查找过程中的访问标记（可复用为显示标记）
        }
        for (i = 0; i <= (this->vertexCount - 1); i++)
        {                            // 初始化最短路径堆数组
            pathQueue[i] = new int[2]; // 0：顶点编号，1：该顶点到起始顶点当前的最短距离
            pathQueue[i][0] = -1;
            pathQueue[i][1] = -1;
        }
        currentNext[0] = startVertex;                                // 设置第一个插入最短路径堆的元素（起始顶点）
        currentNext[1] = 0;                                     // 设置起始顶点到自己的距离为0
        this->insertElementToHeap(pathQueue, pathQueueCount, currentNext, 2, 1); // 将起始顶点插入最短路径堆
        this->pathTable[startVertex][0] = 0;                       // 在最短路径列表中设置起始顶点到自己的距离为0
        this->pathTable[startVertex][1] = startVertex;                  ////在最短路径列表中设置起始顶点的前驱顶点为自己
        currentVertexPos = this->deleteMinFromHeap(pathQueue, pathQueueCount, 1, 0); // 从最短路径堆中取出起始节点

        for (i = 0; i <= this->vertexCount - 1; i++)
        { // 遍历全图（连通情况下）的所有顶点
            while (this->pathTable[currentVertexPos][2] != 0 && pathQueueCount >= 1)
            { // 最短路径堆不为空时循环，直至取出的顶点尚未访问（访问标记为0）时停止
                currentVertexPos = this->deleteMinFromHeap(pathQueue, pathQueueCount, 1, 0);
            } // 取出当前尚未处理的距离起始顶点最近的顶点
            this->pathTable[currentVertexPos][2] = 1; // 成功取出尚未访问，且距离起始顶点最近的顶点，将其标记为已访问
            if (this->pathTable[currentVertexPos][0] == (-1))
            {
                break;
            } // 若该正待处理的顶点也无法到达，则退出循环
            else
            {
                for (j = 0; j <= this->vertexTable[currentVertexPos][2] - 1; j++)
                { // 遍历当前处理顶点的连边列表（邻接表）
                    if (this->pathTable[this->edgeTable[currentVertexPos][j]][0] == (-1) && this->pathTable[currentVertexPos][0] >= 0)
                    {                                                                                                                                                   // 若该连边终点当前不可达（则从当前处理顶点到该终点的连边一定是该终点更短路径的一部分）
                        this->pathTable[this->edgeTable[currentVertexPos][j]][0] = this->pathTable[currentVertexPos][0] + this->calculateDistance(currentVertexPos, this->edgeTable[currentVertexPos][j], searchMode); // 按照Dijkstra算法更新该连边终点到起始顶点当前的最短距离
                        currentNext[0] = this->edgeTable[currentVertexPos][j];                                                                                                      // 准备将更新后的顶点（该连边终点）信息插入最短路径堆
                        currentNext[1] = this->pathTable[this->edgeTable[currentVertexPos][j]][0];
                        this->pathTable[this->edgeTable[currentVertexPos][j]][1] = currentVertexPos; // 更新该连边终点到起始顶点当前的前驱顶点为当前处理的顶点
                        this->insertElementToHeap(pathQueue, pathQueueCount, currentNext, 2, 1);
                    } // 更新后的顶点（该连边终点）信息插入最短路径堆
                    else if (this->pathTable[this->edgeTable[currentVertexPos][j]][0] >
                             this->pathTable[currentVertexPos][0] + this->calculateDistance(currentVertexPos, this->edgeTable[currentVertexPos][j], searchMode))
                    {

                        int neighbor = this->edgeTable[currentVertexPos][j];
                        int newDist = this->pathTable[currentVertexPos][0] + this->calculateDistance(currentVertexPos, neighbor, searchMode);

                        // 更新最短距离和前驱
                        this->pathTable[neighbor][0] = newDist;
                        this->pathTable[neighbor][1] = currentVertexPos;

                        // 检查是否已在堆中
                        int pos = this->searchInHeap(pathQueue, pathQueueCount, neighbor);
                        if (pos != -1)
                        {
                            // 存在则先删除
                            deleteFromHeap(pathQueue, pathQueueCount, pos, 1);
                        }
                        // 重新插入堆
                        currentNext[0] = neighbor;
                        currentNext[1] = newDist;
                        this->insertElementToHeap(pathQueue, pathQueueCount, currentNext, 2, 1);
                    }
                    else
                    {
                        continue;
                    }
                }
            }
        } // 除上述两种情况外对于当前处理顶点和其某条连边的终点无需更新最短路径列表中的信息，继续循环

        for (j = 0; j <= this->vertexCount - 1; j++)
        { // 检查图中是否仍有不可达的顶点
            if (this->pathTable[j][0] < 0)
            {
                cout << j << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << '\n';
            }
            else
            {
                continue;
            }
        }
        cout << i << '\n';
        for (i = 0; i <= this->vertexCount - 1; i++)
        {
            for (j = 0, currentVertexPos = i;
                 currentVertexPos != this->pathTable[currentVertexPos][1];
                 currentVertexPos = this->pathTable[currentVertexPos][1], j++)
                ;
            this->pathTable[i][2] = j;
        }
        j = this->vertexCount / 2;
        for (i = this->vertexCount - 2000 - j; i <= this->vertexCount - 1 - j; i++)
        {
            cout << setw(6) << i
                 << setw(10) << this->pathTable[i][0]
                 << setw(6) << this->pathTable[i][1]
                 << setw(4) << this->pathTable[i][2]
                 << '\n';
        }
        cout << '\n';

        for (i = 0; i <= this->vertexCount - 1; i++)
        { // 将使用过的路径列表的标记位清零复位
            this->pathTable[i][2] = 0;
        }
        for (i = 0; i <= (this->vertexCount - 1); i++)
        {
            delete[] pathQueue[i];
            pathQueue[i] = NULL;
        }
        delete[] pathQueue;
        pathQueue = NULL;
        delete[] currentNext;
        currentNext = NULL;
    }
    return 1;
}

int Map::markShortestPath // 最短路径标记函数
    (const int &startVertex, const int &destinationVertex)
{ // 参数startVertex和destinationVertex分别标记所需要查询路径的起始顶点和终止顶点编号
    if (startVertex <= (-1) || startVertex >= this->vertexCount || destinationVertex <= (-1) || destinationVertex >= this->vertexCount)
    {
        return (-1);
    } // 保险机制，若输入的顶点编号参数超限则报错并直接退出
    else if (this->pathTable[startVertex][0] != 0)
    {
        return (-1);
    } // 若输入的顶点编号参数与当前路径列表中存储的数据不匹配，则也报错并直接退出
    else
    {
        int i = 0, currentVertexPos = destinationVertex; // currentVertexPos用于追踪当前顶点的位置
        for (i = 0; i <= this->vertexCount - 1; i++)
        { // 刷新重置路径列表的标记位，避免标记冲突
            this->pathTable[i][2] = 0;
        }
        for (i = 1; currentVertexPos != startVertex && i <= this->vertexCount; i++)
        { // 倒序标记路径上的各个顶点，便于显示时分辨这些顶点以及其之间的连边
            this->pathTable[currentVertexPos][2] = i;
            currentVertexPos = this->pathTable[currentVertexPos][1];
        }
        this->pathTable[startVertex][2] = i;
        return this->pathTable[destinationVertex][0];
    }
}

int Map::refreshAllTraffic // 全图交通流量刷新函数
    (const int &refreshMode)
{ // 参数refreshMode指示刷新交通流量的策略（完全刷新或部分刷新）
    if (refreshMode <= 0)
    {                                    // 第一种情况（完全刷新交通流量）
        srand((unsigned int)time(NULL)); // 设置（伪）随机数种子
        for (int i = 0; i <= this->vertexCount - 1; i++)
        { // 遍历顶点列表，将每个顶点的交通流量权值重置为一个随机数
            this->vertexTable[i][3] = ((rand() * rand()) % (int)(1.6 * this->vertexTable[i][1]) + 1);
        }
    }
    else
    { // 第二种情况（部分刷新交通流量）
        for (int i = 0; i <= this->vertexCount - 1; i++)
        { // 遍历顶点列表，每个顶点当前的交通流量权值与一个随机数取加权平均值作为其新的交通流量权值
            this->vertexTable[i][3] *= refreshMode;
            this->vertexTable[i][3] += ((rand() * rand()) % (int)(1.6 * this->vertexTable[i][1]) + 1);
            this->vertexTable[i][3] /= (refreshMode + 1);
        }
    }
    return 1;
}

int Map::refreshDisplayPriority // 显示优先级设置函数
    (const int &refreshMode)
{ // 参数refreshMode指示需要更改标记位的模式（设置或者刷新重置）
    switch (refreshMode)
    { // refreshMode为0表示全新设置标记位，为1表示刷新重置标记位
    case 0:
    {
        // 初始化所有顶点标记为1
        for (int i = 0; i < this->vertexCount; i++)
        {
            this->vertexTable[i][4] = 1;
            this->pathTable[i][2] = 0;
        }

        // 标记跨区域连边的顶点为4（直接遍历crossEdges）
        for (int e = 0; e < this->zoneTable[this->zoneCount * this->zoneCount][2]; e++)
        {
            int v1 = this->crossEdges[e][1]; // 起始顶点
            int v2 = this->crossEdges[e][2]; // 终止顶点
            this->vertexTable[v1][4] = 4;
            this->vertexTable[v2][4] = 4;
        }

        // 标记邻居顶点为3或2
        for (int i = 0; i < this->vertexCount; i++)
        {
            if (this->vertexTable[i][4] == 4)
            {
                for (int j = 0; j < this->vertexTable[i][2]; j++)
                {
                    int neighbor = this->edgeTable[i][j];
                    if (this->vertexTable[neighbor][4] < 3)
                    {
                        this->vertexTable[neighbor][4] = 3;
                        // 标记二级邻居为2
                        for (int k = 0; k < this->vertexTable[neighbor][2]; k++)
                        {
                            int neighbor2 = this->edgeTable[neighbor][k];
                            if (this->vertexTable[neighbor2][4] < 2)
                            {
                                this->vertexTable[neighbor2][4] = 2;
                            }
                        }
                    }
                }
            }
        }
        break;
    }
    case 1:
    {
        for (int i = 0; i <= this->vertexCount - 1; i++)
        { // 有需要时对顶点列表中的标记位作完全重置处理，恢复其默认的标记位取值
            this->vertexTable[i][4] %= 10;
        }
        break;
    }
    case 2:
    {
        for (int i = 0; i <= this->vertexCount - 1; i++)
        { // 有需要时对路径列表作完全重置处理，恢复其默认的标记位取值
            this->pathTable[i][0] = (-1);
            this->pathTable[i][1] = (-1);
            this->pathTable[i][2] = 0;
        }
        break;
    }
    default:
    {
        break;
    }
    }
    return 1;
}

int Map::isVertexVisible // 顶点显示可见性扫描确认函数
    (const int &vertexToCheck, const int &displayMode) const
{ // 参数vertexToCheck代表待检查的顶点编号，displayMode指示当前需要显示的优先级（缩放比例）
    if (vertexToCheck < 0 || vertexToCheck >= this->vertexCount || displayMode <= (-1) || displayMode >= 4)
    {
        return 0;
    } // 若传入的顶点编号参数或者模式指示参数的取值超限则报错并直接退出
    else if (this->pathTable[vertexToCheck][2] != 0)
    {
        return 7;
    } // 待检查的顶点位于当前显示的最短路径上
    else if (this->vertexTable[vertexToCheck][4] >= 20)
    {
        return 6;
    } // 待检查的顶点属于到当前顶点距离最近的100个顶点之一
    else if (this->vertexTable[vertexToCheck][4] >= 10)
    {
        return 5;
    } // 待检查的顶点不属于到当前顶点距离最近的100个顶点之一，但与其中之一有连边相连接
    else
    {
        return (this->vertexTable[vertexToCheck][4] <= displayMode ? // 待检查的顶点若对于给定的优先级不需要显示则返回0，否则返回其优先级标记的正相对值
                    0
                                                                   : (this->vertexTable[vertexToCheck][4] - displayMode));
    }
}

int Map::isEdgeVisible // 连边显示可见性扫描确认函数
    (const int &vertexA, const int &vertexB, const int &displayMode) const
{ // 参数vertexA和vertexB分别代表待检查的连边对应的两个端点的编号，displayMode指示当前需要显示的优先级（缩放比例）
    if (vertexA < 0 || vertexA >= this->vertexCount || vertexB < 0 || vertexB >= this->vertexCount || displayMode <= (-1) || displayMode >= 4)
    {
        return 0;
    } // 若传入的顶点编号参数或者模式指示参数的取值超限则报错并直接退出
    else if (this->pathTable[vertexA][2] != 0 && this->pathTable[vertexB][2] != 0 && (this->pathTable[vertexA][2] - this->pathTable[vertexB][2] == 1 || this->pathTable[vertexB][2] - this->pathTable[vertexA][2] == 1))
    {
        return 7;
    } // 待检查的连边位于当前显示的最短路径上
    else if (this->vertexTable[vertexA][4] >= 20 && this->vertexTable[vertexB][4] >= 20)
    {
        return 6;
    } // 待检查的连边连接到当前顶点距离最近的100个顶点中的其中两个
    else if (this->vertexTable[vertexA][4] >= 10 && this->vertexTable[vertexB][4] >= 10)
    {
        return 5;
    } // 待检查的连边连接到当前顶点距离最近的100个顶点中的其中一个
    else if (this->vertexTable[vertexA][4] <= displayMode || this->vertexTable[vertexB][4] <= displayMode)
    {
        return 0;
    } // 待检查的顶点若对于给定的优先级不需要显示则返回0
    else
    {
        return (this->vertexTable[vertexA][4] <= this->vertexTable[vertexB][4] ? (this->vertexTable[vertexA][4] - displayMode) : (this->vertexTable[vertexB][4] - displayMode));
    }
} // 待检查的顶点若对于给定的优先级需要显示则返回其两个端点优先级标记中较小的正相对值

int Map::insertionSort // 插入排序辅助函数（默认降序排列）
    (int **&listArray, const int &leftEdge, const int &rightEdge, const int &sortMode)
{ // 参数listArray为待排序数组，leftEdge和rightEdge分别代表排序的左右边界（左闭右开），sortMode为排序依据选择参数，详见下
    for (int i = leftEdge; i <= rightEdge - 1; i++)
    {
        for (int j = i; j >= leftEdge + 1; j--)
        {
            if (listArray[j][sortMode] >= listArray[j - 1][sortMode] + 1)
            { // 根据参数sortMode的取值确定数据比较的依据，下同
                int *temp = listArray[j];
                listArray[j] = listArray[j - 1];
                listArray[j - 1] = temp;
                continue;
            }
            else
            {
                break;
            }
        }
    }
    return 1;
}

int Map::quickSort // 快速排序辅助函数（默认降序排列）
    (int **&listArray, const int &leftEdge, const int &rightEdge, const int &sortMode)
{ // 参数意义同上述插入排序辅助函数
    if (rightEdge - leftEdge <= 9)
    {
        return 1;
    } // 为性能优化考虑，子区域太小时不予排序，最后使用插入排序收尾
    else
    {
        int pivot = (leftEdge + rightEdge) / 2, newLeft = rightEdge, newRight = leftEdge;
        int *temp = listArray[pivot];
        listArray[pivot] = listArray[rightEdge - 1];
        listArray[rightEdge - 1] = temp;
        for (; newRight <= newLeft - 1;)
        {
            for (; listArray[newRight][sortMode] >= listArray[rightEdge - 1][sortMode] + 1; newRight++)
                ;
            for (; newLeft >= newRight + 1 && listArray[newLeft - 1][sortMode] <= listArray[rightEdge - 1][sortMode]; newLeft--)
                ;
            if (newRight <= newLeft - 1)
            {
                temp = listArray[newRight];
                listArray[newRight] = listArray[newLeft - 1];
                listArray[newLeft - 1] = temp;
                continue;
            }
            else
            {
                break;
            }
        }
        if (newLeft == newRight)
        {
            temp = listArray[newLeft];
            listArray[newLeft] = listArray[rightEdge - 1];
            listArray[rightEdge - 1] = temp;
            quickSort(listArray, leftEdge, newRight, sortMode);
            quickSort(listArray, newLeft + 1, rightEdge, sortMode);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

// 在堆中查找顶点vertexId的位置，返回下标（未找到返回-1）
int Map::searchInHeap(int **&heap, int heapSize, int vertexId)
{
    for (int i = 0; i < heapSize; i++)
    {
        if (heap[i][0] == vertexId)
            return i;
    }
    return -1;
} // 新添加

// 删除堆中指定位置的元素，并维护堆性质
void Map::deleteFromHeap(int **heap, int &heapSize, int position, const int &sortMode)
{
    if (position < 0 || position >= heapSize)
        return;

    // 用最后一个元素覆盖待删除元素
    heap[position][0] = heap[heapSize - 1][0];
    heap[position][1] = heap[heapSize - 1][1];
    heapSize--;

    // 从position位置下沉调整堆
    siftDown(heap, position, heapSize, sortMode);
}

int Map::siftDown // 堆排序下筛辅助函数（默认小顶堆）
    (int **&listArray, int currentPos, const int &arraySize, const int &sortMode)
{ // 参数listArray代表堆数组，currentPos指示当前下筛的位置，arraySize指示堆的大小（不可修改），sortMode意义同上述排序函数
    for (; currentPos <= (arraySize / 2 - 1) && currentPos >= 0;)
    {
        if ((2 * currentPos + 2) == arraySize)
        {
            if (listArray[currentPos][sortMode] > listArray[2 * currentPos + 1][sortMode])
            {
                int *temp = listArray[currentPos];
                listArray[currentPos] = listArray[2 * currentPos + 1];
                listArray[2 * currentPos + 1] = temp;
            }
            else
            {
                ;
            }
            break;
        }
        else
        {
            int minChild = (listArray[2 * currentPos + 1][sortMode] <= listArray[2 * currentPos + 2][sortMode] ? (2 * currentPos + 1) : (2 * currentPos + 2));
            if (listArray[currentPos][sortMode] > listArray[minChild][sortMode])
            {
                int *temp = listArray[currentPos];
                listArray[currentPos] = listArray[minChild];
                listArray[minChild] = temp;
                currentPos = minChild;
                continue;
            }
            else
            {
                break;
            }
        }
    }
    return 1;
}

int Map::buildHeap // 堆创建辅助函数（默认小顶堆）
    (int **&listArray, const int &arraySize, const int &sortMode)
{ // 各同名参数意义同上述下筛辅助函数
    for (int pos = arraySize / 2 - 1; pos >= 0; pos--)
    {
        this->siftDown(listArray, pos, arraySize, sortMode);
    }
    return 1;
}

int Map::deleteMinFromHeap // 堆顶元素删除辅助函数（默认小顶堆）
    (int **&listArray, int &heapSize, const int &sortMode, const int &returnType)
{ // 各同名参数意义同上述下筛辅助函数，参数heapSize指示堆的大小（在函数中修改），returnType指示需要传回堆顶元素信息的类别
    if (heapSize <= 0)
    {
        return -1;
    }
    else
    {
        int returnValue = listArray[0][returnType];
        int *temp = listArray[0];
        listArray[0] = listArray[heapSize - 1];
        listArray[heapSize - 1] = temp;
        heapSize--;
        this->siftDown(listArray, 0, heapSize, sortMode);
        return returnValue;
    }
}

int Map::insertElementToHeap // 堆元素插入辅助函数（默认小顶堆）
    (int **&listArray, int &heapSize, const int *newElement, const int &elementSize, const int &sortMode)
{ // 各同名参数意义同上述下筛辅助函数，参数newElement和elementSize分别指示将要插入堆的新元素和其分量个数（不可修改，函数中传值复制处理）
    int i = 0, pos = heapSize;
    for (i = 0; i <= elementSize - 1; i++)
    {
        listArray[heapSize][i] = newElement[i];
    }
    heapSize++;
    for (; pos >= 0 && listArray[pos][sortMode] <= listArray[(pos - 1) / 2][sortMode] - 1; pos = (pos - 1) / 2)
    {
        int *temp = listArray[pos];
        listArray[pos] = listArray[(pos - 1) / 2];
        listArray[(pos - 1) / 2] = temp;
    }
    return 1;
}

int Map::getEdgeCapacity // 获取连边最大流量的辅助函数
    (const int &vertexA, const int &vertexB) const
{ // 参数vertexA，vertexB分别指示连边的起始顶点和终止顶点（双向均可）的编号，下同
    long long int weightA = (long long int)(this->vertexTable[vertexA][1]),
        weightB = (long long int)(this->vertexTable[vertexB][1]);
    return (int)((2 * weightA * weightB) / (weightA + weightB));
} // 用二顶点权值的调和平均数作为连边最大流量的模拟值

int Map::getEdgeTraffic // 获取连边当前流量的辅助函数
    (const int &vertexA, const int &vertexB) const
{ // 参数意义同上述获取连边最大流量的辅助函数
    long long int randomA = (long long int)(this->vertexTable[vertexA][3]),
        randomB = (long long int)(this->vertexTable[vertexB][3]),
        weightA = (long long int)(this->vertexTable[vertexA][1]),
        weightB = (long long int)(this->vertexTable[vertexB][1]);
    return (int)((((randomA * weightA + randomB * weightB) / (weightA + weightB)) * randomB) / randomA);
} // 用二顶点随机数权值相对于其权值的加权平均数作为连边当前流量的模拟值

int Map::calculateDistance // 获取二顶点间距离的辅助函数
    (const int &vertexA, const int &vertexB, const int &distanceMode) const
{                                                                                                                                                                                                                                                                                                                                                                  // 参数vertexA、vertexB意义同上述获取连边最大流量的辅助函数，distanceMode指示所需距离的种类（简单距离或通行时间）
    int dSquared = ((((this->vertexTable[vertexA][0] / 10000) % 10000) - ((this->vertexTable[vertexB][0] / 10000) % 10000)) * (((this->vertexTable[vertexA][0] / 10000) % 10000) - ((this->vertexTable[vertexB][0] / 10000) % 10000)) + ((this->vertexTable[vertexA][0] % 10000) - (this->vertexTable[vertexB][0] % 10000)) * ((this->vertexTable[vertexA][0] % 10000) - (this->vertexTable[vertexB][0] % 10000))); // dSquared代表两顶点间坐标距离的平方值(物理距离，这里没有开方)
    switch (distanceMode)
    {
    case 0:
    {
        return dSquared;
        break;
    } // 最简单情况下直接返回dSquared的值（用于单纯依连边距离排序时）
    case 1:
    {
        return (int)(sqrt((double)(dSquared)));
        break;
    } // 对dSquared开平方，返回取整的距离值（对单纯的最短路径搜索有用）
    case 2:
    {
        double currentTraffic = (double)(this->getEdgeTraffic(vertexA, vertexB));
        double maxTraffic = (double)(this->getEdgeCapacity(vertexA, vertexB)); // 分别获取连边当前的和最大的交通流量
        if (currentTraffic <= maxTraffic)
        {
            return (int)(sqrt((double)(dSquared)));
        } // （分段函数）若当前交通流量未达到最大容量，则认为通行速度为常数1，仍然返回取整的距离值
        else
        {
            double passTime = exp((currentTraffic / maxTraffic) - 1); // 若当前交通流量超过最大容量则使用指数函数计算通行时间
            if (passTime > 100.0)
            {
                passTime = 100.0;
            }
            else
            {
                ;
            } // 对通行时间的取值规定上界，防止数据溢出
            passTime *= sqrt((double)(dSquared)); // 连边间距离仍然对通行时间有影响
            return (int)(passTime);
        }
        break;
    }
    case 3:
    {
        double currentTraffic = (double)(this->getEdgeTraffic(vertexA, vertexB));
        double maxTraffic = (double)(this->getEdgeCapacity(vertexA, vertexB)); // 分别获取连边当前的和最大的交通流量
        return (int)((currentTraffic / (maxTraffic + 1)) * 100000);
        break;
    }
    default:
    {
        return dSquared;
        break;
    }
    }
}

int Map::checkEdgeIntersection // 用于连边交叉检查的辅助函数
    (int **&edgeListInput, const int &edgeCountInput, const int &fromVertex, const int &toVertex) const
{ // 参数edgeListInput代表待查询的（临时）连边列表，edgeCountInput指示其大小，fromVertex和toVertex分别指示待检查连边起始顶点和终止顶点的编号
    int i = 0, ax = 0, ay = 0, bx = 0, by = 0, ab1 = 0, ab2 = 0, cd1 = 0, cd2 = 0,
        cx = (this->vertexTable[fromVertex][0] / 10000) % 10000,
        cy = (this->vertexTable[fromVertex][0] % 10000),
        dx = (this->vertexTable[toVertex][0] / 10000) % 10000,
        dy = (this->vertexTable[toVertex][0] % 10000); // 计算得到顶点fromVertex（c）和toVertex（d）的横纵坐标
    for (i = 0; i <= edgeCountInput - 1; i++)
    {
        ax = (this->vertexTable[edgeListInput[i][0]][0] / 10000) % 10000; // 遍历临时连边列表，计算得到每条列表两个顶点（a和b）的横纵坐标
        ay = (this->vertexTable[edgeListInput[i][0]][0] % 10000);
        bx = (this->vertexTable[edgeListInput[i][1]][0] / 10000) % 10000;
        by = (this->vertexTable[edgeListInput[i][1]][0] % 10000);
        ab1 = (by - ay) * cx - (bx - ax) * cy + bx * ay - ax * by; // 分别将c，d两点坐标代入直线ab表达式，若所得二值异号说明c，d两点分别位于直线ab的两侧
        ab2 = (by - ay) * dx - (bx - ax) * dy + bx * ay - ax * by;
        cd1 = (dy - cy) * ax - (dx - cx) * ay + dx * cy - cx * dy; // 同理判定a，b两点是否位于直线cd两侧，若都是则认为线段ab和线段cd发生交叉（不允许出现）
        cd2 = (dy - cy) * bx - (dx - cx) * by + dx * cy - cx * dy;
        if (ax == cx && ay == cy && bx == dx && by == dy)
        {
            return 1;
            break;
        } // 若连边发生重复则认为其交叉（不允许出现），返回真
        else if (ax == dx && ay == dy && bx == cx && by == cy)
        {
            return 1;
            break;
        } // 若连边发生重复则认为其交叉（不允许出现），返回真
        else if ((ab1 * ab2) < 0 && (cd1 * cd2) < 0)
        {
            return 1;
            break;
        } // 按照上述判定，若连边发生交叉则返回真
        else
        {
            continue;
        }
    }
    return 0;
} // 遍历整个临时连边列表仍然未发现重复连边或者交叉现象，判定（fromVertex，toVertex）连边合法，返回假

// 获取MapGraph中每个顶点的信息:xy，权值，边的数目，车流模拟，访问记录，边的记录,最短路径记录
int Map::getVertexX(int vertexIndex) const
{
    return this->vertexTable[vertexIndex][0] / 10000;
}
int Map::getVertexY(int vertexIndex) const
{
    return this->vertexTable[vertexIndex][0] % 10000;
}
int Map::getVertexWeight(int vertexIndex) const
{
    return this->vertexTable[vertexIndex][1];
}
int Map::getEdgeCount(int vertexIndex) const
{
    return this->vertexTable[vertexIndex][2];
}
int Map::getEdgeTrafficFlow(int vertexIndex) const
{
    return this->vertexTable[vertexIndex][3];
}
int Map::getVertexMark(int vertexIndex) const
{
    return this->vertexTable[vertexIndex][4];
}
int *Map::getAdjacentVertices(int vertexIndex) const
{
    return this->edgeTable[vertexIndex];
}
int Map::getPathNode(int pathIndex, int nodeIndex) const
{
    return this->pathTable[pathIndex][nodeIndex];
}
int Map::getVertexCount() const
{
    return this->vertexCount;
}
