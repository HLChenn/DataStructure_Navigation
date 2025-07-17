#include "widget.h"
#include "ui_widget.h"

#include "graphicdraw.h"
#include "graphsystem.h"
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include "vertexdraw.h"
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <QFileInfo>
#include <chrono>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{

    ui->setupUi(this);

    //图的建立
    graph=GraphSystem();

    initGraphicsView();
    //这是用来测试整个页面的生成时间，不重要

    //以下是几个重要功能按钮的槽函数绑定：
    //0.定位某一点
    //1.导出地图
    //2.导入地图
    //3.搜索路径
    //4.搜索最近的100个点
    //5.显示实时车流量
    //6.刷新地图
    connect(ui->locateButton,&QPushButton::clicked,this,&Widget::locateVertex);

    connect(ui->saveGraphButton,&QPushButton::clicked,this,&Widget::exportGraph);
    ui->saveGraphButton->setToolTip("保存当前地图");//设置鼠标悬停在这里时显示一下是什么功能，其实无用

    connect(ui->openButton,&QPushButton::clicked,this,&Widget::importGraph);
    ui->openButton->setToolTip("载入文件地图");

    connect(ui->searchShortestPathButton,&QPushButton::clicked,this,&Widget::searchShortestPath);
    connect(ui->searchFastestPathButton,&QPushButton::clicked,this,&Widget::searchFastestPath);

    connect(ui->searchNearestButton,&QPushButton::clicked,this,&Widget::nearestVertex);

    connect(ui->searchCapacityButton,&QPushButton::clicked,this,&Widget::showTrafficCapacity);


    connect(ui->refreshButton,&QPushButton::clicked,this,&Widget::clearPath);
    ui->refreshButton->setToolTip("刷新");

    //重要：点击生成地图的按钮
    QObject::connect(ui->createGraphButton, &QPushButton::clicked, [&]() {
        bool ok;
        int value = QInputDialog::getInt(nullptr, "正在建立一个新的地图", "请输入地图顶点数量（1万-10万之间):", 10000, 10000, 100000, 100, &ok);
        if (ok) {
            // qDebug() << "请输入要生成的地点数目：" << value;用来测试，无用
            auto start = std::chrono::steady_clock::now();
            createGraph(value);
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            qDebug()<<double(duration.count())/1000<<"seconds!";//用以测试建立一个图所需的时间
        }
        else{
            //QMessageBox::about(nullptr, "提示", "已退出地图创建流程！");
        }
    });
    ui->createGraphButton->setToolTip("创建新地图");
    ui->xPositionValue->setText("未知");
    ui->yPositionValue->setText("未知");
    trafficTimer.setInterval(3000);
    connect(&trafficTimer, &QTimer::timeout, this, [this]() {
        if (!flagToShowTrafficCapacity) return;

        graph.refreshTrafficFlow(-1);
        qint64 jam_factor = 0;
        for (int i = 0; i < graph.VertexNumber; i++) {
            for (int j = 0; j < graph.VertexList[i]->getEdgeCount(); j++) {
                jam_factor = (graph.calculateTrafficCondition(i, graph.VertexList[i]->getOneEdgeRecord(j)) / 1000) % 256;
                auto& edg = edgeMatrix[i][j];
                edg->set_mypen(QPen(QBrush(QColor(jam_factor, 256 - jam_factor, 0, 255)), edg->pen().widthF()));
            }
        }
    });
}

void Widget::initGraphicsView(){

    //创建GraphicsView与GraphicsScene
    mapOpen=0;
    graphicsView=new GraphicDraw(this);

    //定义一下图形界面中地图显示的区域，宽高11000，从（-500,-500）到（10500,10500）
    //实际地图最大的大小肯定是从（0,0）到（10000，10000）
    graphicsView->setSceneRect(-500,-500,11000,11000);
    sceneAll = new QGraphicsScene();
    graphicsView->setScene(sceneAll);
    scenePath=new QGraphicsItemGroup();

    //取消滚动条
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->verticalScrollBar()->setDisabled(1);
    graphicsView->horizontalScrollBar()->setDisabled(1);

    //初始化成员变量：路径查询的起点终点为-1
    pathStart=-1;
    pathEnd=-1;

    //将缩放功能与槽函数链接
    connect(graphicsView, &GraphicDraw::signalWheelEvent, this, &Widget::theSlotWheelEvent);
    connect(graphicsView, &GraphicDraw::signalMouseEvent, this, &Widget::theSlotMouseEvent);
    ui->mapArea->addWidget(graphicsView);//把地图显示的区域添加到图形页面当中
}

//创建地图的图形表示
void Widget::geneSceneForAll(){
    int vertexlen=graph.VertexNumber;

    //笔刷颜色设置
    // QBrush darkgrayBrush(Qt::darkGray);
    // QPen outlinePen(Qt::black);
    QPen pathPen(Qt::black,2);

    //画边，这里的实现逻辑比较重要
    edgeMatrix.clear();
    for(int i=0;i<vertexlen;i+=1){
        int xv=graph.VertexList[i]->getX(),
            yv=graph.VertexList[i]->getY(),
            edgenum=graph.VertexList[i]->getEdgeCount();
        vector<PathDraw*> _edges;
        for(int j=0;j<edgenum;j++){
            //首先获当前i节点第j条边连接的点的编号，即i→vertexEnd
            int vertexEnd=graph.VertexList[i]->getOneEdgeRecord(j);
            PathDraw* sp=new PathDraw(i,vertexEnd,xv,yv,graph.VertexList[vertexEnd]->getX(),graph.VertexList[vertexEnd]->getY(),0,pathPen);

            sp->setFlagIgnore(graph.checkEdgeVisibility(sp->getv1(),sp->getv2(),0)-1);

            sceneAll->addItem(sp);
            connect(this,&Widget::signalOfOmit_e,sp,&PathDraw::theSlotOfOmit_e);
            _edges.push_back(sp);
        }
        edgeMatrix.push_back(_edges);
    }

    //描跨区域的边
    /* code to be done... */

    //画点，与画边逻辑重要性相当
    for(int i=0;i<vertexlen;i+=1) {
        VertexDraw* sp=new VertexDraw(graph.VertexList[i]->getX(),graph.VertexList[i]->getY(),i,4,NULL);
        sceneAll->addItem(sp);

        sp->setFlagInomit(graph.checkVertexVisibility(sp->getNo(),0)-1);
        sp->setZValue(1000);

        connect(sp,&VertexDraw::signalClickPoint,this,&Widget::theSlotForPoint);
        connect(this,&Widget::signalOfOmit,sp,&VertexDraw::theSlotOfOmit);
        vertexArray.push_back(sp);
    }

}

void Widget::geneSceneForIgnore(){
    int znum=graph.MP->zoneCount;

    // //画点
    // for(int i=0;i<znum;i+=1) {
    //     for(int j=0;j<znum;j++){
    //         int vertex_not_omit=graph.Zones[i][j][0];//每个区域权重最高的那个点
    //         vertexArray[vertex_not_omit]->setFlagInomit(4);//设置为最高级别，怎么都不会被隐藏（除了放到最大的时候）
    //         /*
    //         scenePoint* sp=new scenePoint(gph.VertexList[vertex_scene_few]->getx(),gph.VertexList[vertex_scene_few]->gety(),vertex_scene_few,10,NULL);
    //         scene_few->addItem(sp);
    //         connect(sp,&scenePoint::signalClickPoint,this,&Widget::theSlotForPoint);
    //         point_array_few.push_back(sp);
    //         cout<<"vertex "<<sp->getNo()<<" is No."<<i<<" at "<<sp->getx()<<" "<<sp->gety()<<endl;*/
    //     }
    // }

    //画边
    int _v1,_v2;
    //cout<<"start draw line in scene_few\n";
    for(int i=0;i<znum;i+=1) {
        for(int j=0;j<znum;j++){
            cout<<"zone: "<<i<<" "<<j<<endl;
            //up
            if(j-1>=0&&j-1<znum){
                _v1=graph.Zones[i][j][0];
                _v2=graph.Zones[i][j-1][0];
                graph.MP->findAStarPath(_v1,_v2);
                pathFlagIgnore();
            }
            //down
            if(j+1>=0&&j+1<znum){
                _v1=graph.Zones[i][j][0];
                _v2=graph.Zones[i][j+1][0];
                graph.MP->findAStarPath(_v1,_v2);
                pathFlagIgnore();
            }
            //left
            if(i-1>=0&&i-1<znum){
                _v1=graph.Zones[i][j][0];
                _v2=graph.Zones[i-1][j][0];
                graph.MP->findAStarPath(_v1,_v2);
                pathFlagIgnore();
            }
            //right
            if(i+1>=0&&i+1<znum){
                _v1=graph.Zones[i][j][0];
                _v2=graph.Zones[i+1][j][0];
                graph.MP->findAStarPath(_v1,_v2);
                pathFlagIgnore();
            }
        }
    }
}

int colorpen_width=3;
QPen redpen(Qt::red,colorpen_width,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
QPen bluepen(Qt::blue,colorpen_width,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
QPen darkyellowpen(Qt::darkYellow,colorpen_width,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
QPen greenpen(Qt::green,colorpen_width,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
QPen yellowpen(Qt::yellow,colorpen_width,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
QPen darkredpen(Qt::darkRed,colorpen_width,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
QPen blackpen(Qt::black,colorpen_width,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
QPen darkgreenpen(Qt::darkGreen,colorpen_width,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
QPen backgroundpen(Qt::black,colorpen_width+1,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);

//给缩略图中显示的路径标上记号
void Widget::pathFlagIgnore(){

    int num=graph.MP->path.size();
    auto &path=graph.MP->path;
    for(int i=0;i<num-1;i++){
        int start_p=path[i];//当前路径段的起点编号
        int end_p=path[i+1];//当前路径段的终点编号
        int edgeNumOfStart=graph.VertexList[start_p]->getEdgeCount();//获取起点所相连的边的总数
        for(int j=0;j<edgeNumOfStart;j++){
            if(end_p==graph.VertexList[start_p]->getOneEdgeRecord(j)){
                edgeMatrix[start_p][j]->setFlagIgnore(4);
                break;
            }
        }
        int enum_e=graph.VertexList[end_p]->getEdgeCount();
        for(int j=0;j<enum_e;j++){
            if(start_p==graph.VertexList[end_p]->getOneEdgeRecord(j)){
                edgeMatrix[end_p][j]->setFlagIgnore(4);
                break;
            }
        }
    }
}



void Widget::resetQpenWidth(int pen_width){
    redpen.setWidth(pen_width);
    darkyellowpen.setWidth(pen_width);
    greenpen.setWidth(pen_width);
    yellowpen.setWidth(pen_width);
    darkredpen.setWidth(pen_width);
    blackpen.setWidth(pen_width);
    darkgreenpen.setWidth(pen_width);
    backgroundpen.setWidth(pen_width+2);
}

void Widget::setPathIcon(){
    //设置路径起终点
    vertexArray[pathStart]->setBrush(QBrush(QColorConstants::Svg::gold));
    vertexArray[pathStart]->setFlagVisi(1);
    vertexArray[pathEnd]->setBrush(QBrush(QColorConstants::Svg::orange));
    vertexArray[pathEnd]->setFlagVisi(1);
    //设置路径起终点图标，测试，无用
    // cout<<"begin set icon"<<endl;
    //scene_all->addItem(PIN[0]);
    // int ratio=size_icon[getCurrentLevel()];
    // PIN[0]->show();
    // PIN[1]->show();

    // PIN[0]->setScale(ratio);
    // PIN[0]->setPos(vertexArray[pathStart]->getx()-PIN[0]->pixmap().width()*ratio/2,
    //                vertexArray[pathStart]->gety()-PIN[0]->pixmap().height()*ratio);
    // PIN[0]->setZValue(10000);

    // // scene_all->addItem(PIN[1]);
    // PIN[1]->setScale(ratio);
    // PIN[1]->setPos(vertexArray[pathEnd]->getx()-PIN[1]->pixmap().width()*ratio/2,
    //                vertexArray[pathEnd]->gety()-PIN[1]->pixmap().height()*ratio);
    // PIN[1]->setZValue(10000);
}

//按下查询最短路径查询后，显示路径，传进来的参数pathlist是用最短路径算法算完后的路径数组
void Widget::showShortestPath(vector<int> pathlist){
    refreshPath();
    //delete scene_path;
    // cout<<"create new path"<<endl;
    scenePath=new QGraphicsItemGroup();//新建一个路径显示组

    int len=pathlist.size();
    QGraphicsLineItem* lin;
    resetQpenWidth(edgeMatrix[1][1]->pen().width()*2+10);
    for(int i=0;i<len-1;i++){
        int x1=graph.VertexList[pathlist[i]]->getX(),
            y1=graph.VertexList[pathlist[i]]->getY(),
            x2=graph.VertexList[pathlist[i+1]]->getX(),
            y2=graph.VertexList[pathlist[i+1]]->getY();

        lin=new QGraphicsLineItem(x1,y1,x2,y2);
        lin->setPen(redpen);
        lin->setZValue(3);
        scenePath->addToGroup(lin);//将画好的线段添加到路径显示组里
    }
    //路径组全部添加后，设置路径的起点与终点，然后加下图标和点的颜色
    pathStart =pathlist[0];
    pathEnd = pathlist[len-1];
    setPathIcon();

    sceneAll->addItem(scenePath);
    flagToExsitPath=1;//表明此时存在路径显示，便于在refreshPath中判断
}

void Widget::showFastestPath(vector<int> pathlist){
    refreshPath();
    scenePath=new QGraphicsItemGroup();

    int len=pathlist.size();
    QGraphicsLineItem* line,* lineBackground;
    resetQpenWidth(edgeMatrix[1][1]->pen().width()*1.5);

    // 创建更粗的画笔
    QPen thickGreenPen = greenpen;
    QPen thickYellowPen = yellowpen;
    QPen thickRedPen = redpen;
    QPen thickDarkRedPen = darkredpen;
    QPen thickBackgroundPen = backgroundpen;

    // 设置画笔宽度（例如原始宽度的5倍）
    float thicknessFactor = 2.0;
    thickGreenPen.setWidth(greenpen.width() * thicknessFactor);
    thickYellowPen.setWidth(yellowpen.width() * thicknessFactor);
    thickRedPen.setWidth(redpen.width() * thicknessFactor);
    thickDarkRedPen.setWidth(darkredpen.width() * thicknessFactor);
    thickBackgroundPen.setWidth(backgroundpen.width() * thicknessFactor);

    for(int i=0,edgeno=5;i<len-1;i++){
        int x1=graph.VertexList[pathlist[i]]->getX(),
            y1=graph.VertexList[pathlist[i]]->getY(),
            x2=graph.VertexList[pathlist[i+1]]->getX(),
            y2=graph.VertexList[pathlist[i+1]]->getY();

        line=new QGraphicsLineItem(x1,y1,x2,y2);
        lineBackground=new QGraphicsLineItem(x1,y1,x2,y2);

        // 使用更粗的画笔
        lineBackground->setPen(thickBackgroundPen);

        int cap=graph.calculateTrafficCondition(pathlist[i],pathlist[i+1]);
        if(cap<=10000)
        {
            line->setPen(thickGreenPen);//表示畅通
        }
        else if(cap>10000&&cap<=50000)
        {
            line->setPen(thickYellowPen);//表示轻度拥堵
        }
        else if(cap>50000&&cap<=100000)
        {
            line->setPen(thickRedPen);//表示中度拥堵
        }
        else
        {
            line->setPen(thickDarkRedPen);//表示重度拥堵
        }

        lineBackground->setZValue(edgeno++);
        line->setZValue(edgeno++);

        scenePath->addToGroup(line);
        scenePath->addToGroup(lineBackground);
    }
    pathStart =pathlist[0];
    pathEnd = pathlist[len-1];
    setPathIcon();

    sceneAll->addItem(scenePath);
    flagToExsitPath=1;
}

void Widget::showNearestVertexPath(vector<int> pathlist,int edgenum){
    refreshPath();
    //delete scene_path;测试用，已废弃
    scenePath=new QGraphicsItemGroup();
    resetQpenWidth(edgeMatrix[1][1]->pen().width()*1.5);
    //int len=pathlist.size();
    QGraphicsLineItem* lin;
    for(int i=0;i<edgenum;i++){
        int x1=graph.VertexList[pathlist[2*i]]->getX(),
            y1=graph.VertexList[pathlist[2*i]]->getY(),
            x2=graph.VertexList[pathlist[2*i+1]]->getX(),
            y2=graph.VertexList[pathlist[2*i+1]]->getY();

        lin=new QGraphicsLineItem(x1,y1,x2,y2);
        lin->setPen(bluepen);//设置线的显示颜色
        lin->setZValue(4);//把它的显示级别设置的高一点，放在界面顶层
        scenePath->addToGroup(lin);
    }
    sceneAll->addItem(scenePath);
    flagToExsitPath=1;
}

//int tot=1;
void Widget::refreshPath(){
    if(flagToExsitPath){
        // cout<<"showpath4: delete old path"<<endl;
        foreach (QGraphicsItem *itemToRefresh, scenePath->childItems()) {
            sceneAll->removeItem(itemToRefresh);
            delete itemToRefresh;
        }
        if(flagToShowShortestPath){
            /*if(logos){
                foreach (QGraphicsItem *itemToR, logos->childItems()) {
                    scene_all->removeItem(itemToR);
                    delete itemToR;
                }
            }else{;}*/

            //        PIN[0]->setPos(15000,15000);
            //        PIN[1]->setPos(15000,15000);
            // PIN[0]->hide();
            // PIN[1]->hide();


            vertexArray[pathStart]->setFlagVisi(0);
            vertexArray[pathStart]->setBrush(QBrush(Qt::darkGray));
            vertexArray[pathEnd]->setFlagVisi(0);
            vertexArray[pathEnd]->setBrush(QBrush(Qt::darkGray));
        }
        flagToExsitPath=0;
        //delete scene_path;
        // cout<<"showpath4: delete over!"<<endl;
    }
}

bool Pressing;
QPointF mousePos;

//拽动鼠标可移动地图
void Widget::theSlotMouseEvent(int eventId, QMouseEvent *event){
    if (eventId==0&&event->button()==Qt::LeftButton&&Pressing==0){
        Pressing=1;
        graphicsView->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
        return;
    }
    // else if (eventId==1&&event->button()==Qt::LeftButton){
    //     if(Pressing){

    //     }
    // }
    else if (eventId==2&&event->button()==Qt::LeftButton){
        Pressing=0;
        graphicsView->setDragMode(QGraphicsView::DragMode::NoDrag);
        return;
    }
    else{}
}

//地图缩放时，改变当前层级zoomlevel
void Widget::zoomLevelChange(int level,int size,int width){
    setSceneIgnoreLevel(level);

    int len=graph.VertexNumber;
    for(int i=0;i<len;i++){
        VertexDraw* p=vertexArray[i];
        p->setRect(p->getx()-size,p->gety()-size,size*2,size*2);
        if(p->getFlagInomit()<level&&!p->getFlagVisi()){//两个判断条件，1.某一个点的显示级别要大于等于当前level才可显示  2.因其他功能而赋予某些点始终显示的跳过此逻辑
            p->setVisible(0);
        }else{
            p->setVisible(1);
        }
    }
    len=edgeMatrix.size();
    for(int i=0;i<len;i++){
        int _len=edgeMatrix[i].size();
        for(int j=0;j<_len;j++){
            PathDraw* p=edgeMatrix[i][j];
            p->set_width(width);
            if((p->getFlagOmit()<level)||(level==0&&p->getFlagOmit()==3)){
                p->setVisible(0);
            }else{
                p->setVisible(1);
            }
        }
    }
    if(flagToExsitPath){
        foreach (auto *item, scenePath->childItems()) {
            if (QGraphicsLineItem* lineItem = dynamic_cast<QGraphicsLineItem*>(item)){
                auto pe=lineItem->pen();
                if(int(lineItem->zValue())%2)
                    pe.setWidth(width*1.5+2+1);
                else pe.setWidth(width*1.5+1);
                lineItem->setPen(pe);
                continue;
            }
        }
    }
    // if(flagToShowShortestPath){
    //     int ratio=size_icon[level];
    //     PIN[0]->setScale(ratio);
    //     PIN[1]->setScale(ratio);
    //     PIN[0]->setPos(vertexArray[pathStart]->getx()-PIN[0]->pixmap().width()*ratio/2,
    //                    vertexArray[pathStart]->gety()-PIN[0]->pixmap().height()*ratio);
    //     PIN[1]->setPos(vertexArray[pathEnd]->getx()-PIN[1]->pixmap().width()*ratio/2,
    //                    vertexArray[pathEnd]->gety()-PIN[1]->pixmap().height()*ratio);
    // }
}

//地图缩放，重要函数
void Widget::theSlotWheelEvent(QWheelEvent *event) {
    //获取鼠标光标所在位置
    QPointF mousePos = event->position();
    QPointF scenePos = graphicsView->mapToScene(QPoint(mousePos.x(),mousePos.y()));//将鼠标所在位置转化为地图坐标位置

    //获取缩放因子与滚轮转动角度
    qreal scaleFactor = graphicsView->transform().m11();
    int wheelDeltaValue = event->angleDelta().y();
    cout<<scaleFactor<<" "<<getCurrentLevel()<<endl;

    if (wheelDeltaValue > 0)//如果角度大于0，需要放大地图（也就是鼠标向上滚）
    {
        if(scaleFactor>1.5){
            return;//当前缩放太大了，直接返回
        }
        graphicsView->scale(1.05, 1.05);//对地图进行缩放
        scaleFactor = graphicsView->transform().m11();
        //如果进入了不同的缩放等级，则相应变化细节
        if(scaleFactor>scale[0]&&getCurrentLevel()!=0){
            zoomLevelChange(0,vertexRadiusForDiffLevel[0],pathWidthForDiffLevel[0]);
        }
        else if(scaleFactor<=scale[0]&&scaleFactor>scale[1]&&getCurrentLevel()!=1){
            zoomLevelChange(1,vertexRadiusForDiffLevel[1],pathWidthForDiffLevel[1]);
        }
        else if(scaleFactor<=scale[1]&&scaleFactor>scale[2]&&getCurrentLevel()!=2){
            zoomLevelChange(2,vertexRadiusForDiffLevel[2],pathWidthForDiffLevel[2]);
        }
        else if(scaleFactor<=scale[2]&&scaleFactor>scale[3]&&getCurrentLevel()!=3){
            zoomLevelChange(3,vertexRadiusForDiffLevel[3],pathWidthForDiffLevel[3]);
        }
    }
    else
    {
        if(scaleFactor < 0.05) return;//缩放因子过小时，无法继续缩放，直接返回
        graphicsView->scale(1.0 / 1.05, 1.0 / 1.05);//对地图进行缩放
        scaleFactor = graphicsView->transform().m11();
        if(scaleFactor<=scale[3]&&getCurrentLevel()!=4){
            zoomLevelChange(4,vertexRadiusForDiffLevel[4],pathWidthForDiffLevel[4]);
        }
        else if(scaleFactor<=scale[2]&&scaleFactor>scale[3]&&getCurrentLevel()!=3){
            zoomLevelChange(3,vertexRadiusForDiffLevel[3],pathWidthForDiffLevel[3]);
        }
        else if(scaleFactor<=scale[1]&&scaleFactor>scale[2]&&getCurrentLevel()!=2){
            zoomLevelChange(2,vertexRadiusForDiffLevel[2],pathWidthForDiffLevel[2]);
        }
        else if(scaleFactor<=scale[0]&&scaleFactor>scale[1]&&getCurrentLevel()!=1){
            zoomLevelChange(1,vertexRadiusForDiffLevel[1],pathWidthForDiffLevel[1]);
        }

    }
    cout<<scaleFactor<<" "<<getCurrentLevel()<<endl;

    QPointF sceneInMap = graphicsView->mapFromScene(scenePos);

    graphicsView->horizontalScrollBar()->setValue(//设置水平滚动条位置
        int(graphicsView->horizontalScrollBar()->value()+sceneInMap.x() - mousePos.x()));
    graphicsView->verticalScrollBar()->setValue(//设置垂直滚动条位置
        int(graphicsView->verticalScrollBar()->value()+sceneInMap.y() - mousePos.y()));
    if(flagCenterOn)vertexArray[lastCenterPoint]->setVisible(1);//保证定位的点不会被省略掉
    cout<<"zoom ending"<<endl;

    /*测试用，已废弃
    cout<<mousePos.x()<<" "<<mousePos.y()<<endl
       <<scenePos.x()<<" "<<scenePos.y()<<endl
     <<sceneInMap.x()<<" "<<sceneInMap.y()<<endl
     <<graphicsView->horizontalScrollBar()->value()<<" "<<graphicsView->verticalScrollBar()->value()<<endl<<endl;*/
}

void Widget::theSlotForPoint(int ID,QGraphicsSceneMouseEvent* event,int i,int x,int y){
    if(ID==0&&event->button()==Qt::MiddleButton&&Pressing==0){
        Pressing=1;
        //printf("vertex: %d,%d,%d\n",i,x,y);
    }
    else if(ID==1&&event->button()==Qt::MiddleButton){
        Pressing=0;
    }
    else if(ID==2&&event->button()==Qt::LeftButton){
        //cout<<"vertex: "<<i<<" x: "<<x<<" y: "<<y<<endl;
        if(flagCenterOn)
            vertexArray[lastCenterPoint]->setBrush(QBrush(Qt::darkGray));
        if(flagToShowShortestPath||flagToShowNearestVertex)//如果图上有最短路径标记或最近100个点标记
        {
            this->ui->vetexNumberValue->setText(QString::number(i));
            lastCenterPoint=ui->vetexNumberValue->text().toInt();
            ui->xPositionValue->setText(QString::number(x));
            ui->yPositionValue->setText(QString::number(y));
        }
        //更新注视的点
        else
        {
            this->ui->vetexNumberValue->setText(QString::number(i));
            lastCenterPoint=ui->vetexNumberValue->text().toInt();
            ui->xPositionValue->setText(QString::number(x));
            ui->yPositionValue->setText(QString::number(y));
            vertexArray[lastCenterPoint]->setBrush(QBrush(Qt::cyan));
            flagCenterOn=1;
        }
    }

}

//2.定位某一个点
void Widget::locateVertex(){
    auto start = std::chrono::steady_clock::now();
    if(!mapOpen)return;
    if(!ui->vetexNumberValue->text().isEmpty()){//1.按照编号来直接定位顶点坐标信息
        if(ui->vetexNumberValue->text().toInt()<0||ui->vetexNumberValue->text().toInt()>=vertex)
            QMessageBox::warning(nullptr, "错误", "顶点标号不在范围内");
        else{
            if(flagCenterOn){
                vertexArray[lastCenterPoint]->setBrush(QBrush(Qt::darkGray));
                if(vertexArray[lastCenterPoint]->getFlagInomit()<getCurrentLevel())
                    vertexArray[lastCenterPoint]->setVisible(0);
            }
            if(flagToShowShortestPath)//图上是否有上次搜索的路径
            {
                for(int i=0;i<lastshortestNumber;i++)
                {
                    vertexArray[lastCenterVertex[i]]->setBrush(QBrush(Qt::darkGray));
                    if(vertexArray[lastCenterVertex[i]]->getFlagInomit()<getCurrentLevel())
                        vertexArray[lastCenterVertex[i]]->setVisible(0);

                }
                refreshPath();
                //顺带把界面显示的值也删掉
                ui->startTextInput->clear();
                ui->endTextInput->clear();
                flagToShowShortestPath=0;
            }
            if(flagToShowNearestVertex)//图上是否有上次搜索的最近100个结点
            {
                for(int i=0;i<100;i++)
                {
                    vertexArray[lastNearestVertex[i]]->setBrush(QBrush(Qt::darkGray));
                    if(vertexArray[lastNearestVertex[i]]->getFlagInomit()<getCurrentLevel())
                        vertexArray[lastNearestVertex[i]]->setVisible(0);
                    vertexArray[lastNearestVertex[i]]->setFlagVisi(0);
                    flagToShowNearestVertex=0;
                }
                refreshPath();
            }
            //更新当前标注的点
            lastCenterPoint=ui->vetexNumberValue->text().toInt();
            vertexArray[lastCenterPoint]->setBrush(QBrush(QColorConstants::Svg::gold));
            vertexArray[lastCenterPoint]->setVisible(1);//全局可见
            graphicsView->centerOn(graph.VertexList[lastCenterPoint]->getX(),graph.VertexList[lastCenterPoint]->getY());
            ui->xPositionValue->setText(QString::number(graph.VertexList[lastCenterPoint]->getX()));
            ui->yPositionValue->setText(QString::number(graph.VertexList[lastCenterPoint]->getY()));
            flagCenterOn=1;
        }
    }
    else if(!ui->xPositionValue->text().isEmpty()&&!ui->yPositionValue->text().isEmpty()){//2.按坐标查找最接近的顶点，并重新显示坐标
        if(ui->xPositionValue->text().toInt()<0||ui->xPositionValue->text().toInt()>9999||ui->yPositionValue->text().toInt()<0||ui->yPositionValue->text().toInt()>9999){
            QMessageBox::warning(nullptr,"错误","坐标数字不在范围内！");
        }
        else{
            int vertexNumber=graph.coordinateToVertex(ui->xPositionValue->text().toInt(),ui->yPositionValue->text().toInt());
            if(flagCenterOn){
                vertexArray[lastCenterPoint]->setBrush(QBrush(Qt::darkGray));
                if(vertexArray[lastCenterPoint]->getFlagInomit()<getCurrentLevel())
                    vertexArray[lastCenterPoint]->setVisible(0);
            }
            if(flagToShowShortestPath)//图上是否有上次搜索的路径
            {
                for(int i=0;i<lastshortestNumber;i++)
                {
                    vertexArray[lastCenterVertex[i]]->setBrush(QBrush(Qt::darkGray));
                    if(vertexArray[lastCenterVertex[i]]->getFlagInomit()<getCurrentLevel())
                        vertexArray[lastCenterVertex[i]]->setVisible(0);

                }
                refreshPath();
                //顺带把界面显示的值也删掉
                ui->startTextInput->clear();
                ui->endTextInput->clear();
                flagToShowShortestPath=0;
            }
            if(flagToShowNearestVertex)//图上是否有上次搜索的最近100个结点
            {
                for(int i=0;i<100;i++)
                {
                    vertexArray[lastNearestVertex[i]]->setBrush(QBrush(Qt::darkGray));
                    if(vertexArray[lastNearestVertex[i]]->getFlagInomit()<getCurrentLevel())
                        vertexArray[lastNearestVertex[i]]->setVisible(0);
                    vertexArray[lastNearestVertex[i]]->setFlagVisi(0);
                    flagToShowNearestVertex=0;
                }
                refreshPath();
            }
            //更新当前标注的点
            lastCenterPoint=vertexNumber;
            vertexArray[lastCenterPoint]->setBrush(QBrush(QColorConstants::Svg::gold));
            vertexArray[lastCenterPoint]->setVisible(1);//全局可见
            graphicsView->centerOn(graph.VertexList[lastCenterPoint]->getX(),graph.VertexList[lastCenterPoint]->getY());
            ui->xPositionValue->setText(QString::number(graph.VertexList[lastCenterPoint]->getX()));
            ui->yPositionValue->setText(QString::number(graph.VertexList[lastCenterPoint]->getY()));
            ui->vetexNumberValue->setText(QString::number(vertexNumber));
            flagCenterOn=1;
        }
        }
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    qDebug()<<double(duration.count())/1000<<"seconds!";//用以测试定位点用时
}

//1.生成地图
void Widget::createGraph(int vnum)
{
    //int vnum=(ui->Vertexnum->text()).toInt();测试用，已废弃
    vertex=vnum;//只是给widget的属性初始化一下，没啥用
    if (vnum <= 9999 || vnum >= 100001)
        QMessageBox::warning(nullptr, "错误", "地图结点数量应在1万-10万之间");
    else{
        QMessageBox::about(nullptr,"提示","生成较多点数的图需要一定时间，请稍等！");
        graph.MP = new Map;
        graph.MP->initializeMap(0,vnum,0);//这就算是将后端应用到前端来了
        graph.VertexNumber = graph.MP->getVertexCount();

        graph.VertexList = new Vertex * [graph.VertexNumber];

        //将MP中的各顶点值导入Graph类的属性
        for (int i = 0; i < graph.VertexNumber; i++)
        {
            Vertex* v = new Vertex;
            v->setVertexProperties(graph.MP->getVertexX(i), graph.MP->getVertexY(i), graph.MP->getVertexWeight(i), graph.MP->getEdgeCount(i), graph.MP->getEdgeTrafficFlow(i), graph.MP->getVertexMark(i), graph.MP->getAdjacentVertices(i));
            graph.VertexList[i] = v;

        }
        vertexArray.clear();
        for (auto& innerVector : edgeMatrix)
        {
            innerVector.clear(); // 清空每个内部向量
        }
        edgeMatrix.clear(); // 清空外部向量

        graph.findImportantVertices();
        zoomLevel=0;
        lastCenterPoint=INT_MAX;
        flagCenterOn=0;
        mapOpen=1;
        flagToShowTrafficCapacity=0;
        ui->searchCapacityButton->setText("查询附近车流量");
        flagToShowShortestPath=0;flagToShowNearestVertex=0;
        pathStart=0;pathEnd=0;
        flagToExsitPath=0;

        graphicsView->setScene(nullptr);
        graphicsView->resetTransform();
        graphicsView->centerOn(5000,5000);//每次新建就定位到地图的中心

        sceneAll->clear();//因为可能不是在运行中第一次建立，所以先清除可能有上次建立的数据
        sceneAll = new QGraphicsScene();
        geneSceneForAll();//生成完整的场景
        cout<<"set scene_all"<<endl;
        geneSceneForIgnore();//生成简化的场景
        cout<<"set scene_omit"<<endl;
        graphicsView->setScene(sceneAll);

        //这里的资源显示还是有问题，要该动
        // PIN[0]=new QGraphicsPixmapItem(QPixmap(":/imgs/resource/placeholder.png"));
        // PIN[1]=new QGraphicsPixmapItem(QPixmap(":/resource/end.png"));
        // sceneAll->addItem(PIN[0]);
        // sceneAll->addItem(PIN[1]);
        // PIN[0]->hide();//一开始肯定是要隐藏的，只有在查询路径后才会显示
        // PIN[1]->hide();
        //在生成地图后，因为还没有定位点，所以x和y都暂时显示未知
        ui->xPositionValue->setText("未知");
        ui->yPositionValue->setText("未知");
        ui->vetexNumberValue->clear();
        // trafficTimer.start();//开始计时
    }

}

void Widget::exportGraph(void)const//地图写入文本文件执行函数
{
    //QString file_path =  QFileDialog::getSaveFileName(self,"save file","C:\Users\Administrator\Desktop" ,"xj3dp files (*.xj3dp);;all files(*.*)");
    if(!mapOpen){
        QMessageBox::warning(nullptr,"警告","当前未打开地图，无法进行保存");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(nullptr, "保存文件", "", "文本文件(*.txt)");
    if(fileName.isEmpty())
        QMessageBox::warning(nullptr,"错误","未选择文件");
    else{
        //if(fileName[0]==NULL&&fileName[1]==NULL)return;
        QFile storagefile(fileName);
        storagefile.open(QIODevice::WriteOnly|QIODevice::Truncate);
        QTextStream out(&storagefile);
        //以覆写方式打开存储目标文本文件
        if (!storagefile.isOpen())//保险机制，若目标文本文件打开失败则报错并直接退出
        {
            qDebug()<< "Storage Fail: Error in Opening Destination File" << '\n';
            QMessageBox::warning(nullptr, "错误", "无法打开文件进行写入");
        }
        else {
            int i = 0;
            out << "-------MapGraph-------" << '\n';//写入地图标题
            out<<graph.MP->zoneCount<<'\t'<<graph.MP->vertexCount<<'\n';//写入地图的顶点数量数据
            for (i=0;i<=(graph.MP->zoneCount*graph.MP->zoneCount);i++)//写入地图区域列表部分的数据
            {
                out << i<<'\t'
                    <<graph.MP->zoneTable[i][0]<<'\t'
                    <<graph.MP->zoneTable[i][1]<<'\t'
                    <<graph.MP->zoneTable[i][2]<<'\t'<<'\n';
            }
            out << '\n';
            for (i=0;i<=graph.MP->zoneTable[graph.MP->zoneCount*graph.MP->zoneCount][2]-1;i++)//写入地图区域间连边列表部分的数据
            {
                out << i<<'\t'
                    <<graph.MP->crossEdges[i][0]<<'\t'
                    <<graph.MP->crossEdges[i][1]<<'\t'
                    <<graph.MP->crossEdges[i][2]<<'\t'<<'\n';
            }
            out << '\n';
            for (i = 0; i <= graph.MP->vertexCount - 1; i++)//写入地图顶点列表部分的数据
            {
                out << i<<'\t'
                    <<graph.MP->vertexTable[i][0]<<'\t'
                    <<graph.MP->vertexTable[i][1]<<'\t'
                    <<graph.MP->vertexTable[i][2]<<'\t';
                for (int j = 0; j <= graph.MP->vertexTable[i][2] - 1; j++)//写入地图全图连边列表部分的数据
                {
                    out << graph.MP->edgeTable[i][j]<<'\t';
                }
                out << '\n';
            }
            storagefile.close();
        }
        QMessageBox::information(nullptr, "成功", "文件已成功保存");}
}

//读取已有图
void Widget::importGraph(void)
{
    // cout<<"open another file"<<endl;
    QString file_name = QFileDialog::getOpenFileName(nullptr, "Open File","", "文本文件 (*.txt)");
    if(file_name.isEmpty())
        QMessageBox::warning(nullptr,"错误","未选择文件");
    else{
        QByteArray fileName=file_name.toUtf8();
        char*file=fileName.data();
        graph.MP = new Map;
        graph.MP->initializeMap(1, 0, file);

        graph.VertexNumber = graph.MP->getVertexCount();
        vertex=graph.VertexNumber;

        graph.VertexList = new Vertex * [graph.VertexNumber];

        //将MP中的各顶点值导入Graph类的属性
        for (int i = 0; i < graph.VertexNumber; i++)
        {
            Vertex* v = new Vertex;
            v->setVertexProperties(graph.MP->getVertexX(i), graph.MP->getVertexY(i), graph.MP->getVertexWeight(i), graph.MP->getEdgeCount(i), graph.MP->getEdgeTrafficFlow(i), graph.MP->getVertexMark(i), graph.MP->getAdjacentVertices(i));
            graph.VertexList[i] = v;

        }

        vertexArray.clear();
        for (auto& innerVector : edgeMatrix) {
            innerVector.clear(); // 清空每个内部向量
        }
        edgeMatrix.clear(); // 清空外部向量

        graph.findImportantVertices();
        zoomLevel=0;
        lastCenterPoint=INT_MAX;
        flagCenterOn=0;
        mapOpen=1;
        flagToShowTrafficCapacity=0;
        ui->searchCapacityButton->setText("查询附近车流量");
        flagToShowShortestPath=0;flagToShowNearestVertex=0;
        pathStart=0;pathEnd=0;
        flagToExsitPath=0;

        graphicsView->setScene(nullptr);
        graphicsView->resetTransform();
        graphicsView->centerOn(5000,5000);

        sceneAll->clear();
        sceneAll = new QGraphicsScene();
        geneSceneForAll();
        cout<<"set scene_all"<<endl;
        geneSceneForIgnore();
        cout<<"set scene_omit"<<endl;
        graphicsView->setScene(sceneAll);

        // PIN[0]=new QGraphicsPixmapItem(QPixmap(":/resource/start.png"));
        // PIN[1]=new QGraphicsPixmapItem(QPixmap(":/resource/end.png"));
        // sceneAll->addItem(PIN[0]);
        // sceneAll->addItem(PIN[1]);
        // PIN[0]->hide();
        // PIN[1]->hide();

        ui->xPositionValue->setText("未知");
        ui->yPositionValue->setText("未知");
        ui->vetexNumberValue->clear();

        cout<<"end read file"<<endl;}
}

//5.查询最短路径模块
void Widget::searchShortestPath()
{
    auto start1 = std::chrono::steady_clock::now();
    if(!mapOpen)return;
        QString start=ui->startTextInput->text();
        QString end=ui->endTextInput->text();

        int startNumber=start.toInt();//先获取要查询的起点与终点的顶点编号
        int endNumber=end.toInt();
        //检查是否合法输入
        if (startNumber <0 || startNumber >= vertex)
            QMessageBox::warning(nullptr, "错误", "起始结点标号不在范围内");
        else if (endNumber <0 || endNumber >= vertex)
            QMessageBox::warning(nullptr, "错误", "终止结点标号不在范围内");
        else{
            //合法则：
            // cout<<"serachpath checkpoint"<<endl;
            ui->vetexNumberValue->clear();
            ui->xPositionValue->setText("未知");
            ui->yPositionValue->setText("未知");
            graph.findShortestDistancePath(startNumber,endNumber);//最短路径查询算法，可以得到更新后的Pathlist[][]和ShortestDistanceVertex[]以及ShortestDistanceNumber
            if(flagCenterOn||flagToShowShortestPath||flagToShowNearestVertex)
            {
                if(flagCenterOn)//首先看是否有定位的点
                {
                    vertexArray[lastCenterPoint]->setBrush(QBrush(Qt::darkGray));
                    if(vertexArray[lastCenterPoint]->getFlagInomit()<getCurrentLevel())
                        vertexArray[lastCenterPoint]->setVisible(0);
                    flagCenterOn=0;
                    ui->vetexNumberValue->clear();
                    ui->xPositionValue->setText("暂无");
                    ui->yPositionValue->setText("暂无");
                }
                if(flagToShowShortestPath)//图上是否有上次搜索的路径
                {
                    for(int i=0;i<lastshortestNumber;i++)
                    {
                        vertexArray[lastCenterVertex[i]]->setBrush(QBrush(Qt::darkGray));
                        if(vertexArray[lastCenterVertex[i]]->getFlagInomit()<getCurrentLevel())
                            vertexArray[lastCenterVertex[i]]->setVisible(0);
                        vertexArray[lastNearestVertex[i]]->setFlagVisi(0);
                        flagToShowShortestPath=0;
                    }
                }
                if(flagToShowNearestVertex)//图上是否有上次搜索的最近100个点
                {
                    for(int i=0;i<100;i++)
                    {
                        vertexArray[lastNearestVertex[i]]->setBrush(QBrush(Qt::darkGray));
                        if(vertexArray[lastNearestVertex[i]]->getFlagInomit()<getCurrentLevel())
                            vertexArray[lastNearestVertex[i]]->setVisible(0);
                        vertexArray[lastNearestVertex[i]]->setFlagVisi(0);
                        flagToShowNearestVertex=0;
                    }
                }

            }
            // cout<<"serachpath checkpoint"<<endl;
            lastshortestNumber=graph.ShortestDistanceNumber;
            //把点画出来
            for(int i=0;i<graph.ShortestDistanceNumber;i++)
            {
                lastCenterVertex[i]=graph.ShortestDistanceVertex[i];
                vertexArray[lastCenterVertex[i]]->setBrush(QBrush(Qt::cyan));
                vertexArray[lastCenterVertex[i]]->setVisible(1);
            }
            // cout<<"serachpath checkpoint"<<endl;
            graphicsView->centerOn(graph.VertexList[lastCenterVertex[0]]->getX(),graph.VertexList[lastCenterVertex[0]]->getY());//定位到路径查询的起点
            flagToShowShortestPath=1;
            // cout<<"serachpath checkpoint"<<endl;
            vector<int> vec;
            //就是把最短路径的数组换成vector的形式
            vec.assign(graph.ShortestDistanceVertex, graph.ShortestDistanceVertex + graph.ShortestDistanceNumber);
            // cout<<"serachpath checkpoint"<<endl;

            showShortestPath(vec);//把用最短路径搜到的路径数组传进去，得到图形化的显示，这里是用来画边
            // cout<<"serachpath checkpoint"<<endl;
            QString pathStr;
            for(size_t i=0;i<vec.size();++i){
                pathStr+=QString("顶点%1").arg(vec[i]);
                if(i!=vec.size()-1){
                    pathStr+="-->";
                }
            }
            QMessageBox::about(nullptr,"信息",QObject::tr("从顶点%1到顶点%2的最短路径的长度为%3\n路径：%4").arg(startNumber).arg(endNumber).arg(graph.getShortestDistanceLength(startNumber,endNumber)).arg(pathStr));}
    auto end1 = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    qDebug()<<double(duration.count())/1000<<"seconds!";//用以测试查询路径所需的时间
}
//查询最快路径
void Widget::searchFastestPath()
{
    auto start1 = std::chrono::steady_clock::now();
    if(!mapOpen)return;
        QString start=ui->startTextInput->text();
        QString end=ui->endTextInput->text();
        int startno=start.toInt();
        int endno=end.toInt();
        if (startno <0 || startno >= vertex)
            QMessageBox::warning(nullptr, "错误", "起始结点标号不在范围内");
        else if (endno <0 || endno >= vertex)
            QMessageBox::warning(nullptr, "错误", "终止结点标号不在范围内");
        else{
            // cout<<"serachpath checkpoint"<<endl;
            graph.findShortestTrafficPath(startno,endno);
            ui->vetexNumberValue->clear();
            ui->xPositionValue->setText("暂无");
            ui->yPositionValue->setText("暂无");
            if(flagCenterOn||flagToShowShortestPath||flagToShowNearestVertex)
            {
                if(flagCenterOn)//首先看是否有定位的点
                {
                    vertexArray[lastCenterPoint]->setBrush(QBrush(Qt::darkGray));
                    if(vertexArray[lastCenterPoint]->getFlagInomit()<getCurrentLevel())
                        vertexArray[lastCenterPoint]->setVisible(0);
                    flagCenterOn=0;
                    ui->vetexNumberValue->clear();
                    ui->xPositionValue->setText("暂无");
                    ui->yPositionValue->setText("暂无");
                }
                if(flagToShowShortestPath)//图上是否有上次搜索的路径
                {
                    for(int i=0;i<lastshortestNumber;i++)
                    {
                        vertexArray[lastCenterVertex[i]]->setBrush(QBrush(Qt::darkGray));
                        if(vertexArray[lastCenterVertex[i]]->getFlagInomit()<getCurrentLevel())
                            vertexArray[lastCenterVertex[i]]->setVisible(0);
                        flagToShowShortestPath=0;
                    }
                }
                if(flagToShowNearestVertex)//图上是否有上次搜索的最近100个点
                {
                    for(int i=0;i<100;i++)
                    {
                        vertexArray[lastNearestVertex[i]]->setBrush(QBrush(Qt::darkGray));
                        if(vertexArray[lastNearestVertex[i]]->getFlagInomit()<getCurrentLevel())
                            vertexArray[lastNearestVertex[i]]->setVisible(0);
                        vertexArray[lastNearestVertex[i]]->setFlagVisi(0);
                        flagToShowNearestVertex=0;
                    }
                    //ui->nearby->clear();
                }

            }
            // cout<<"serachpath checkpoint"<<endl;
            lastshortestNumber=graph.ShortestTrafficNumber;
            for(int i=0;i<graph.ShortestTrafficNumber;i++)
            {
                lastCenterVertex[i]=graph.ShortestTrafficVertex[i];
                vertexArray[lastCenterVertex[i]]->setBrush(QBrush(Qt::cyan));
                vertexArray[lastCenterVertex[i]]->setVisible(1);
            }
            // cout<<"serachpath checkpoint"<<endl;
            graphicsView->centerOn(graph.VertexList[lastCenterVertex[0]]->getX(),graph.VertexList[lastCenterVertex[0]]->getY());
            flagToShowShortestPath=1;
            vector<int> vec;
            vec.assign(graph.ShortestTrafficVertex, graph.ShortestTrafficVertex + graph.ShortestTrafficNumber);
            // cout<<"serachpath checkpoint"<<endl;
            showFastestPath(vec);
            QString pathStr;
            for(size_t i=0;i<vec.size();++i){
                pathStr+=QString("顶点%1").arg(vec[i]);
                if(i!=vec.size()-1){
                    pathStr+="-->";
                }
            }
            graph.calculateTrafficTime(vec);
            // cout<<"serachpath checkpoint"<<endl;
            // QMessageBox::about(nullptr,"信息",QObject::tr("从顶点%1到顶点%2的考虑路况时最佳路径的长度为%3").arg(startno).arg(endno).arg(graph.ShortestTrafficLength(startno,endno)));}
            QMessageBox::about(nullptr,"信息",QObject::tr("从顶点%1到顶点%2的考虑路况时最快路径的长度为%3\n模拟通行时间为：%4秒\n路径：%5").arg(startno).arg(endno).arg(graph.PathList[startno][0]).arg(graph.trafficTime).arg(pathStr));}
    auto end1 = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    qDebug()<<double(duration.count())/1000<<"seconds!";//用以测试查询路径所需的时间
}

//3.显示周围100点及其路径
void Widget::nearestVertex()
{
    auto start = std::chrono::steady_clock::now();
    if(!mapOpen)return;
    int vno=(ui->vetexNumberValue->text()).toInt();
    if (vno <0 || vno >= vertex)
        QMessageBox::warning(nullptr, "错误", "结点标号不在范围内");
    else{
        graph.searchNearestVertex(vno);//用后端函数，最终结果就是体现在NearestVertex[100]

        if(flagCenterOn||flagToShowShortestPath||flagToShowNearestVertex)
        {
            if(flagToShowShortestPath)//图上是否有上次搜索的路径
            {
                for(int i=0;i<lastshortestNumber;i++)
                {
                    vertexArray[lastCenterVertex[i]]->setBrush(QBrush(Qt::darkGray));
                    if(vertexArray[lastCenterVertex[i]]->getFlagInomit()<getCurrentLevel())
                        vertexArray[lastCenterVertex[i]]->setVisible(0);
                }refreshPath();
                ui->startTextInput->clear();
                ui->endTextInput->clear();
                flagToShowShortestPath=0;
            }
            if(flagToShowNearestVertex)//图上是否有上次搜索的最近100个结点
            {
                for(int i=0;i<100;i++)
                {
                    vertexArray[lastNearestVertex[i]]->setBrush(QBrush(Qt::darkGray));
                    if(vertexArray[lastNearestVertex[i]]->getFlagInomit()<getCurrentLevel())
                        vertexArray[lastNearestVertex[i]]->setVisible(0);
                    vertexArray[lastNearestVertex[i]]->setFlagVisi(0);
                }
                flagToShowNearestVertex=0;
            }
        }
        int pos=0;
        for(int i=0;i<100;i++)//找到输入的点在数组中的位置
        {
            if(graph.NearestVertex[i]==vno)
            {
                pos=i;
                break;
            }
        }
        for(int i=0;i<100;i++)
        {
            //更新注视的点
            lastNearestVertex[i]=graph.NearestVertex[i];
            if(i==pos)//如果是查询的点那么特殊标记
                vertexArray[lastNearestVertex[i]]->setBrush(QBrush(QColorConstants::Svg::gold));
            else
                vertexArray[lastNearestVertex[i]]->setBrush(QBrush(Qt::cyan));
            vertexArray[lastNearestVertex[i]]->setVisible(1);//显示这些被标记的点
            vertexArray[lastNearestVertex[i]]->setFlagVisi(1);
        }
        graphicsView->centerOn(graph.VertexList[lastNearestVertex[pos]]->getX(),graph.VertexList[lastNearestVertex[pos]]->getY());
        ui->xPositionValue->setText(QString::number(graph.VertexList[lastNearestVertex[pos]]->getX()));
        ui->yPositionValue->setText(QString::number(graph.VertexList[lastNearestVertex[pos]]->getY()));
        flagToShowNearestVertex=1;//此时标记为1
        int store=0;//记录边的端点此时记录在数组当中的位置
        edgenum=0;//重新初始化当下最近的100个点的相互共边数
        for(int i=0;i<100;i++)//遍历100个临近顶点
        {
            for(int j=0;j<graph.VertexList[graph.NearestVertex[i]]->getEdgeCount();j++)//遍历每个顶点的所有相连边
            {
                int edgeLevel=graph.MP->isEdgeVisible(graph.NearestVertex[i],graph.VertexList[graph.NearestVertex[i]]->getOneEdgeRecord(j),2);
                if(edgeLevel==6)//这条边连接的两个顶点都是最近的100点
                {
                    NearestlineVertex[store]=graph.NearestVertex[i];//把这条边的起点放进去
                    NearestlineVertex[store+1]=graph.VertexList[graph.NearestVertex[i]]->getOneEdgeRecord(j);//把这条边的终点放进去（紧跟）
                    store=store+2;
                    edgenum++;
                }
            }
        }
        vector<int> vec;
        vec.assign(NearestlineVertex, NearestlineVertex + 2*edgenum);
        showNearestVertexPath(vec,edgenum);

        QString NearestVertexStr;
        for(size_t i=0;i<100;++i){
            NearestVertexStr+=QString("顶点%1").arg(lastNearestVertex[i]);
            if(i!=99){
                NearestVertexStr+=", ";
            }
        }
        QMessageBox::about(nullptr,"信息",QObject::tr("临近顶点%1的最近100个顶点分别有：\n%2").arg(lastNearestVertex[pos]).arg(NearestVertexStr));
    }
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    qDebug()<<double(duration.count())/1000<<"seconds!";//用来测试找到100点的时间
}


/*void Widget::showNearestCurrent()测试用，已废弃
{
    int vno=(ui->current->text()).toInt();
    graph.SearchNearestVertex(vno);
}*/

//6.刷新操作
void Widget::clearPath()
{
    //    ui->v_no_value->clear();
    //    if(flagCenterOn)//首先看是否有定位的点
    //    {
    //        point_array[lastCenterPoint]->setBrush(QBrush(Qt::darkGray));
    //        if(point_array[lastCenterPoint]->getFlagInomit()<getFlagOmit())
    //            point_array[lastCenterPoint]->setVisible(0);
    //        flagCenterOn=0;
    //        ui->v_no_value->clear();
    //    }
    if(flagToShowShortestPath)//图上是否有上次搜索的路径
    {
        for(int i=0;i<lastshortestNumber;i++)
        {
            vertexArray[lastCenterVertex[i]]->setBrush(QBrush(Qt::darkGray));
            if(vertexArray[lastCenterVertex[i]]->getFlagInomit()<getCurrentLevel())
                vertexArray[lastCenterVertex[i]]->setVisible(0);
            vertexArray[lastNearestVertex[i]]->setFlagVisi(0);

        }
        //vector<int> vec;
        //vec.assign(lastCenterVertex, lastCenterVertex + lastshortestNumber);
        refreshPath();
        ui->startTextInput->clear();
        ui->endTextInput->clear();
        //ui->path->setChecked(false);
        flagToShowShortestPath=0;
    }
    if(flagToShowNearestVertex)//图上是否有上次搜索的最近100个结点
    {
        for(int i=0;i<100;i++)
        {
            vertexArray[lastNearestVertex[i]]->setBrush(QBrush(Qt::darkGray));
            if(vertexArray[lastNearestVertex[i]]->getFlagInomit()<getCurrentLevel())
                vertexArray[lastNearestVertex[i]]->setVisible(0);
            vertexArray[lastNearestVertex[i]]->setFlagVisi(0);
            flagToShowNearestVertex=0;
        }
        //vector<int> vec;
        //vec.assign(lastNearestVertex, lastNearestVertex + 2*edgenum);
        refreshPath();
        //ui->nearby->clear();
    }
    // if(ui->refreshDataAll->isChecked())
    // {
    //     ui->refreshDataAll->setAutoExclusive(false);
    //     ui->refreshDataAll->setChecked(false);
    //     ui->refreshDataAll->setAutoExclusive(true);
    // }
    // if(ui->refresh2->isChecked())
    // {
    //     graph.RefreshTrafficFlow(1);
    //     if(flagToShowTrafficCapacity){
    //         qint64 jam_factor=0;
    //         for(int i=0;i<graph.VertexNumber;i++){
    //             for(int j=0;j<graph.VertexList[i]->getEdgeNum();j++){
    //                 jam_factor=(graph.CalculateTrafficCondition(i,graph.VertexList[i]->getOneEdgeRecord(j))/1000)%256;
    //                 auto& edg=edgeMatrix[i][j];
    //                 edg->set_mypen(QPen( QBrush(QColor(jam_factor,256-jam_factor,0,255)) , edg->pen().widthF() ));
    //             }
    //         }
    //     }
    //     ui->refresh2->setChecked(false);
    //     ui->refresh2->setAutoExclusive(false);
    //     ui->refresh2->setChecked(false);
    //     ui->refresh2->setAutoExclusive(true);
    // }
    // else if(ui->refresh3->isChecked())
    // {
    //     graph.RefreshTrafficFlow(-1);
    //     if(flagToShowTrafficCapacity){
    //         qint64 jam_factor=0;
    //         for(int i=0;i<graph.VertexNumber;i++){
    //             for(int j=0;j<graph.VertexList[i]->getEdgeNum();j++){
    //                 jam_factor=(graph.CalculateTrafficCondition(i,graph.VertexList[i]->getOneEdgeRecord(j))/1000)%256;
    //                 auto& edg=edgeMatrix[i][j];
    //                 edg->set_mypen(QPen( QBrush(QColor(jam_factor,256-jam_factor,0,255)) , edg->pen().widthF() ));
    //             }
    //         }
    //     }
        //ui->refresh3->setChecked(false);
        // ui->refresh3->setAutoExclusive(false);
        // ui->refresh3->setChecked(false);
        // ui->refresh3->setAutoExclusive(true);
    // }
}
// 启动定时刷新
void Widget::startTrafficUpdates() {
    trafficTimer.start();  // 启动3秒定时器
    qDebug() << "车流量自动刷新已启动";
}

// 停止定时刷新
void Widget::stopTrafficUpdates() {
    trafficTimer.stop();
    qDebug() << "车流量自动刷新已停止" ;
}
//4.显示某一点周围附近的实时车流量（其实所有的路径车流量都可以显示出来）
void Widget::showTrafficCapacity(){
    auto start = std::chrono::steady_clock::now();
    if (!mapOpen) return;

    int vno = (ui->vetexNumberValue->text()).toInt();
    if (vno < 0 || vno >= vertex) {
        QMessageBox::warning(nullptr, "错误", "结点标号不在范围内");
        return;
    }

    if (!flagToShowTrafficCapacity) {
        // 高亮目标顶点
        vertexArray[vno]->setBrush(QBrush(QColorConstants::Svg::orange));
        vertexArray[vno]->setVisible(1);
        vertexArray[vno]->setFlagVisi(1);
        flagToShowNearestVertex = 0;

        // 更新UI状态
        flagToShowTrafficCapacity = 1;
        graphicsView->centerOn(vertexArray[vno]->getx(), vertexArray[vno]->gety());
        ui->xPositionValue->setText(QString::number(vertexArray[vno]->getx()));
        ui->yPositionValue->setText(QString::number(vertexArray[vno]->gety()));
        ui->searchCapacityButton->setText("关闭车流量显示");
        graph.refreshTrafficFlow(-1);//先刷新一次全局流量
        qint64 CongestionFactor = 0;
        for (int i = 0; i < graph.VertexNumber; i++) {
            for (int j = 0; j < graph.VertexList[i]->getEdgeCount(); j++) {
                CongestionFactor = (graph.calculateTrafficCondition(i, graph.VertexList[i]->getOneEdgeRecord(j)) / 1000) % 256;
                auto& edg = edgeMatrix[i][j];
                edg->set_mypen(QPen(QBrush(QColor(CongestionFactor, 256 - CongestionFactor, 0, 255)), edg->pen().widthF()));
            }
        }
        startTrafficUpdates();

    } else {

        // 恢复默认显示
        vertexArray[vno]->setBrush(QBrush(Qt::darkGray));
        vertexArray[vno]->setFlagVisi(0);
        for (int i = 0; i < graph.VertexNumber; i++) {
            for (int j = 0; j < graph.VertexList[i]->getEdgeCount(); j++) {
                auto& edg = edgeMatrix[i][j];
                edg->set_mypen(QPen(Qt::black, edg->pen().widthF()));
            }
        }
        // 更新UI状态
        flagToShowTrafficCapacity = 0;
        stopTrafficUpdates();
        ui->searchCapacityButton->setText("查询附近车流量");
    }
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    qDebug()<<double(duration.count())/1000<<"seconds!";//用来测试显示车流量的时间
}

Widget::~Widget()
{

    cout<<"destrcutor of Widget\n";

    graphicsView->close();
    delete graphicsView;

    sceneAll->clear();
    delete sceneAll;

    vertexArray.clear();
    for (size_t i=0;i<edgeMatrix.size();i++){
        edgeMatrix[i].clear();
    }
    edgeMatrix.clear();

    delete ui;
    exit(0);

}


