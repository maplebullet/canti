#include "heartmap.h"
#include <QImage>
#include <QColor>
#include <QPainter>
#include <QRadialGradient>
#include <QDebug>
#include "gradientpalette.h"

/*
 * 构造函数
 * @param image 用于显示输出的图像
 * @param radius 半径，决定了径向渐变的大小
 * @param opacity 不透明度，取值范围为[0, 100]
 */
// heatmapper::heatmapper(int radius, int opacity, QWidget *parent)
//     : radius_(radius), opacity_(opacity), max_(1),QWidget(parent)
heatmapper::heatmapper(int radius, int opacity, const QVector<QPoint> &drawPoints, int imgWidth, int imgHeight, QWidget *parent)
    : radius_(radius),
    opacity_(opacity),
    max_(1),
    QWidget(parent),
    m_draw_pt(drawPoints), // 保存绘图点坐标
    ImgWidth_main(imgWidth), // 保存图像宽度
    ImgHeight_main(imgHeight) // 保存图像高度
{
    palette_ = new GradientPalette(DEFAULT_WIDTH);
    palette_->setColorAt(palette_min,Qt::white);
    palette_->setColorAt(0.20, Qt::blue);
    palette_->setColorAt(0.35, Qt::cyan);
    palette_->setColorAt(0.60, Qt::green);
    palette_->setColorAt(0.75, Qt::yellow);
    palette_->setColorAt(0.90, Qt::red);

    // 初始化QImage对象，确保非空，避免导致绘图失败并引发分段错误--1030
    mainCanvas_= new QImage(ImgWidth_main, ImgHeight_main, QImage::Format_ARGB32);
    alphaCanvas_ = new QImage(ImgWidth_main, ImgHeight_main, QImage::Format_ARGB32);

    // 使用前检查是否为nullptr--1030
    if (mainCanvas_->isNull() || alphaCanvas_->isNull()) {
        qWarning() << "Failed to initialize QImages";
    } else {
        alphaCanvas_->fill(QColor(0, 0, 0, 0));
        mainCanvas_->fill(QColor(0, 0, 0, 0));
    }

    ImgWidth =  (x_num+1)*5;
    ImgHeight = (y_num+1)*7;
    //qDebug()<<ImgWidth<<ImgHeight<<"ImgHeight";
    width_ = ImgWidth_main;
    height_ = ImgHeight_main;
    data_.resize(ImgWidth_main * ImgHeight_main);
    data_.fill(0);

        // mapper_->setGeometry(70,70,850,260);
        // currentTime=new QElapsedTimer();
        // intialPtSize(120,120,20,20);
        // ImgWidth_main =  (x_num_main+1)*7;
        // ImgHeight_main = (y_num_main+1)*2;
}

heatmapper::~heatmapper()
{
    mainCanvas_->fill(Qt::transparent);
    alphaCanvas_->fill(Qt::transparent);
    _posList.clear();
}


/*
 * 增加指定坐标点的命中次数
 * @param x 横坐标
 * @param y 纵坐标
 * @param delta 增加的次数值
 * @return 返回更新后的命中次数
 */
int heatmapper::increase(int x, int y, int delta)
{
    int index = (y - 1) * width_ + (x - 1);
    data_[index] += delta;
    return data_[index];
}

void heatmapper::paintEvent(QPaintEvent*)
{
    QPainter p(this);

#ifdef QT_DEBUG
    qDebug() << "Long:" << this->height() << "Width:" << this->width();
#endif
    // p.setRenderHint(QPainter::Antialiasing);
    //
    //绘制一个黑色网格，便于查看渐变色
    // ImgWidth_main =  (x_num_main+1)*11; //300 121*11=1331
    // ImgHeight_main = (y_num_main+1)*7; //41*7=287
    // p.setPen(QPen(Qt::black,1));
    // for(int i=0;i<width()+11;i+=11){
    //     p.drawLine(i, 0, i, height());
    // }
    // for(int i=0;i<height()+7;i+=7){
    //     p.drawLine(0,i,width(),i);
    // }
    p.drawImage(0, 0, *alphaCanvas_);
    //
    p.drawImage(0, 0, *mainCanvas_);


    alphaCanvas_->fill(QColor(0, 0, 0, 0));
    mainCanvas_->fill(QColor(0, 0, 0, 0));
    LOG_DEBUG("进入步态绘图 Point_commondata posList_数据个数： "+QString::number(posList_.size()));
    for (const Point_commondata &pt : posList_) {
        drawAlpha(pt.posX, pt.posY, pt.count);
    }
}

/*
 * 添加一个数据点
 * @param x 横坐标
 * @param y 纵坐标
 */
//void heatmapper::addPoint(int x, int y)
//{
//    if (x <= 0 || y <= 0 || x > width_ || y > height_)
//        return;

//    int count = increase(x, y);

//    if (max_ < count) {
//        max_ = count;
//        redraw();

//        return;
//    }

//   // drawAlpha(x, y, count);
//}

/*
 * 设置调色板
 * @param palette 调色板对象指针
 */
void heatmapper::setPalette(GradientPalette *palette)
{
    Q_ASSERT(palette);

    if (palette)
        palette_ = palette;
}

/*
 * 获得指定点的命中次数
 */
int heatmapper::getCount(int x, int y)
{
    if (x < 0 || y < 0)
        return 0;
    return data_[(y - 1) * width_ + (x - 1)];
}


void heatmapper::processAndDrawData(const QVector<double> &data, double maxCount)
{

    LOG_DEBUG("进入步态绘图数据个数： "+QString::number(data.size()));
    if (data.size() != m_draw_pt.size()) {
        qDebug() << "Error: Data size and m_draw_pt size do not match!";
        //return;
    }

    QList<Point_commondata> posList;
    posList.reserve(data.size());

    // qreal maxCount = 0.0;
    // qreal maxCount = calculatMaxCount(data);
   LOG_DEBUG("步态绘图页面尺寸 长 宽： "+QString::number(ImgWidth_main)+" "+QString::number(ImgHeight_main));

    // 2. 生成并绘制4800个点
   //  const int pointsPerRow = 120;
   // const int pointsPercol = 40;
    const int spacing_x = 11;
    const int spacing_y = 7;
    qDebug()<<  posList.size()<<"posList.size";
    for (int i = 0; i < data.size(); i++) {
        Point_commondata pos;
        // 计算x坐标（列方向）：从右到左逆序填充
        // 列偏移量：i / 40（每列40个点，0~119对应120列）
        // 右到左列索引 = 总列数-1 - 列偏移量（119 - (i/40)）
        // 初始x=11（最右列起点），每左移一列增加spacing_x
        pos.posX = 11 + (119 - (i / 40)) * spacing_x;

        // 计算y坐标（行方向）：从上到下顺序填充
        // 行内偏移量：i % 40（0~39对应40行）
        // 初始y=7（最上行起点），每下移一行增加spacing_y
        pos.posY = 7 + (i % 40) * spacing_y;
        pos.radius = 12;       // 点的半径（固定值）
        pos.count = data[i];   // 点的数值（来自data数组）
       // qDebug()<<"坐标:"<<i<<pos.posX<<pos.posY;
        posList.push_back(pos); // 将点加入列表
    }

    // 调用绘制
    drawImage(posList, maxCount * 0.8);
}

qreal heatmapper::calculatMaxCount(const QVector<double> &data)
{
    qreal maxCount = 0.0;
    for (double value : data) {
        if (value > maxCount) {
            maxCount = value;
        }
    }
    return maxCount;
}

void heatmapper::drawImage(QList<Point_commondata> posList,qreal maxNum)
{
#ifdef QT_DEBUG
    qDebug() << "drawImage";
#endif

    _maxCount=maxNum;

    // 1030 注释以上内容
    posList_ = posList; // 保存绘制的数据
    update();  // 触发paintEvent以便绘制
}


void heatmapper::drawPt(QVector<int> data)
{

}

void heatmapper::intialPtSize(int x, int y, int dis, int rad)
{

    QVector<QPoint> draw_pt;
    int n_x_sum = 0;
    int n_y_sum = 0;
    for (int i = 0; i < x; i++) {
       // Point mid_pt;
        for (int j = 0; j < y; j++) {
            draw_pt.push_back(QPoint(j*20, i*20));
            n_x_sum += j*20;
            n_y_sum += i*20;
        }
    }
    float n_x_ = n_x_sum / (x*y);
    float n_y_ = n_y_sum / (x*y);
    for (int i = 0; i < draw_pt.size(); i++) {
        draw_pt[i].setX(draw_pt[i].x() - n_x_);
        draw_pt[i].setY(draw_pt[i].y() - n_y_);
    }
    m_draw_pt = draw_pt; //m_draw_pt：对应坐标点
}

void heatmapper::drawText(int x, int y, int count)
{
    QPainter textPainter(this);

   textPainter.setPen(QPen(Qt::black,1));


   if(count!=0){

       QString str_text=QString::number(count);
       textPainter.drawText(x+5,y-5,str_text);
   }

}


void heatmapper::precompute(QVector<int> data)
{
    //计算权重
     qDebug()<<data<<"m_data";
    m_matrix1 = std::vector<std::vector<double>>(datax, std::vector<double>(datay));
    m_matrix = std::vector<std::vector<double>>(nx, std::vector<double>(ny));
    m_Weight=std::vector<std::vector<double>>(datax, std::vector<double>(datay));
    int maxCount=0;


for(int i=0;i<data.count();i++){
if(data[i]>maxCount) maxCount=data[i];
}
//const double max_count=maxCount;
//qDebug()<<maxCount<<minCount<<"最值";

    //开始计算
   // double x, y;
    //缩放系数
    int n=0;
    //int ratex = nx/datax, ratey = ny/datay;
    for(int i = 0; i < datax; i++){
        for(int j = 0; j < datay; j++){
           //  m_pColorMap->data()->cellToCoord(i * 0.05, j * 0.05, &x, &y);
             if(n<400)
             {
                 m_Weight[i][j]=data[n];
               // qDebug()<<i<<j<<m_Weight[i][j]<<data[n]<<n<<"data";
               n++;
             }
        }
    }
   //  qDebug()<<m_Weight<<data<<"weight";
   //  qDebug()<<m_pColorMap->data<<"data";
    calMatrix();
    // m_calMatrix();
}

void heatmapper::calMatrix()
{
    alphaCanvas_->fill(QColor(0, 0, 0, 0));
    mainCanvas_->fill(QColor(0, 0, 0, 0));
    //缩放系数
    int ratex = nx/datax, ratey = ny/datay;
    //int ratex=ImgWidth/datax , ratey=ImgHeight/datay;
    double x,y;
    for (int i=0;i<datax;++i) {
        for (int j=0;j<datay;++j) {
          // m_pColorMap->data()->cellToCoord(i * ratex, j * ratey, &x, &y);
             m_matrix1[i][j]=m_Weight[i][j];
           //  qDebug()<<m_matrix1[i][j]<<i<<j<<"m_matrix1[i][j]";
        }

    }
    //50*50 -> 200 * 200 双线性插值
    for (int i = 0; i < datax-1 ; i++)
    {
        for (int j = 0; j < datay-1 ; j++)
        {
            double V1 = m_matrix1[i][j];
            double V2 = m_matrix1[i + 1][j];
            double V3 = m_matrix1[i + 1][j + 1];
            double V4 = m_matrix1[i][j + 1];
            for (int m = 0; m < ratex; m++)
            {
                for (int n = 0; n < ratey; n++)
                {
                    //int x = (i+0.5) * 0.05 + m-0.5, y = (j+0.5) * ratey + n-0.5;
                     int x = (i) * ratex+m , y = (j) * ratey+n ;
                     m_matrix[x][y] = doubleLinear(m, n, ratex, ratey, V1, V2, V3, V4);
                 //    m_pColorMap->data()->setCell(x, y, m_matrix[x][y]);
                      drawAlpha(y, x, m_matrix[x][y]);
                     qDebug()<<x<<y<<m_matrix[x][y]<<"m_matrix";

              }
            }
        }

    }


}

void heatmapper::m_calMatrix()
{
    alphaCanvas_->fill(QColor(0, 0, 0, 0));
    mainCanvas_->fill(QColor(0, 0, 0, 0));
    for (int i = 0; i < datax-1 ; i++)
    {
        for (int j = 0; j < datay-1 ; j++)
        {
            double V1 =_posList[i+20*j].count;
            double V2 = _posList[i+20*j+1 ].count;
            double V3 = _posList[i+20*(j+1)+1].count;
            double V4 = _posList[i+20*(j+1)].count;
            for(int n=0;n<40;n++){
                for(int m=0;m<40;m++){
                    int x=_posList[i+20*j].posX+2*n;
                    int y=_posList[i+20*j].posY+2*m;
                      int m_count=doubleLinear(m, n, 20, 20, V1, V2, V3, V4);
                     drawAlpha(x, y, m_count);
                }


            }
        }
    }
}

double heatmapper::doubleLinear(int m, int n, int X, int Y, double V1, double V2, double V3, double V4)
{
   return (m * n * (V3 - V4 - V2 + V1) + X * n * (V4 - V1) + m * Y * (V2 - V1)) / (X * Y) + V1;
}

/*
 * 重载方法，着色
 */
void heatmapper::colorize()
{
    colorize(0, 0, width_, height_);
}


/*
 * 绘制透明径向渐变
 * @param x 横坐标
 * @param y 纵坐标
 * @param count 被命中次数
 * @param colorize_now 是否调用着色方法
 */
void heatmapper::drawAlpha(int x, int y, int count, bool colorize_now)
{
    // 使用前检查，避免为空时使用QPainter--1030
    if (!alphaCanvas_ || alphaCanvas_->isNull()) {
        qWarning() << "alphaCanvas_ is not initialized!";
        return;
    }

    const double max_count=_maxCount;
    const uchar alpha=uchar((count/max_count)*255);
    //文字

    QRadialGradient gradient(x, y, radius_);
    gradient.setColorAt(0, QColor(0, 0, 0, alpha));
    gradient.setColorAt(1, QColor(0, 0, 0, 0));
    //QPainter  textPainter(mainCanvas_);
     QPainter painter(alphaCanvas_);
    QPen pen;
     pen.setBrush(Qt::NoBrush);
     pen.setColor(Qt::black);
     pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    painter.drawEllipse(QPoint(x, y), 15, 25);
    if (colorize_now)
        colorize(x, y);
}

/*
 * 重载方法，着色
 * @param x 横坐标
 * @param y 纵坐标
 * @param subArea 透明径向渐变区域
 */
void heatmapper::colorize(int x, int y)
{
    int left = x - radius_;
    int top = y - radius_;
    int right = x + radius_;
    int bottom = y + radius_;


    if (left < 0)
        left = 0;

    if (top < 0)
        top = 0;

    if (right > width_)
        right = width_;

    if (bottom > height_)
        bottom = height_;

    colorize(left, top, right, bottom);
}

/*
 * 重载函数，实际的着色操作在本方法
 * @param left   左上角横坐标
 * @param top    左上角纵坐标
 * @param right  右下角横坐标
 * @param bottom 右下角纵坐标
 */
void heatmapper::colorize(int left, int top, int right, int bottom)
{
    int alpha = 0;
    int finalAlpha = 0;
    QColor color;
    for (int i = left; i < right; ++i) {
        for (int j = top; j < bottom; ++j) {
            alpha = qAlpha(alphaCanvas_->pixel(i, j));
            if (!alpha)
                continue;
            finalAlpha = (alpha < opacity_ ? alpha : opacity_);
            color = palette_->getColorAt(alpha);
            mainCanvas_->setPixel(i, j, qRgba(color.red(),
                                              color.green(),
                                              color.blue(),
                                              finalAlpha));
//            QPainter painter;
//            painter.drawImage(0, 0, *mainCanvas_);
        }
    }

}

void heatmapper::redraw()
{
    QColor color(0, 0, 0, 0);
    alphaCanvas_->fill(color);
    mainCanvas_->fill(color);

    int size = data_.size();
    for (int i = 0; i < size; ++i) {
        if (0 == data_[i])
            continue;
        drawAlpha(i % width_ + 1, i / width_ + 1, data_[i], false);
    }
    colorize();
}

void heatmapper::changePercent(QVector<double> num)
{
    // trimIndex=num.at(0)/100;
    // endTrimIndex=num.at(1)/100;
    // qDebug()<<trimIndex<<"1112"<<endTrimIndex;
    // this->update();
}
