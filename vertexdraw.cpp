#include "vertexdraw.h"
#include "widget.h"

VertexDraw::VertexDraw()
{

}

VertexDraw::VertexDraw(int X,int Y,int i,int _size, QGraphicsItem *parent = nullptr):QGraphicsEllipseItem(X-_size, Y-_size, _size*2, _size*2, parent){
    No=i;
    x=X;
    y=Y;
    flagOfIgnoreLevel=0;
    flagToAlwaysShow=0;

    setFlag(QGraphicsItem::ItemIsSelectable);

    setBrush(QBrush(Qt::darkGray));
    pen_outline.setColor(Qt::blue);
    pen_outline.setWidth(_size/3);
    setPen(pen_outline);
    {
        //    labelItem = new QGraphicsTextItem(QString::number(-1),this);
        //    // 设置标号文本项的字体和颜色
        //    QFont font("Arial", 10);
        //    labelItem->setFont(font);
        //    labelItem->setDefaultTextColor(Qt::black);
        //    // 设置标号文本项的位置
        //    QPointF labelPos = boundingRect().topRight() + QPointF(5, -5); // 在椭圆的右上角偏移一些距离
        //    labelItem->setPos(labelPos);
    }
}

void VertexDraw::mousePressEvent(QGraphicsSceneMouseEvent *event){
    //printf("click point!\n");
    emit signalClickPoint(0,event,getNo(),getx(),gety());
    //    printf("vertex: %d,%d,%d",getNo(),getx(),gety());
    QGraphicsEllipseItem::mousePressEvent(event);
}

void VertexDraw::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    emit signalClickPoint(1,event,getNo(),getx(),gety());
    //    printf("vertex: %d,%d,%d",getNo(),getx(),gety());
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}

void VertexDraw::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    emit signalClickPoint(2,event,getNo(),getx(),gety());
    QGraphicsEllipseItem::mouseDoubleClickEvent(event);
}

void VertexDraw::theSlotOfOmit(int level,int _size){
    setVisible(1);
    setRect(x-_size,y-_size,_size*2,_size*2);
    pen_outline.setWidth(_size/3);
    if(flagOfIgnoreLevel<level)
        setVisible(0);

}
