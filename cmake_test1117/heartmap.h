#ifndef HEATMAPPER_H
#define HEATMAPPER_H
#include "global.h"
#include"commondata.h"
#include <QVector>
#include <QList>
#include<QPainter>
#include<QString>

#include<QLabel>
#include"log.h"
class MainWindow;

QT_BEGIN_NAMESPACE
class QImage;
QT_END_NAMESPACE

class  GradientPalette;

class  QHEATMAP_DLL_EXPORT heatmapper :public QWidget
{
Q_OBJECT
public:

    // heatmapper(int radius, int opacity, QWidget *parent);
    heatmapper(int radius, int opacity, const QVector<QPoint> &drawPoints, int imgWidth, int imgHeight, QWidget *parent);

    virtual ~heatmapper();

    enum {
        CANVAS_WIDTH  = 221,
        CANVAS_HEIGHT = 881,
        DEFAULT_RADIUS = 8,
        DEFAULT_OPACITY = 160,
        DEFAULT_WIDTH = 255
    };

    qreal  _maxCount;
    int  x_num=40;
    int  y_num=120;
    int  m_rad=8;
    int  m_dis=20;
    Point_commondata Point;
    qreal palette_min=0.06;

    //固定宽高的演示
    int ImgWidth;
    int ImgHeight;
    //数据点
    QList<Point_commondata> _posList;
    void setPalette(GradientPalette *palette);
    int  getCount(int x, int y);
    void colorize(int x, int y);
    void colorize();
    virtual void drawAlpha(int x, int y, int count, bool colorize_now = true);
    void drawImage(QList<Point_commondata> posList,qreal maxNum);
    void drawPt(QVector<int>data);
    void intialPtSize(int x, int y, int dis, int rad);
    void drawText(int x, int y,int count);

    const int nx = 400;
    const int ny = 400;
    const int datax = 20;
    const int datay = 20;
    void precompute(QVector<int> data);
    std::vector<std::vector<double>> m_matrix;
    std::vector<std::vector<double>> m_matrix1;
    std::vector<std::vector<double>>  m_Weight;
    // QVector<QPoint> m_draw_pt;
    void calMatrix(); //计算插值
    void  m_calMatrix();//单线性插值
    QImage *mainCanvas_;
    double doubleLinear(int m, int n, int X, int Y, double V1, double V2, double V3, double V4);

    QList<Point_commondata> posList_; // 保存绘制数据--1030
protected:
    virtual void colorize(int left, int top, int right, int bottom);
    void redraw();

    void paintEvent(QPaintEvent *event) override;

public slots:
  void changePercent(QVector<double> die);

    void processAndDrawData(const QVector<double> &data, double maxCount); // 将 MainWindow::drawPt_160_2 的逻辑在此实现

private:
    int increase(int x, int y, int delta = 1);

    // 存储点频率的数组，大小和图像一样
    QVector<int> data_;
    // 用于存储渐变透明数据的图像副本
    QImage *alphaCanvas_;
    // 用于显示输出的图像

    // 调色板
    GradientPalette *palette_;
    // 半径
    int radius_;
    // 不透明度
    int opacity_;
    // 最大命名数
    qreal max_;
    // 图像宽度
    int width_;
    // 图像高度
    int height_;
    MainWindow* maindlg;

    QVector<QPoint> m_draw_pt;
    int ImgWidth_main;
    int ImgHeight_main;

    qreal calculatMaxCount(const QVector<double> &data);

};

#endif // HEATMAPPER_H
