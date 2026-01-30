#ifndef COORDINATEWIDGET_H
#define COORDINATEWIDGET_H

#include <QWidget>
#include <QList>
#include <QPointF>
#include <QMouseEvent>  // 包含鼠标事件头文件
#include <QToolTip>     // 包含工具提示头文件

class CoordinateWidget : public QWidget {
    Q_OBJECT
public:
    explicit CoordinateWidget(QWidget *parent = nullptr);

    void setPoints(const QList<QPointF>& points);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;  // 声明鼠标按下事件

private:
    QList<QPointF> m_points;
    qreal m_xMin = 0.0;
    qreal m_xMax = 0.0;
    qreal m_yMin = 0.0;
    qreal m_yMax = 0.0;
    const qreal m_margin = 20.0;
    const qreal m_pointRadius = 3.0;  // 点的显示半径（用于点击检测）

    void calculateRange();  // 计算坐标范围（需实现）
    QPointF toPixelCoordinate(const QPointF &point) const;  // 实际坐标转像素坐标（需实现）
};

#endif // COORDINATEWIDGET_H
