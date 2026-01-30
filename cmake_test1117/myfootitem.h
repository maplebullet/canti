#ifndef MYFOOTITEM_H
#define MYFOOTITEM_H
#pragma once

#include <QObject>
#include <QGraphicsItem>
#include <QPixmap>
class myFootItem:public QObject,public QGraphicsItem
{
    Q_OBJECT
public:
    myFootItem(QGraphicsItem *parent = 0);
    myFootItem(QPixmap img ,QGraphicsItem *parent = 0);
    QRectF boundingRect() const override;
    void paint(QPainter *painter,const QStyleOptionGraphicsItem* option,QWidget *widget) override;
    void setPixmap(QPixmap img);

private:
    QPixmap m_srcImg;
};

#endif // MYFOOTITEM_H
