#include "myfootitem.h"
#include <QPainter>
#include <QBitmap>
myFootItem::myFootItem(QGraphicsItem *parent)
{

}

myFootItem::myFootItem(QPixmap img, QGraphicsItem *parent)
{
    m_srcImg = img;
    QBitmap bmp = m_srcImg.createMaskFromColor(QColor(0, 0, 0), Qt::MaskInColor);
    m_srcImg.setMask(bmp);
}

QRectF myFootItem::boundingRect() const
{
    return QRectF(-m_srcImg.width()*0.5,-m_srcImg.height()*0.5,m_srcImg.width(),m_srcImg.height());
}

void myFootItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF paintRect(-m_srcImg.width()*0.5,-m_srcImg.height()*0.5,m_srcImg.width(),m_srcImg.height());
    painter->drawPixmap(boundingRect(),m_srcImg,m_srcImg.rect());
}

void myFootItem::setPixmap(QPixmap img)
{
    m_srcImg = img;
    QBitmap bmp = m_srcImg.createMaskFromColor(QColor(0, 0, 0), Qt::MaskInColor);
    m_srcImg.setMask(bmp);
    update();
}


