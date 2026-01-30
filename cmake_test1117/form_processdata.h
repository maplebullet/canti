#ifndef FORM_PROCESSDATA_H
#define FORM_PROCESSDATA_H
#include "process.h"
#include <QWidget>
#include <QLabel>
#include"ellipsewidget.h"
#include"coordinatewidget.h"
#include<QDateTime>
#include<QFile>
#include<QPdfWriter>
#include<QPainter>
#include<QMouseEvent>
#include "log.h"
class CoordinateWidget;
class EllipseWidget;

namespace Ui {
class Form_processdata;
}

class Form_processdata : public QWidget
{
    Q_OBJECT

public:
    explicit Form_processdata(QWidget *parent = nullptr);
    ~Form_processdata();

    void readProcessdata(Jingtai jingtaiResult);
    QImage Mat2QImage(cv::Mat& image);
    void setLabelImage(QLabel* label, const cv::Mat& image);


    void recordReportJingtai(); //构建类
    void paintPdfJingtai(QPainter *pPainter); //绘制足底压力报告
    void drawTargetEllipse(QPainter &painter, const QPointF &center,
                           double longRadius, double shortRadius, double tiltAngle);

private:
    bool LeftBorderHit(const QPoint &pos);
    bool RightBorderHit(const QPoint &pos);
    bool TopBorderHit(const QPoint &pos);
    bool BottomBorderHit(const QPoint &pos);

signals:
    void hideAndShowMain_process();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void checkBorderDragging(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent* ev);
private slots:
    void on_main_return_clicked();
    void on_main_restore_clicked();

    void on_main_min_clicked();

    void on_main_max_clicked();

    void on_main_close_clicked();

private:
    Ui::Form_processdata *ui;
    EllipseWidget *left_ellipseWidget; // 自定义椭圆Widget指针
    EllipseWidget *right_ellipseWidget; // 自定义椭圆Widget指针
    EllipseWidget *left_right_ellipseWidget; // 自定义椭圆Widget指针
    CoordinateWidget* zhongxin_coordinateWidget;


    QPoint mousePoint;
    bool mouse_press;
    QPoint m_mousePos;
    QPoint m_wndPos;
    bool m_bMousePressedTitle = false;
    bool m_bMousePressed = false;
    bool m_bDragTop = false;
    bool m_bDragLeft = false;
    bool m_bDragRight = false;
    bool m_bDragBottom = false;
    QRect m_StartGeometry;
    const int CONST_DRAG_BORDER_SIZE = 5;

    Jingtai m_processResult;

};



#endif // FORM_PROCESSDATA_H
