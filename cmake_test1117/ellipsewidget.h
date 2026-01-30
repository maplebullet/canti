#ifndef ELLIPSEWIDGET_H
#define ELLIPSEWIDGET_H

#include <QWidget>
#include <qmath.h>


class EllipseWidget : public QWidget {
    Q_OBJECT
public:
    explicit EllipseWidget(QWidget *parent = nullptr);

    // 设置椭圆参数：长轴a、短轴b、倾斜角（角度制）、中心坐标(h,k)
    void setEllipseParams(double a, double b, double thetaDeg, double h, double k,double liXInlv);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_a = 0;       // 长轴半径
    double m_b = 0;       // 短轴半径
    double m_theta = 0;   // 倾斜角（弧度）
    double m_h = 200;     // 中心X坐标（默认居中）
    double m_k = 200;     // 中心Y坐标（默认居中）
    double m_liXinlv=0;
};

#endif // ELLIPSEWIDGET_H
