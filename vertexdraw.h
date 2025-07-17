#ifndef SCENEPOINT_H
#define SCENEPOINT_H

#include <QGraphicsEllipseItem>
#include <QObject>
#include <QWidget>
#include <QGraphicsTextItem>
#include "graphsystem.h"

class VertexDraw : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT

public:
    explicit VertexDraw();
    explicit VertexDraw(int,int,int,int,QGraphicsItem *);
    ~VertexDraw(){}
    inline int getx(){return x;}
    inline int gety(){return y;}
    inline int getNo(){return No;}
    inline int getFlagInomit(){return flagOfIgnoreLevel;}
    inline void setFlagInomit(int k){flagOfIgnoreLevel=k;}
    inline int getFlagVisi(){return flagToAlwaysShow;}
    inline void setFlagVisi(bool k){flagToAlwaysShow=k;}

public:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void theSlotOfOmit(int ID,int _size);

signals:
    void signalClickPoint(int ID,QGraphicsSceneMouseEvent* event,int i,int x,int y);

private:
    int No;
    int x,y;
    bool flagOfIgnoreLevel;
    bool flagToAlwaysShow;
    QPen pen_outline;

    QGraphicsTextItem* labelItem;
};

#endif // SCENEPOINT_H
