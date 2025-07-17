#include "pathdraw.h"
#include "graphsystem.h"

PathDraw::PathDraw()
{

}

PathDraw::PathDraw(int V1,int V2,int X1,int Y1,int X2,int Y2,int z,const QPen & pen):QGraphicsLineItem(X1,Y1,X2,Y2){
    v1=V1;
    v2=V2;
    x1=X1;
    x2=X2;
    y1=Y1;
    y2=Y2;
    flagOfIgnoreLevel=0;
    flag_cross=0;
    setZValue(z);
    setPen(pen);
    mypen=pen;

}

PathDraw::~PathDraw(){}

void PathDraw::theSlotOfOmit_e(int key){
    setVisible(flagOfIgnoreLevel>=key);
}
