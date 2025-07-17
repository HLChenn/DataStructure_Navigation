#include "graphicdraw.h"


GraphicDraw::GraphicDraw(QWidget *parent) {

}

GraphicDraw::~GraphicDraw() {}

void GraphicDraw::mousePressEvent(QMouseEvent *event) {
    emit signalMouseEvent(0, event);
    QGraphicsView::mousePressEvent(event);
}

void GraphicDraw::mouseMoveEvent(QMouseEvent *event) {
    emit signalMouseEvent(1, event);
    QGraphicsView::mouseMoveEvent(event);
}

void GraphicDraw::mouseReleaseEvent(QMouseEvent *event) {
    emit signalMouseEvent(2, event);
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicDraw::mouseDoubleClickEvent(QMouseEvent *event) {
    emit signalMouseEvent(3, event);
    QGraphicsView::mouseDoubleClickEvent(event);
}

void GraphicDraw::wheelEvent(QWheelEvent *event) {
    emit signalWheelEvent(event);
    QGraphicsView::wheelEvent(event);
}

void GraphicDraw::locatePoint(int x,int y){

}
