#include "coordinatewidget.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <vector>

// 计算坐标范围（根据点列表）
void CoordinateWidget::calculateRange() {
    if (m_points.isEmpty()) return;

    m_xMin = m_points.first().x();
    m_xMax = m_xMin;
    m_yMin = m_points.first().y();
    m_yMax = m_yMin;

    for (const QPointF& p : m_points) {
        if (p.x() < m_xMin) m_xMin = p.x();
        if (p.x() > m_xMax) m_xMax = p.x();
        if (p.y() < m_yMin) m_yMin = p.y();
        if (p.y() > m_yMax) m_yMax = p.y();
    }

    // 处理所有点在同一位置的情况（避免除零错误）
    if (qFuzzyIsNull(m_xMax - m_xMin)) {
        m_xMin -= 0.5;
        m_xMax += 0.5;
    }
    if (qFuzzyIsNull(m_yMax - m_yMin)) {
        m_yMin -= 0.5;
        m_yMax += 0.5;
    }
}

// 实际坐标 → 像素坐标转换
QPointF CoordinateWidget::toPixelCoordinate(const QPointF &point) const {
    qreal plotWidth = width() - 2 * m_margin;
    qreal plotHeight = height() - 2 * m_margin;

    qreal px = m_margin + (point.x() - m_xMin) * plotWidth / (m_xMax - m_xMin);
    qreal py = height() - m_margin - (point.y() - m_yMin) * plotHeight / (m_yMax - m_yMin);
    return QPointF(px, py);
}

CoordinateWidget::CoordinateWidget(QWidget *parent) : QWidget(parent) {
    setFixedSize(400, 400);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void CoordinateWidget::setPoints(const QList<QPointF>& points) {
    m_points = points;
    if (m_points.isEmpty()) return;

    calculateRange();  // 计算坐标范围
    update();          // 触发重绘
}

void CoordinateWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制白色背景
    //painter.fillRect(rect(), Qt::white);

    // 绘制坐标轴
    QPen axisPen(Qt::black, 1.5);
    painter.setPen(axisPen);
    painter.drawLine(m_margin, height() - m_margin, width() - m_margin, height() - m_margin);  // x轴
    painter.drawLine(m_margin, m_margin, m_margin, height() - m_margin);                      // y轴

    // 绘制点连线
    if (m_points.size() >= 2) {
        QPen linePen(Qt::blue, 2);
        painter.setPen(linePen);
        for (int i = 0; i < m_points.size() - 1; ++i) {
            QPointF pixelP1 = toPixelCoordinate(m_points[i]);
            QPointF pixelP2 = toPixelCoordinate(m_points[i + 1]);
            painter.drawLine(pixelP1, pixelP2);
        }
    }

    // 绘制点（红色圆点）
    QBrush pointBrush(Qt::red);
    painter.setBrush(pointBrush);
    painter.setPen(Qt::NoPen);
    for (const QPointF& p : m_points) {
        QPointF pixelP = toPixelCoordinate(p);
        painter.drawEllipse(pixelP, m_pointRadius, m_pointRadius);
    }
}

// 鼠标点击事件处理（显示原坐标）
void CoordinateWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {  // 仅响应左键点击
        QPointF mousePos = event->position();  // 鼠标点击的像素坐标

        // 遍历所有点，检测是否点击到点
        for (const QPointF& realPoint : m_points) {
            QPointF pixelPoint = toPixelCoordinate(realPoint);  // 点的像素坐标
            qreal distance = QLineF(mousePos, pixelPoint).length();  // 计算距离

            // 若距离小于2倍点半径（允许一定误差），显示坐标
            if (distance <= 2 * m_pointRadius) {
                // 格式化原坐标（保留4位小数）
                QString tooltipText = QString("(%1, %2)")
                                          .arg(realPoint.x(), 0, 'f', 4)
                                          .arg(realPoint.y(), 0, 'f', 4);
                // 显示工具提示（全局位置）
                QToolTip::showText(event->globalPosition().toPoint(), tooltipText, this);
                break;  // 找到第一个匹配点后退出循环
            }
        }
    }
}
