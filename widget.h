#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "Qhead.h"
#include "graphicdraw.h"
#include "vertexdraw.h"
#include "pathdraw.h"



QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    void initGraphicsView();//初始化view

    void showShortestPath(vector<int>);//展示最短路径
    void showFastestPath(vector<int>);//展示最快路径
    void showNearestVertexPath(vector<int>,int);//展示最近100个点
    void refreshPath();//用于清除图上的路径
    void pathFlagIgnore();//给缩略图中显示的路径标上记号

    void geneSceneForAll();//导入数据，创建可交互地图
    void geneSceneForIgnore();//给点设置缩放层级

    int getCurrentLevel(){return zoomLevel;}//获取地图当前缩放层级
    void setSceneIgnoreLevel(int k){zoomLevel=k;}//设置地图的缩放层级

    void zoomLevelChange(int,int,int);//改变缩放层级时，刷新地图上的所有点
    void resetQpenWidth(int);//重设所有笔的宽度
    void setPathIcon();//设置路径两个端点的坐标

    void startTrafficUpdates();
    void stopTrafficUpdates();


public slots:
    void theSlotMouseEvent(int eventId, QMouseEvent *event);//接受鼠标信号，操作地图的槽函数
    void theSlotWheelEvent(QWheelEvent *event);//接受滚轮信号的槽函数
    void theSlotForPoint(int ID,QGraphicsSceneMouseEvent* event,int i,int x,int y);//接受鼠标信号，与点交互的槽函数

    void locateVertex();//定位按钮的槽函数
    void exportGraph(void)const;//导出地图的槽函数
    void importGraph();//读取地图的槽函数
    void searchShortestPath();//搜索最短路径的槽函数
    void searchFastestPath();//搜索最快路径的槽函数
    void nearestVertex();//搜索最近100个点的槽函数
    void clearPath();//刷新按钮的槽函数
    void showTrafficCapacity();//显示车流按钮的槽函数
    void createGraph(int);//创建新地图的槽函数

signals:
    void signalLocatePoint(int X,int Y);
    void signalOfOmit(int ID,int _size);
    void signalOfOmit_e(int ID);

private:
    Ui::Widget *ui;
    GraphicDraw * graphicsView;//view
    QGraphicsItemGroup * scenePath;//存放展示的路径
    bool flagToExsitPath;//scene_path是否当前存在的旗子
    QTimer trafficTimer;//车流量的定时刷新器
    int lastUpdateTime=-1;

    QGraphicsScene * sceneAll;//存放包含所有点的场景
    int zoomLevel;//缩略图层级
    vector<VertexDraw*> vertexArray;//点的数组
    vector<vector<PathDraw*>> edgeMatrix;//边的邻接表
    GraphSystem graph;//图系统

    int lastCenterPoint;//用于记录上次标注（定位点或查询起点）的点
    int lastCenterVertex[100000];//用于记录上次显示的路径上的点，因为此系统最多生成10万点，所以设为阈值
    bool flagCenterOn;//已经启用lastcenterpoint变量

    int flagToShowShortestPath;//是否进行最短路径显示
    int lastshortestNumber;//用于记录上次显示的最短路径的结点数量
    int pathStart,pathEnd;//路径的起点终点

    int flagToShowNearestVertex;//是否进行最近100个点的路径显示
    int lastNearestVertex[100];//用于记录上次显示的最近100个点
    int NearestlineVertex[100000];//用于记录需要显示的连边的端点
    int edgenum=0;//记录100个最近的点相互共有多少边

    bool flagToShowTrafficCapacity;//记录当前是否显示车流量

    int vertex;//记录创建图的结点数量

    bool mapOpen;//判断当前是否打开了地图的标记


public:
    const qreal scale[4]={0.85,0.55,0.35,0.2};//缩放层级之间的界限，层级越小代表地图越“缩”
    const int vertexRadiusForDiffLevel[5]={3,6,9,12,15};//不同层级下点的半径
    const qreal pathWidthForDiffLevel[5]={2,3,4,5,7};//不同层级下线的宽度
    const int size_icon[5]={2,4,6,9,14};//不同层级下端点图标的放大倍率
    QGraphicsPixmapItem* PIN[2];//端点图标的指针
};


#endif // WIDGET_H
