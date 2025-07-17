#ifndef SCENEEDGE_H
#define SCENEEDGE_H

#include <QGraphicsLineItem>
#include <QObject>
#include <QWidget>
#include<QPen>

class PathDraw : public QObject,public QGraphicsLineItem
{
    Q_OBJECT
public:
    PathDraw();
    PathDraw(int ,int ,int,int ,int ,int ,int ,const QPen &);
    ~PathDraw();

private:
    int v1,v2;
    int x1,y1;
    int x2,y2;

    int flagOfIgnoreLevel;//
    int flag_cross;//记录是否是跨区域的边，已废弃

    QPen mypen;

public:
    void setFlagIgnore(int i){flagOfIgnoreLevel=i;}
    void setFlagCross(int i){flag_cross=i;}
    int getFlagOmit(){return flagOfIgnoreLevel;}
    int getFlagCross(){return flag_cross;}

    int getv1(){return v1;}
    int getv2(){return v2;}

    void set_width(int w){
        mypen.setWidth(w);
        setPen(mypen);
    }

    void set_mypen(QPen P){
        mypen=P;
        setPen(mypen);
    }


public slots:
    void theSlotOfOmit_e(int key);

};

#endif // SCENEEDGE_H
