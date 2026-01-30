#include "form_processdata.h"
#include "ui_form_processdata.h"
#include"jingtai.h"
#include<QDebug>
#include"globaldata.h"
#include <algorithm>
#include<QMessageBox>

Form_processdata::Form_processdata(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form_processdata)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, false); // 因为我们手动管理生命周期，不自动 delete
    setWindowFlag(Qt::FramelessWindowHint);  //取消默认功能
    QApplication::instance()->installEventFilter(this);
    ui->main_restore->setVisible(true);
    ui->main_max->setVisible(false);
}

Form_processdata::~Form_processdata()
{
    delete ui;
}

void Form_processdata::readProcessdata(Jingtai result)
{
    m_processResult=result;


    // 打印重心拟合椭圆信息

    LOG_DEBUG("双足重心拟合椭圆信息 Ellipse (a, b, angle, area, eccentricity)"+QString::number(result.ellipse.a)
              +" "+QString::number(result.ellipse.a)
              +" "+QString::number(result.ellipse.b)
              +" "+QString::number(result.ellipse.angle)
              +" "+QString::number(result.ellipse.area)
              +" "+QString::number(result.ellipse.eccentricity));


    //左脚

    ui->lineEdit_25->setText(QString::number(result.left.zugongzhishu,'f',2));
    ui->lineEdit_8->setText(QString::number(result.left.pressure,'f',2) + " KPa"); //平均压力
    ui->lineEdit_23->setText(QString::number(result.left.area,'f',2) + " cm²");//平均面积
    ui->lineEdit_15->setText(QString::number(result.left.nei_wai,'f',2) + " %"); //左右占比
    ui->lineEdit_16->setText(QString::number(result.left.up_back,'f',2) + " %"); //前后占比
    ui->lineEdit_24->setText(QString::number(result.left.center_width,'f',2) + " pt");
    ui->lineEdit_26->setText(QString::number(result.left.center_hight,'f',2) + " pt");
    cv::Vec2f fft0 = result.left.ffts[0];
    QString fftString0 = QString("%1, %2").arg(fft0[0]).arg(fft0[1]);
    ui->lineEdit_48->setText(fftString0);
    ui->lineEdit_mianji->setText(QString::number(result.left.ellipse.area,'f',2) + " cm²"); //静止COP摆动面积：
    left_ellipseWidget=new EllipseWidget(ui->widget_left);
    //left_ellipseWidget->setGeometry(0, 0, 500, 500);

    // 设置椭圆参数（示例值）
    double longRadius = result.left.ellipse.a;   // 长轴半径（半长轴）
    double shortRadius = result.left.ellipse.b;   // 短轴半径（半短轴）
    double tiltAngle = result.left.ellipse.angle;     // 倾斜角（度数，逆时针45°）
    QPointF center(200, 200);    // 圆心（400x400 Widget的中心）

    // 确定当前半径的最小值和最大值（宽高比相关）
    double minR = std::min(longRadius, shortRadius);
    double maxR = std::max(longRadius, shortRadius);
    // 计算两个关键缩放因子：
    double scaleFactorMin = 50.0 / minR;
    double scaleFactorMax = 100.0 / maxR;

    // 最终缩放因子取较小值
    double scaleFactor = std::min(scaleFactorMax, scaleFactorMin);

    longRadius=longRadius*scaleFactor;
    shortRadius=shortRadius*scaleFactor;

    LOG_DEBUG("左脚重心拟合椭圆信息 Ellipse (a长轴半径, b短轴半径, angle倾斜角, area面积, 离心率)"+QString::number(result.ellipse.a)+" "
              +QString::number(result.left.ellipse.a)
              +" "+QString::number(result.left.ellipse.b)
              +" "+QString::number(result.left.ellipse.angle)
              +" "+QString::number(result.left.ellipse.area)
              +" " +QString::number(result.left.ellipse.eccentricity)
              +" "+QString::number(result.left.ellipse.center.x)
              +" "+QString::number(result.left.ellipse.center.y));
    // 传递参数并触发绘制
    left_ellipseWidget->setEllipseParams(longRadius, shortRadius, tiltAngle, center.x(),center.y(),result.left.ellipse.eccentricity);

    //右脚
    ui->lineEdit_38->setText(QString::number(result.right.zugongzhishu,'f',2));
    ui->lineEdit_27->setText(QString::number(result.right.pressure,'f',2) + " KPa");
    ui->lineEdit_36->setText(QString::number(result.right.area,'f',2) + " cm²");
    ui->lineEdit_28->setText(QString::number(result.right.nei_wai,'f',2) + " %");
    ui->lineEdit_29->setText(QString::number(result.right.up_back,'f',2) + " %");
    ui->lineEdit_37->setText(QString::number(result.right.center_width,'f',2) + " pt");
    ui->lineEdit_39->setText(QString::number(result.right.center_hight,'f',2) + " pt");
    cv::Vec2f fft1 = result.right.ffts[0];
    QString fftString1 = QString("%1, %2").arg(fft1[0]).arg(fft1[1]);
    ui->lineEdit_49->setText(fftString1);

    //拟合椭圆

    right_ellipseWidget=new EllipseWidget(ui->widget_2);
    // 设置椭圆参数（示例值）
    double longRadius_right = result.right.ellipse.a;   // 长轴半径（半长轴）
    double shortRadius_right = result.right.ellipse.b;   // 短轴半径（半短轴）
    double tiltAngle_right = result.right.ellipse.angle;     // 倾斜角（度数，逆时针45°）
    QPointF center_right(200, 200);    // 圆心（400x400 Widget的中心）

    LOG_DEBUG("右脚重心拟合椭圆信息 Ellipse (a, b, angle, area, eccentricity)"+QString::number(result.ellipse.a)
              +QString::number(result.right.ellipse.a)
              +QString::number(result.right.ellipse.b)
              +QString::number(result.right.ellipse.angle)
              +QString::number(result.right.ellipse.area)
              +QString::number(result.right.ellipse.eccentricity));
    // 确定当前半径的最小值和最大值（宽高比相关）
    double minR_right = std::min(longRadius_right, shortRadius_right);
    double maxR_right = std::max(longRadius_right, shortRadius_right);
    // 计算两个关键缩放因子：
    double scaleFactorMin_right = 50.0 / minR_right;
    double scaleFactorMax_right = 100.0 / maxR_right;

    // 最终缩放因子取较小值
    double scaleFactor_right = std::min(scaleFactorMax_right, scaleFactorMin_right);

    longRadius_right=longRadius_right*scaleFactor_right;
    shortRadius_right=shortRadius_right*scaleFactor_right;


    // 传递参数并触发绘制
    right_ellipseWidget->setEllipseParams(longRadius_right, shortRadius_right, tiltAngle_right, center_right.x(),center_right.y(),result.right.ellipse.eccentricity);


    //左右足数据
    QString zhongxin="";

    QList<QPointF> points;
    points.clear();
    for(int  i=0;i<result.centers.size();i++){
        float x_zhongxin=result.centers[i].x;
        float y_zhongxin=result.centers[i].y;
        QPointF point(x_zhongxin,y_zhongxin);
        points.append(point);

    }

    // 创建水平布局并添加到 widget_m
    QHBoxLayout *layout = new QHBoxLayout(ui->widget_zhongxin);
    layout->setContentsMargins(0, 0, 0, 0); // 布局边距
    layout->setSpacing(20); // 子部件间距
    zhongxin_coordinateWidget=new CoordinateWidget(ui->widget_zhongxin);
    layout->addWidget(zhongxin_coordinateWidget);
    layout->setAlignment(Qt::AlignCenter);
    zhongxin_coordinateWidget->setGeometry(0, 0, 450, 450); // 显示在主窗口左上角
    zhongxin_coordinateWidget->setPoints(points);

    //拟合椭圆

    left_right_ellipseWidget =new EllipseWidget(ui->widget_left_right);
    // 设置椭圆参数（示例值）
    double longRadius_right_left = result.ellipse.a;   // 长轴半径（半长轴）
    double shortRadius_right_left =result.ellipse.b;   // 短轴半径（半短轴）
    double tiltAngle_right_left = result.ellipse.angle;     // 倾斜角（度数，逆时针45°）
    QPointF center_right_left(200, 200);    // 圆心（400x400 Widget的中心）


    // 确定当前半径的最小值和最大值（宽高比相关）
    double minR_right_left = std::min(longRadius_right_left, shortRadius_right_left);
    double maxR_right_left = std::max(longRadius_right_left, shortRadius_right_left);
    // 计算两个关键缩放因子：
    double scaleFactorMin_right_left = 50.0 / minR_right_left;
    double scaleFactorMax_right_left = 100.0 / maxR_right_left;

    // 最终缩放因子取较小值
    double scaleFactor_right_left = std::min(scaleFactorMax_right_left, scaleFactorMin_right_left);

    longRadius_right_left=longRadius_right_left*scaleFactor_right_left;
    shortRadius_right_left=shortRadius_right_left*scaleFactor_right_left;

    // 传递参数并触发绘制
    left_right_ellipseWidget->setEllipseParams(longRadius_right_left, shortRadius_right_left, tiltAngle_right_left, center.x(),center.y(),result.ellipse.eccentricity);



}


QImage Form_processdata::Mat2QImage(cv::Mat& image)
{
    if (image.empty()) {
        qDebug() << "Error: Mat is empty.";
        return QImage();
    }

    // 如果类型不是支持的类型，进行归一化和转换
    if (image.type() != CV_8UC1 && image.type() != CV_8UC3) {
        qDebug() << "Converting Mat type:" << image.type();
        cv::Mat normalized;
        cv::normalize(image, normalized, 0, 255, cv::NORM_MINMAX); // 归一化
        normalized.convertTo(image, CV_8U); // 转化为 8-bit
    }

    // 对单通道灰度图应用伪彩色映射
    if (image.type() == CV_8UC1) {
        cv::Mat coloredImage;
        cv::applyColorMap(image, coloredImage, cv::COLORMAP_JET); // 使用 JET 伪彩色
        image = coloredImage; // 将伪彩色图像覆盖到 image
    }

    QImage img;
    if (image.type() == CV_8UC3) { // 3通道 BGR
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        img = QImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
    }
    else {
        qDebug() << "Unsupported Mat type:" << image.type();
        img = QImage();
    }

    return img;
}

// 用于更新图像到 QLabel 控件
void Form_processdata::setLabelImage(QLabel* label, const cv::Mat& image) {
    if (image.empty()) {
        qDebug() << "Error: Image is empty.";
        return;
    }

    QImage img = Mat2QImage(const_cast<cv::Mat&>(image));
    if (img.isNull()) {
        qDebug() << "Error: Failed to convert Mat to QImage.";
        return;
    }

    // 设置 QLabel 的对齐方式为居中，蓝色背景#00007B
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("QLabel { background-color: #00007B; }");

    // 设置 QLabel 的图像，按控件大小保持比例自适应，并确保缩放后图像质量更高
    label->setPixmap(QPixmap::fromImage(img).scaled(label->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}

void Form_processdata::on_main_restore_clicked()
{
    ui->main_restore->setVisible(false);
    ui->main_max->setVisible(true);
    setWindowState(Qt::WindowNoState);
    this->update();
}

void Form_processdata::on_main_min_clicked()
{
    this->showMinimized();
    this->update();
}

void Form_processdata::on_main_max_clicked()
{
    ui->main_restore->setVisible(true);
    ui->main_max->setVisible(false);
    this->setWindowState(Qt::WindowMaximized);
    this->showMaximized();
    setCursor(Qt::ArrowCursor);
    this->update();
}

void Form_processdata::on_main_close_clicked()
{
    qDebug()<<"close";
    emit hideAndShowMain_process();
    this->close();

}

bool Form_processdata::LeftBorderHit(const QPoint &pos)
{
    const QRect& rect = this->geometry();
    if(pos.x() >= rect.x() && pos.x() <= (rect.x() + CONST_DRAG_BORDER_SIZE)){
        return true;
    }

    return false;
}

bool Form_processdata::RightBorderHit(const QPoint &pos)
{
    const QRect& rect = this->geometry();
    int nPosX = rect.x()+rect.width();
    if(pos.x() <= nPosX && pos.x() >= (nPosX - CONST_DRAG_BORDER_SIZE)){
        return true;
    }

    return false;
}

bool Form_processdata::TopBorderHit(const QPoint &pos)
{
    const QRect& rect = this->geometry();
    if(pos.y() > rect.y() && pos.y() <= (rect.y()+CONST_DRAG_BORDER_SIZE)){
        return true;
    }

    return false;
}

bool Form_processdata::BottomBorderHit(const QPoint &pos)
{
    const QRect& rect = this->geometry();
    int nPosY =rect.y()+rect.height();
    if(pos.y() < nPosY && pos.y() >= (nPosY-CONST_DRAG_BORDER_SIZE)){
        return true;
    }

    return false;
}

void Form_processdata::recordReportJingtai()
{
    //1、生成文件名   格式"2000-01-02_hh_mm_ss_Jingtai"
    QString fileName;

    QString strdate=QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss")+"_Jingtai";

    //在debug或release下的pdfadress文件夹内
    globaldata::reportAdress=QCoreApplication::applicationDirPath()+"/pdfadress";
    fileName =globaldata::reportAdress+"/"+strdate+".pdf";
    globaldata::reportPath_jingtai=fileName;

    LOG_DEBUG("该足底压力报告文件名："+fileName);
    //2、 创建pdf文件
    QFile pdfFile(fileName);
    pdfFile.open(QIODevice::WriteOnly);
    //3、创建生成pdf类，作为绘图设备
    QPdfWriter pPdfWriter (&pdfFile);
    pPdfWriter.setResolution(300);
    pPdfWriter.setPageSize(QPageSize::A4);
    pPdfWriter.setPageMargins(QMarginsF(0, 0, 0, 0));




    // ========== 关键修复：分开展示两页绘制逻辑 ==========
    // 3. 核心：全程使用一个Painter
    QPainter painter;
    if (!painter.begin(&pPdfWriter)) {
        qDebug() << "Painter初始化失败";
        pdfFile.close();
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);


    //4、绘制报告内容
    paintPdfJingtai(&painter);

    // 4. 结束绘制（必须最后调用）
    painter.end();
    pdfFile.close();
}

void Form_processdata::paintPdfJingtai(QPainter *pPainter)
{

    // ========== 修复1：创建Painter并绑定设备（必须调用begin()） ==========
    if (!pPainter || !pPainter->isActive()) {
        qDebug() << "第一页Painter未激活";
        return;
    }

    int nPdfWidth = pPainter->viewport().width();
    int nPdfHeight = pPainter->viewport().height();
    LOG_DEBUG("进入生成足底压力报告"+QString::number(nPdfWidth)+" "+QString::number(nPdfHeight));

    QFont font;
    font.setFamily("Adobe Heiti Std, Adobe Heiti Std-R");
    int DPI_1 = 300;
    int DPI_2 = 72;

    int y = DPI_1 * 28 / DPI_2;
    int x = DPI_1 * 22 / DPI_2;
    int nColumnSpace = 720;
    int nLineHeight = 72;
    int nLineSpace = 100;
    //检测日期
    //获取当前时间
    QDateTime datatime=QDateTime::currentDateTime();
    QString  strdate=datatime.toString("yyyy-MM-dd hh:mm:ss");

    pPainter->drawText(QRect(x,y,nColumnSpace,nLineHeight),Qt::AlignLeft|Qt::AlignVCenter,QString("检测日期：%1").arg(strdate));
    //设置标题的绘制位置  根据慕客报告布局内容
    x = DPI_1 * 425 / DPI_2;
    //检测编号
    strdate=datatime.toString("yyyyMMdd");
    // pPainter->drawText(QRect(x,y,nColumnSpace,nLineHeight),Qt::AlignLeft|Qt::AlignVCenter,QString("检测编号:%1").arg(strdate)+"0001");
    pPainter->drawText(QRect(x,y,nColumnSpace,nLineHeight),Qt::AlignLeft|Qt::AlignVCenter,QString("编号:%1").arg(globaldata::user_id));
    y=287;

    //绘制标题
    pPainter->setFont(QFont("Adobe Heiti Std, Adobe Heiti Std-R", 24, 36));
    pPainter->drawText(QRect(0, y, nPdfWidth, 100), Qt::AlignCenter, tr("足部健康报告"));
    //qDebug()<<nPdfWidth<<nPdfHeight<<"报告宽度";
    pPainter->setRenderHint(QPainter::Antialiasing, true); // 现在Painter已激活，可设置抗锯齿


    font.setPointSize(12);
    pPainter->setFont(font);
    x = DPI_1 * 43 / DPI_2;
    y = DPI_1 * 132 / DPI_2;

    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("姓   名：   %1").arg(globaldata::user_name));
    x = DPI_1 * 233 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("性   别：   %1").arg(globaldata::user_gender));
    x = DPI_1 * 403 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("年   龄：   %1").arg(globaldata::user_age));

    x = DPI_1 * 43 / DPI_2;
    y = DPI_1 * 154 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("身高/cm：   %1").arg(globaldata::user_height));
    x = DPI_1 * 233 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("体重/kg：   %1").arg(globaldata::user_weight));
    x = DPI_1 * 403 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("BMI: %1").arg(globaldata::user_bmi, 0, 'f', 1));



    x = DPI_1 * 24 / DPI_2;
    y = DPI_1 * 188 / DPI_2;

    QImage image_9("://new//prefix1//icon//26.png");
    image_9 = image_9.scaled(image_9.width(), image_9.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pPainter->drawImage(x, y, image_9);


    // 左脚数据
    x = DPI_1 * 49 / DPI_2;
    y = DPI_1 * 197 / DPI_2;
    //  qDebug()<<x<<y<<"左脚数据位置";
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, tr("左脚"));

    y = DPI_1 * 218 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("脚宽：%1 cm").arg(QString::number(m_processResult.left.jiaokuan,'f',2)));
    y = DPI_1 * 237 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("脚长：%1 cm").arg(QString::number(m_processResult.left.jiaochang,'f',2)));
    y = DPI_1 * 277 / DPI_2;
    QString zugongType_left;
    if(m_processResult.left.zugongtype==-1){
        zugongType_left="高弓足";
    }  else  if(m_processResult.left.zugongtype==0){
        zugongType_left="正常足";
    }
    else  if(m_processResult.left.zugongtype==1){
        zugongType_left="扁平足";
    }
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("足型：%1 ").arg(zugongType_left));

    y = DPI_1 * 257/ DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("足弓指数：%1").arg(QString::number(m_processResult.left.zugongzhishu,'f',2)));
    y = DPI_1 * 297 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("平均压力：%1 kPa").arg(QString::number(m_processResult.left.pressure,'f',2)));
    y = DPI_1 * 317 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("平均面积：%1 cm²").arg(QString::number(m_processResult.left.area,'f',2)));
    y = DPI_1 * 337 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("内外侧占比：%1 %").arg(QString::number(m_processResult.left.nei_wai,'f',2)));

    // 右脚数据
    x = DPI_1 * 429 / DPI_2;
    y = DPI_1 * 197 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, tr("右脚"));

    y = DPI_1 * 218 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("脚宽：%1 cm").arg(QString::number(m_processResult.right.jiaokuan,'f',2)));
    y = DPI_1 * 237 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("脚长：%1 cm").arg(QString::number(m_processResult.right.jiaochang,'f',2)));
    y = DPI_1 * 257 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("足弓指数：%1").arg(QString::number(m_processResult.right.zugongzhishu,'f',2)));
    y = DPI_1 * 277 / DPI_2;
    QString zugongType_right;
    if(m_processResult.right.zugongtype==-1){
        zugongType_right="高弓足";
    }  else  if(m_processResult.right.zugongtype==0){
        zugongType_right="正常足";
    }
    else  if(m_processResult.right.zugongtype==1){
        zugongType_right="扁平足";
    }
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("足型：%1 ").arg(zugongType_right));
    y = DPI_1 * 297 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("平均压力：%1 kPa").arg(QString::number(m_processResult.right.pressure,'f',2)));
    y = DPI_1 * 317 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("平均面积：%1 cm²").arg(QString::number(m_processResult.right.area,'f',2)));
    y = DPI_1 * 337 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("内外侧占比：%1 %").arg(QString::number(m_processResult.right.nei_wai,'f',2)));

    // 画脚型参考图
    x = DPI_1 * 237 / DPI_2;
    y = DPI_1 * 278 / DPI_2;
    QImage image_jiaoxing("://new//prefix1//icon//jiaoxing.png");
    image_jiaoxing = image_jiaoxing.scaled(image_jiaoxing.width(), image_jiaoxing.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pPainter->drawImage(x, y, image_jiaoxing);

    // 脚型图
    // 画矩形框
    x = DPI_1 * 144 / DPI_2;
    y = DPI_1 * 347 / DPI_2;
    QImage image_12("://new//prefix1//icon//12.png");
    image_12 = image_12.scaled(DPI_1 * 300 / DPI_2, DPI_1 * 260 / DPI_2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pPainter->drawImage(x, y+80, image_12);

    // 画脚

    if(globaldata::image_zudi!=NULL){
        QImage original = *globaldata::image_zudi;

        original = original.scaled(DPI_1 * 312 / DPI_2, DPI_1 * 291 / DPI_2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        // 绘制时指定目标矩形区域，实现等比例放大
        pPainter->drawImage(x, y+80, original);
    }
    // 静止摆动面积
    x = 0;
    y = DPI_1 * 640 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 595 / DPI_2, nLineHeight), Qt::AlignCenter | Qt::AlignVCenter, QString("静止COP摆动面积： %1 mm²").arg(QString::number(m_processResult.ellipse.area,'f',2)));

    // 画椭圆（修复：传入激活的painter）
    QPointF center(DPI_1*298/DPI_2, DPI_1*701/DPI_2); // 转换为毫米/像素适配
    drawTargetEllipse(*pPainter, center, m_processResult.ellipse.a, m_processResult.ellipse.b, m_processResult.ellipse.angle);
    font.setPointSize(12);
    pPainter->setFont(font);
    pPainter->setPen(Qt::black);
    // 左脚数据
    x = DPI_1 * 49 / DPI_2;
    y = DPI_1 * 645 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, tr("左脚"));

    y = DPI_1 * 666 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("前后占比：%1 %").arg(QString::number(m_processResult.left.up_back,'f',2)));
    y = DPI_1 * 690 / DPI_2;
    cv::Vec2f fft0 = m_processResult.left.ffts[0];
    QString fftString0 = QString("%1, %2").arg(fft0[0]).arg(fft0[1]);
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("傅里叶变换: %1").arg(fftString0));
    y = DPI_1 * 713 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("重心前后偏移： %1 pt").arg(m_processResult.left.center_hight));
    y = DPI_1 * 735 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("重心左右偏移： %1 pt").arg(m_processResult.left.center_width));

    // 右脚数据
    x = DPI_1 * 429 / DPI_2;
    y = DPI_1 * 645 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, tr("右脚"));

    y = DPI_1 * 666 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("前后占比：%1 %").arg(QString::number(m_processResult.right.up_back,'f',2)));
    y = DPI_1 * 690 / DPI_2;
    cv::Vec2f fft1 = m_processResult.right.ffts[0];
    QString fftString1 = QString("%1, %2").arg(fft1[0]).arg(fft1[1]);
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("傅里叶变换: %1").arg(fftString1));
    y = DPI_1 * 713 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("重心前后偏移： %1 pt").arg(m_processResult.right.center_hight));
    y = DPI_1 * 735 / DPI_2;
    pPainter->drawText(QRect(x, y, nColumnSpace, nLineHeight), Qt::AlignLeft | Qt::AlignVCenter, QString("重心左右偏移： %1 pt").arg(m_processResult.right.center_width));

    x = DPI_1 * 42 / DPI_2;
    y = DPI_1 * 762 / DPI_2;
    QImage image_setiao("://new//prefix1//icon//setiao.png");
    image_setiao = image_setiao.scaled(DPI_1 * 528 / DPI_2, DPI_1 * 17 / DPI_2,Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pPainter->drawImage(x, y, image_setiao);
}

void Form_processdata::drawTargetEllipse(QPainter &painter, const QPointF &center, double longRadius, double shortRadius, double tiltAngle)
{
    // 增加Painter激活判断
    if (!painter.isActive()) {
        qDebug() << "drawTargetEllipse: Painter未激活";
        return;
    }

    qDebug()<<"椭圆数据："<< m_processResult.ellipse.a<<  m_processResult.ellipse.b<<  m_processResult.ellipse.angle;

    // ========== 1. 基础设置（抗锯齿+单位适配） ==========
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // 报告中目标区域是 200×200，需将像素值（如25px/28px）按比例缩放
    // 示例中原始图是 25px ≈ 报告中 25*(200/400) = 12.5mm（400→200 缩放0.5倍）
    const qreal SCALE = 100.0/ 72.0; // 400×400 Widget → 报告中200×200区域的缩放比例
    longRadius *= SCALE;
    shortRadius *= SCALE;


    // ========== 2. 绘制外圆（对应示例中的灰色大圆） ==========
    QPen outerCirclePen(Qt::lightGray, 1.5); // 浅灰色细边框
    painter.setPen(outerCirclePen);
    painter.setBrush(Qt::NoBrush);
    // 外圆半径取示例中最大的28px*SCALE = 14mm
    qreal outerCircleRadius = 72.0*SCALE;
    painter.drawEllipse(center, outerCircleRadius, outerCircleRadius);


    // ========== 3. 绘制中心十字线 ==========
    QPen crossPen(Qt::gray, 2.5); // 灰色十字线
    painter.setPen(crossPen);
    // 十字线长度（示例中25px*2 → 报告中25*SCALE*2 = 25mm）
    qreal crossLength = 100.0 * SCALE * 2.0;
    qDebug()<<crossLength<<"crossLength"<<center;
    // 水平线
    painter.drawLine(center.x() - crossLength/2.0, center.y(),
                     center.x() + crossLength/2.0, center.y());
    // 垂直线
    painter.drawLine(center.x(), center.y() - crossLength/2.0,
                     center.x(), center.y() + crossLength/2.0);


    // ========== 4. 绘制倾斜椭圆（红色，逆时针45°） ==========
    QPen ellipsePen(Qt::red, 2.5); // 红色粗边框
    painter.setPen(ellipsePen);
    painter.setBrush(Qt::NoBrush);

    // 坐标变换：以中心为旋转点，逆时针旋转tiltAngle度
    painter.save(); // 保存当前坐标系
    painter.translate(center); // 平移坐标系到椭圆中心
    painter.rotate(tiltAngle); // 逆时针旋转倾斜角（示例中是45°）
    // 绘制椭圆（长轴=longRadius*2，短轴=shortRadius*2）
    painter.drawEllipse(QPointF(0, 0), longRadius, shortRadius);



    // ========== 5. 绘制中心圆心（示例中的黑色小方块） ==========
    QPen squarePen(Qt::black, 1);
    painter.setPen(squarePen);
    painter.setBrush(Qt::black);
    qreal squareSize = 2.0 * SCALE; // 小方块大小（示例中约4px）
    painter.drawEllipse(QPointF(0, 0), squareSize, squareSize);
    painter.restore(); // 恢复原始坐标系
}



bool Form_processdata::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseMove){
        if(isMaximized()){
            return QWidget::eventFilter(obj,event);
        }
        QMouseEvent* pMouse = dynamic_cast<QMouseEvent*>(event);
        if(pMouse){
            checkBorderDragging(pMouse);
            if(obj == ui->topFrame && m_bMousePressedTitle){
                if(cursor() == Qt::ArrowCursor){
                    move(m_wndPos + (pMouse->globalPosition().toPoint() - m_mousePos));
                }
            }
        }
    }else if(event->type() == QEvent::MouseButtonPress && obj == ui->topFrame){
        QMouseEvent* pMouse = dynamic_cast<QMouseEvent*>(event);
        if(pMouse){
            this->m_bMousePressedTitle = true;
            this->m_mousePos = pMouse->globalPosition().toPoint();
            this->m_wndPos = this->pos();
        }
    }else if(event->type() == QEvent::MouseButtonRelease && obj == ui->topFrame){
        this->m_bMousePressedTitle = false;
    }else if(event->type() == QEvent::MouseButtonPress && obj == this){
        QMouseEvent* pMouse = dynamic_cast<QMouseEvent*>(event);
        if(pMouse){
            mousePressEvent(pMouse);
        }
    }else if(event->type() == QEvent::MouseButtonRelease && obj == this){
        if(this->m_bMousePressed){
            QMouseEvent* pMouse = dynamic_cast<QMouseEvent*>(event);
            if(pMouse){
                mouseReleaseEvent(pMouse);
            }
        }
    }else if(event->type() == QEvent::MouseButtonDblClick && obj == ui->topFrame){
        if(windowState().testFlag(Qt::WindowNoState)){
            this->on_main_max_clicked();
        }else if(windowState().testFlag(Qt::WindowMaximized)){
            this->on_main_restore_clicked();
        }
    }


    return QWidget::eventFilter(obj,event);
}

void Form_processdata::mousePressEvent(QMouseEvent *event)
{
    if(isMaximized()){
        return;
    }

    this->m_bMousePressed = true;
    this->m_StartGeometry = this->geometry();
    QPoint globalMousePos = mapToGlobal(QPoint(event->position().x(),event->position().y()));
    if(LeftBorderHit(globalMousePos)&&TopBorderHit(globalMousePos)){
        m_bDragTop = true;
        m_bDragLeft = true;
        setCursor(Qt::SizeFDiagCursor);
    }else if(RightBorderHit(globalMousePos) && TopBorderHit(globalMousePos)){
        m_bDragTop = true;
        m_bDragRight = true;
        setCursor(Qt::SizeBDiagCursor);
    } else if(LeftBorderHit(globalMousePos) && BottomBorderHit(globalMousePos)){
        m_bDragLeft = true;
        m_bDragBottom = true;
        setCursor(Qt::SizeBDiagCursor);
    }else if(RightBorderHit(globalMousePos) && BottomBorderHit(globalMousePos)){
        m_bDragRight = true;
        m_bDragBottom = true;
        setCursor(Qt::SizeFDiagCursor);
    }else{
        if(TopBorderHit(globalMousePos)){
            m_bDragTop = true;
            setCursor(Qt::SizeVerCursor);
        }else if(LeftBorderHit(globalMousePos)){
            m_bDragLeft = true;
            setCursor(Qt::SizeHorCursor);
        }else if(BottomBorderHit(globalMousePos)){
            m_bDragBottom = true;
            setCursor(Qt::SizeVerCursor);
        }else if(RightBorderHit(globalMousePos)){
            m_bDragRight = true;
            setCursor(Qt::SizeHorCursor);
        }
    }
}

void Form_processdata::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if(isMaximized()){
        return;
    }
    m_bMousePressed = false;
    bool bSwitchBackCursor = m_bDragTop || m_bDragLeft || m_bDragRight || m_bDragBottom;
    m_bDragTop = false;
    m_bDragLeft = false;
    m_bDragBottom = false;
    m_bDragRight = false;
    if(bSwitchBackCursor){
        setCursor(Qt::ArrowCursor);
    }
}

void Form_processdata::checkBorderDragging(QMouseEvent *event)
{
    if(isMaximized()){
        return;
    }

    QPoint globalMousePos = event->globalPosition().toPoint();
    if(this->m_bMousePressed){
        if(m_bDragTop && m_bDragRight){
            int newHeight = m_StartGeometry.height() + m_StartGeometry.y() - globalMousePos.y();
            int newWidth = globalMousePos.x() - m_StartGeometry.x();
            if(newWidth > this->minimumHeight() && newWidth > this->minimumWidth()){
                setGeometry(m_StartGeometry.x(), globalMousePos.y(),newWidth,newHeight);
            }
        }else if(m_bDragTop && m_bDragLeft){
            int newHeight = m_StartGeometry.height() + m_StartGeometry.y() - globalMousePos.y();
            int newWidth = m_StartGeometry.width() + m_StartGeometry.x() - globalMousePos.x();
            if (newHeight > this->minimumHeight() && newWidth > this->minimumWidth()) {
                setGeometry(globalMousePos.x(), globalMousePos.y(), newWidth, newHeight);
            }
        }else if(m_bDragBottom && m_bDragLeft){
            int newHeight = globalMousePos.y() - m_StartGeometry.y();
            int newWidth = m_StartGeometry.width() + m_StartGeometry.x() - globalMousePos.x();
            if (newHeight > this->minimumHeight() && newWidth > this->minimumWidth()) {
                setGeometry(globalMousePos.x(), m_StartGeometry.y(), newWidth, newHeight);
            }
        }else if(m_bDragBottom && m_bDragRight){
            int newHeight = globalMousePos.y() - m_StartGeometry.y();
            int newWidth = globalMousePos.x() - m_StartGeometry.x();
            if (newHeight > this->minimumHeight() && newWidth > this->minimumWidth()) {
                resize(newWidth, newHeight);
            }
        }else if(m_bDragTop){
            int newHeight = m_StartGeometry.height() + m_StartGeometry.y() - globalMousePos.y();
            if (newHeight > this->minimumHeight()) {
                setGeometry(m_StartGeometry.x(), globalMousePos.y(), m_StartGeometry.width(),newHeight);
            }
        }else if (m_bDragLeft) {
            int newWidth = m_StartGeometry.width() + m_StartGeometry.x() - globalMousePos.x();
            if (newWidth > this->minimumWidth()) {
                setGeometry(globalMousePos.x(), m_StartGeometry.y(), newWidth, m_StartGeometry.height());
            }
        } else if(m_bDragRight){
            int newWidth = globalMousePos.x() - m_StartGeometry.x();
            if(newWidth > this->minimumWidth()){
                resize(newWidth, m_StartGeometry.height());
            }
        }else if(m_bDragBottom){
            int newHeight = globalMousePos.y() - m_StartGeometry.y();
            if (newHeight > this->minimumHeight()) {
                resize(m_StartGeometry.width(), newHeight);
            }
        }
    }else{
        if (LeftBorderHit(globalMousePos) && TopBorderHit(globalMousePos)) {
            setCursor(Qt::SizeFDiagCursor);
        } else if (RightBorderHit(globalMousePos) && TopBorderHit(globalMousePos)) {
            setCursor(Qt::SizeBDiagCursor);
        } else if (LeftBorderHit(globalMousePos) && BottomBorderHit(globalMousePos)) {
            setCursor(Qt::SizeBDiagCursor);
        } else if(RightBorderHit(globalMousePos) && BottomBorderHit(globalMousePos)){
            setCursor(Qt::SizeFDiagCursor);
        } else {
            if (TopBorderHit(globalMousePos)) {
                setCursor(Qt::SizeVerCursor);
            } else if (LeftBorderHit(globalMousePos)) {
                setCursor(Qt::SizeHorCursor);
            } else if (RightBorderHit(globalMousePos)) {
                setCursor(Qt::SizeHorCursor);
            } else if (BottomBorderHit(globalMousePos)) {
                setCursor(Qt::SizeVerCursor);
            } else {
                m_bDragTop = false;
                m_bDragLeft = false;
                m_bDragRight = false;
                m_bDragBottom = false;
                setCursor(Qt::ArrowCursor);
            }
        }
    }
}

void Form_processdata::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape){
        if(this->isMaximized()){
            on_main_restore_clicked();
            return;
        }
    }
    QWidget::keyPressEvent(ev);
}


void Form_processdata::on_main_return_clicked()
{
    emit  hideAndShowMain_process();
    this->close();
}

