#include "mapper_40.h"
#include<QPainter>


heatMap::heatMap(int radius, int width,int height,int maxcount,QWidget *parent)
    : m_radius(radius),m_width(width),m_height(height),m_maxCount(maxcount),QWidget(parent)
{
    // m_radius  绘制半经
    //m_width  绘制区域的宽度
    // m_height 绘制区域的长度
    //m_maxCount 最大值

    m_palette = new GradientPalette(DEFAULT_WIDTH);

    m_palette->setColorAt(palette_min_40,Qt::white);
    m_palette->setColorAt(0.20, Qt::blue);
    m_palette->setColorAt(0.35, Qt::cyan);
    m_palette->setColorAt(0.60, Qt::green);
    m_palette->setColorAt(0.75, Qt::yellow);
    m_palette->setColorAt(0.90, Qt::red);


    m_offscreenBuffer = new QImage(width, height, QImage::Format_ARGB32_Premultiplied);
    m_offscreenBuffer->fill(Qt::transparent);


    m_mainCanvas= new QImage(width,height, QImage::Format_ARGB32);
    m_alphaCanvas=new QImage(width,height, QImage::Format_ARGB32);
    m_mainCanvas->fill(Qt::transparent);
    m_alphaCanvas->fill(0);



}

heatMap::~heatMap()
{
    // m_mainCanvas->fill(Qt::transparent);
    // m_alphaCanvas->fill(Qt::transparent);
    m_points.clear();
    delete m_palette;
    delete m_offscreenBuffer;
    delete m_mainCanvas;
    delete m_alphaCanvas;
    delete[] m_colorCache;  // 注意是数组删除
}

void heatMap::updateData(QList<Point_commondata> data)
{
    // QElapsedTimer timer;
    // timer.start();
    m_points=data;
    //qDebug()<<data.length()<<"m_points.size";
    drawMap();
    this-> update();
    //qDebug() << "draw time:" << timer.elapsed() << "ms";
    //repaint();
}

void heatMap::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QFont font;
    qDebug() << "Custom painting started";
    font.setPixelSize(20);
    painter.setFont(font);
    painter.setPen(QPen(Qt::black,1));
#ifdef QT_DEBUG
    qDebug() << "Long:" << this->height() << "Width:" << this->width();
#endif

    // 计算缩放比例，将内部图像缩放到widget实际大小
    double scaleX = static_cast<double>(this->width()) / m_width;
    double scaleY = static_cast<double>(this->height()) / m_height;
    
    // 使用缩放绘制
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.scale(scaleX, scaleY);

//绘制做表格，可注释掉
    // for(int i=0;i<m_width+10;i+=10){
    //     painter.drawLine(i, 0, i, m_height);
    // }
    // for(int i=0;i<m_height+10;i+=10){
    //     painter.drawLine(0,i,m_width,i);
    // }

    painter.drawImage(0, 0, *m_mainCanvas);

}

void heatMap::drawRadialGradient( const Point_commondata &center,QPainter& m_p)
{
    //
    qreal normalized = static_cast<qreal>(center.count) / m_maxCount;
    normalized = qBound(0.0, normalized, 1.0); // 限制在 [0, 1] 范围
    int alpha = qBound(0, static_cast<int>(normalized * 255), 255);
    alpha = std::round(static_cast<float>(alpha) / 25.5f) * 25.5f;  // 优化后的分段
    //qDebug()<<alpha<<"优化后的分段alpha";
    //每个单独的点为径向渐变色的圆，且透明度从中心到边缘逐渐降低
    QRadialGradient gradient(center.posX, center.posY, m_radius);
    gradient.setColorAt(0, QColor(0, 0, 0, alpha));
    gradient.setColorAt(1, QColor(0, 0, 0, 0));

    m_p.setPen(Qt::NoPen);
    m_p.setBrush(gradient);
    m_p.drawEllipse(QPoint(center.posX, center.posY), m_radius, m_radius);
    colorize(center.posX, center.posY,alpha);

}

void heatMap::colorize(int x, int y,int a)
{
    //qDebug()<<"绘制2";
    int left = x - m_radius;
    int top = y - m_radius;
    int right = x + m_radius;
    int bottom = y + m_radius;


    if (left < 0)
        left = 0;

    if (top < 0)
        top = 0;

    if (right > m_width)
        right = m_width;

    if (bottom > m_height)
        bottom = m_height;

    int alpha = 0;
    int finalAlpha = 0;
    QColor color;
    for (int i = left; i < right; ++i) {
        for (int j = top; j < bottom; ++j) {
            alpha = qAlpha(m_alphaCanvas->pixel(i, j));
            //qDebug()<<alpha<<"colorize alpha透明度";
            if (!alpha)
                continue;
            finalAlpha = (alpha < m_opacity ? alpha : m_opacity);

            // 经测试该方法导致崩溃 color = m_colorCache[i];
            color = m_palette->getColorAt(alpha);
            m_mainCanvas->setPixel(i, j, qRgba(color.red(),
                                               color.green(),
                                               color.blue(),
                                               finalAlpha));
        }
    }
}

void heatMap::drawMap()
{
    // 使用离屏缓冲区
    QElapsedTimer timer;
    timer.start();
    m_alphaCanvas->fill(QColor(0, 0, 0, 0));
    m_mainCanvas->fill(QColor(0, 0, 0, 0));
    // 批量绘制所有数据点
    QPainter painter(m_alphaCanvas);
    for ( auto &pt : m_points) {
        drawRadialGradient( pt,painter);
    }
    painter.end();
    // qDebug() << "Draw time:" << timer.elapsed() << "ms";


}


