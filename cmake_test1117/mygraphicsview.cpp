#include "mygraphicsview.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

myGraphicsView::myGraphicsView(QWidget *parent):
    QGraphicsView(parent)
{
//    resize(400,300);
//    setAlignment(Qt::AlignCenter);
    qDebug()<<"view Map to Scene: "<<mapToScene(QPoint(0,0));
}

myGraphicsView::~myGraphicsView()
{
}

void myGraphicsView::mousePressEvent(QMouseEvent *event)
{
    QPoint viewPos = event->pos();
    qDebug()<<"viewPos: "<<viewPos;
    QPointF scenePos = mapToScene(viewPos);
    qDebug()<<"scenePos: "<<scenePos;
}


bool myGraphicsView::eventFilter(QObject *object, QEvent *event)
{
    if ( event->type() == QEvent::Wheel)
    {
        return true;
    }
    return false;
}
