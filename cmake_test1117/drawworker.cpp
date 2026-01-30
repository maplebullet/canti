#include "drawworker.h"
#include <QPainter>
#include <QList>
#include <QRadialGradient>
#include <QDebug>

DrawWorker::DrawWorker(QObject *parent)
    : QObject(parent)
{
}

void DrawWorker::setDrawingParameters(int width, int height, const GradientPalette *palette, int radius, int opacity)
{
    m_width = width;
    m_height = height;
    m_palette = palette;
    m_radius = radius;
    m_opacity = opacity;

    m_alphaCanvas = QImage(width, height, QImage::Format_ARGB32);
    m_mainCanvas = QImage(width, height, QImage::Format_ARGB32);
}

void DrawWorker::drawImage(const QVector<int> &data, const QVector<QPoint> &drawPts, qreal maxCount)
{
    m_alphaCanvas.fill(Qt::transparent);
    m_mainCanvas.fill(Qt::transparent);

    QList<Point_commondata> posList;

    for (int i = 0; i < data.size(); ++i) {
        if (i >= drawPts.size()) break;

        Point_commondata pos;
        pos.posX = drawPts[i].x();
        pos.posY = drawPts[i].y();
        pos.radius = m_radius;
        pos.count = convertData(data[i]); // 转换数据以获得绘制的数值

        posList.append(pos);

        // 执行渐变绘图
        drawAlpha(pos.posX, pos.posY, pos.count);
    }

    colorize();
    emit drawingCompleted(m_mainCanvas);
}

void DrawWorker::drawAlpha(int x, int y, int count)
{
    QRadialGradient gradient(x, y, m_radius);
    gradient.setColorAt(0, QColor(0, 0, 0, uchar((count / maxCount) * 255)));
    gradient.setColorAt(1, QColor(0, 0, 0, 0));

    QPainter painter(&m_alphaCanvas);
    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    painter.drawEllipse(QPoint(x, y), m_radius, m_radius);
}

void DrawWorker::colorize()
{
    for (int x = 0; x < m_width; ++x) {
        for (int y = 0; y < m_height; ++y) {
            int alpha = qAlpha(m_alphaCanvas.pixel(x, y));
            QColor color = m_palette->getColorAt(alpha / 255.0);
            color.setAlpha(m_opacity);
            m_mainCanvas.setPixel(x, y, color.rgba());
        }
    }
}


qreal DrawWorker::convertData(int value)
{
    if (0 < value && value <= 2170) {
        return -0.0854525 * value + 332.9351;
    } else if (2171 <= value && value <= 2380) {
        return -0.0174325 * value + 524.394462;
    } else if (2381 <= value && value <= 3780) {
        return -0.06659 * value + 265.145;
    } else if (3781 <= value && value <= 4102) {
        return -0.4414 * value + 180.954;
    }
    return 0;
}
