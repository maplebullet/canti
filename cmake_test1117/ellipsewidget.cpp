#include "ellipsewidget.h"
#include <QPainter>
#include <QPen>
#include <cmath> // 添加此头文件以使用 M_PI
EllipseWidget::EllipseWidget(QWidget *parent) : QWidget(parent) {
    setFixedSize(450, 450); // 固定Widget大小为450x450
}

// 设置椭圆参数并触发重绘
void EllipseWidget::setEllipseParams(double a, double b, double thetaDeg, double h, double k,double liXInlv) {
    if (a <= 0 || b <= 0) return; // 长轴/短轴需为正数
    m_a = a;
    m_b = b;
    m_theta = qDegreesToRadians(thetaDeg); // 角度转弧度
    m_h = qBound(0.0, h, 400.0);           // 限制中心X在Widget范围内
    m_k = qBound(0.0, k, 400.0);           // 限制中心Y在Widget范围内
    m_liXinlv=liXInlv;
    qDebug()<<m_a<<m_b<<m_theta<<m_h<<m_k<<m_liXinlv<<"触发paintEvent重绘";
    update(); // 触发paintEvent重绘
}

void EllipseWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿优化

    // 获取窗口尺寸
    int width = this->width();
    int height = this->height();
    qDebug()<<width<<height<<"椭圆窗口宽、长";
    int centerX = width / 2;  // 窗口中心X
    int centerY = height / 2; // 窗口中心Y

    // --------------------------
    // 1. 绘制黑色十字交叉线（固定中心）
    // --------------------------
    painter.save();
    QPen crossPen(Qt::black, 2); // 黑色，线宽2
    crossPen.setCapStyle(Qt::RoundCap);
    painter.setPen(crossPen);
    // 水平线
    painter.drawLine(50, centerY,width-50, centerY);
    // 垂直线
    painter.drawLine(centerX, 50, centerX, height-50);
    painter.restore();


    // --------------------------
    // 2. 绘制黑色参考圆（固定中心，直径400）
    // --------------------------
    painter.save();
    QPen basePen(Qt::black, 2); // 黑色边框，线宽2
    painter.setPen(basePen);
    painter.setBrush(Qt::NoBrush);
    // 参考圆：中心在窗口中心，直径220
    int baseCircleRadius = 110;
    painter.drawEllipse(centerX - baseCircleRadius,
                        centerY - baseCircleRadius,
                        2 * baseCircleRadius,
                        2 * baseCircleRadius);
    painter.restore();


    // --------------------------
    // 3. 绘制黑色中心圆点（固定中心）
    // --------------------------
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black); // 黑色填充
    int dotSize = 20; // 圆点直径
    painter.drawEllipse(centerX - dotSize/2,
                        centerY - dotSize/2,
                        dotSize, dotSize);
    painter.restore();


    // --------------------------
    // 4. 绘制红色旋转椭圆（动态参数，中心对齐窗口中心）
    // --------------------------
    painter.save();
    // 平移到窗口中心（椭圆中心与窗口中心对齐）
    painter.translate(centerX, centerY);
    // 旋转椭圆（弧度转角度）
    painter.rotate(m_theta * 180 / M_PI);

    QPen rotatePen(Qt::red, 3); // 红色边框，线宽3
    painter.setPen(rotatePen);
    painter.setBrush(Qt::NoBrush);
    // 绘制椭圆：长半轴m_a，短半轴m_b（外接矩形）
    painter.drawEllipse(QRectF(-m_a, -m_b, 2*m_a, 2*m_b));
    painter.restore();


    // --------------------------
    // 5. 绘制底部文字（重心偏移，黑色）
    // --------------------------
    painter.save();
    painter.setFont(QFont("微软雅黑", 12, QFont::Bold)); // 适配中文
    painter.setPen(Qt::black); // 黑色文字

    QString offsetText = QString("重心偏移:%1").arg(m_liXinlv, 0, 'f', 1);
    QRect textRect(0, height-30, width, 20);
    // 文字位置：底部居中（与目标图一致）
    painter.drawText(textRect, Qt::AlignCenter | Qt::AlignBottom, offsetText);
    painter.restore();


}
