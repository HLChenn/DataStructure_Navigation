#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include<QGraphicsView>
#include<QWheelEvent>
#include<QScrollBar>
#include <QtWidgets>

class GraphicDraw : public QGraphicsView {
    Q_OBJECT

public:
    GraphicDraw(QWidget *parent = nullptr);
    ~GraphicDraw();

public:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

signals:
    void signalMouseEvent(int eventId, QMouseEvent *event);
    void signalWheelEvent(QWheelEvent *event);

public slots:
    void locatePoint(int x,int y);

};


#endif // MYGRAPHICSVIEW_H
