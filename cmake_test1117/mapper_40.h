#ifndef HEATMAP_H
#define HEATMAP_H
#include<QWidget>
#include "commondata.h"
#include"gradientpalette.h"
#include<QElapsedTimer>
#include"globaldata.h"
class globaldata;
class GradientPalette;
class  heatMap :public QWidget
{
    Q_OBJECT
public:
    explicit  heatMap(int radius,  int width,int height,int maxcount,QWidget *parent=0);
    virtual ~heatMap();

    // 半径
    int m_radius;
    // 不透明度
    int m_opacity=255;
    // 图像宽度
    int m_width;
    // 图像高度
    int m_height;
    // 用于存储渐变透明数据的图像副本
    QImage *m_alphaCanvas;
    // 用于显示输出的图像
    QImage *m_mainCanvas;
public slots:
    void updateData(QList<Point_commondata> data);
    //void setColorGradient(const QList<QColor> &colors);

protected:
    void paintEvent(QPaintEvent *event) override;


private:
    void drawRadialGradient( const Point_commondata &center,QPainter& p);

    void colorize(int x, int y,int alp);
    void drawMap();
    // void colorize();
    QList<QColor> m_gradientColors;
    QImage m_buffer;
    //数据点
    QList<Point_commondata> m_points;
    float max_count=0;
 qreal palette_min_40=0.06;

    // 调色板
    GradientPalette *m_palette;
    enum {
        CANVAS_WIDTH  = 900,
        CANVAS_HEIGHT = 900,
        DEFAULT_RADIUS = 32,
        DEFAULT_OPACITY = 160,
        DEFAULT_WIDTH = 255
    };
private:
    QColor *m_colorCache;      // 颜色查找表
    int m_maxCount;            // 最大数据值
    QImage *m_offscreenBuffer;  // 离屏缓冲区
};

#endif // HEATMAP_H
