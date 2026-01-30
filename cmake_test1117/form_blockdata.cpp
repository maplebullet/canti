#include "form_blockdata.h"
#include "ui_form_blockdata.h"


Form_blockdata::Form_blockdata(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Form_blockdata)
{

    // 加载样式表（假设样式表文件为 styles.qss，已添加到资源）
    QFile styleSheet(":/new/prefix1/icon/styles.qss");
    if (styleSheet.open(QIODevice::ReadOnly)) {
        setStyleSheet(styleSheet.readAll());
        styleSheet.close();
    } else {
        qDebug() << "样式表加载失败：" << styleSheet.errorString();
    }
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);  //取消默认功能
    QApplication::instance()->installEventFilter(this);
    ui->main_restore->setVisible(true);
    ui->main_max->setVisible(false);
}

Form_blockdata::~Form_blockdata()
{
    delete ui;
}


void Form_blockdata::readBlockData(Dongtai str)
{
    qDebug() << "readBlockData 1";

     result = str;

    if (result.buchang==NULL) {
        qDebug() << "Error: result is null!";
        return;
    }

    // 在访问之前检查 left_mean_time 数组是否有效
    for (int i = 0; i < 4; ++i) {
        if (std::isnan(result.left_mean_time[i])) {
            qDebug() << "Warning: Invalid left_mean_time[" << i << "] value: NaN";
            result.left_mean_time[i] = 0.0;  // Default to 0.0 if NaN
        }
    }
    // 4个阶段平均时间----左脚
    ui->left_mean_time_0->setText(QString::number(result.left_mean_time[0],'f',2)+ " s");
    ui->left_mean_time_1->setText(QString::number(result.left_mean_time[1],'f',2)+ " s");
    ui->left_mean_time_2->setText(QString::number(result.left_mean_time[2],'f',2)+ " s");
    ui->left_mean_time_3->setText(QString::number(result.left_mean_time[3],'f',2)+ " s");
    // // 4个阶段平均时间----右脚
    ui->right_mean_time_0->setText(QString::number(result.right_mean_time[0],'f',2)+ " s");
    ui->right_mean_time_1->setText(QString::number(result.right_mean_time[1],'f',2)+ " s");
    ui->right_mean_time_2->setText(QString::number(result.right_mean_time[2],'f',2)+ " s");
    ui->right_mean_time_3->setText(QString::number(result.right_mean_time[3],'f',2)+ " s");

    //平均时间差值
    ui->diff_mean_time_0->setText(QString::number(result.left_mean_time[0]-result.right_mean_time[0],'f',2)+ " KPa");
    ui->diff_mean_time_1->setText(QString::number(result.left_mean_time[1]-result.right_mean_time[1],'f',2)+ " KPa");
    ui->diff_mean_time_2->setText(QString::number(result.left_mean_time[2]-result.right_mean_time[2],'f',2)+ " KPa");
    ui->diff_mean_time_3->setText(QString::number(result.left_mean_time[3]-result.right_mean_time[3],'f',2)+ " KPa");



    // // 4个阶段平均压力----左脚
    ui->left_mean_pressures_0->setText(QString::number(result.left_mean_pressures[0],'f',2) + " KPa");
    ui->left_mean_pressures_1->setText(QString::number(result.left_mean_pressures[1],'f',2) + " KPa");
    ui->left_mean_pressures_2->setText(QString::number(result.left_mean_pressures[2],'f',2) + " KPa");
    ui->left_mean_pressures_3->setText(QString::number(result.left_mean_pressures[3],'f',2) + " KPa");
    // // 4个阶段平均压力----右脚
    ui->right_mean_pressures_0->setText(QString::number(result.right_mean_pressures[0],'f',2) + " KPa");
    ui->right_mean_pressures_1->setText(QString::number(result.right_mean_pressures[1],'f',2) + " KPa");
    ui->right_mean_pressures_2->setText(QString::number(result.right_mean_pressures[2],'f',2) + " KPa");
    ui->right_mean_pressures_3->setText(QString::number(result.right_mean_pressures[3],'f',2) + " KPa");
    // 4个阶段平均压力差值
    ui->diff_mean_pressures_0->setText(QString::number(result.left_mean_pressures[0]-result.right_mean_pressures[0],'f',2) + " KPa");
    ui->diff_mean_pressures_1->setText(QString::number(result.left_mean_pressures[1]-result.right_mean_pressures[1],'f',2) + " KPa");
    ui->diff_mean_pressures_2->setText(QString::number(result.left_mean_pressures[2]-result.right_mean_pressures[2],'f',2) + " KPa");
    ui->diff_mean_pressures_3->setText(QString::number(result.left_mean_pressures[3]-result.right_mean_pressures[3],'f',2) + " KPa");

    // // 4个阶段平均面积----左脚
    ui->left_mean_areas_0->setText(QString::number(result.left_mean_areas[0],'f',2) + " cm²");
    ui->left_mean_areas_1->setText(QString::number(result.left_mean_areas[1],'f',2) + " cm²");
    ui->left_mean_areas_2->setText(QString::number(result.left_mean_areas[2],'f',2) + " cm²");
    ui->left_mean_areas_3->setText(QString::number(result.left_mean_areas[3],'f',2) + " cm²");
    // // 4个阶段平均面积----右脚
    ui->right_mean_areas_0->setText(QString::number(result.right_mean_areas[0],'f',2) + " cm²");
    ui->right_mean_areas_1->setText(QString::number(result.right_mean_areas[1],'f',2) + " cm²");
    ui->right_mean_areas_2->setText(QString::number(result.right_mean_areas[2],'f',2) + " cm²");
    ui->right_mean_areas_3->setText(QString::number(result.right_mean_areas[3],'f',2) + " cm²");
    // 4个阶段平均面积 差值
    ui->diff_mean_areas_0->setText(QString::number(result.left_mean_areas[0]-result.right_mean_areas[0],'f',2) + " cm²");
    ui->diff_mean_areas_1->setText(QString::number(result.left_mean_areas[1]-result.right_mean_areas[1],'f',2) + " cm²");
    ui->diff_mean_areas_2->setText(QString::number(result.left_mean_areas[2]-result.right_mean_areas[2],'f',2) + " cm²");
    ui->diff_mean_areas_3->setText(QString::number(result.left_mean_areas[3]-result.right_mean_areas[3],'f',2) + " cm²");

    // 前后侧受力比例----左脚
   ui->left_mean_up_lower->setText(QString::number(result.left_mean_up_lower,'f',2) + " %");
    // 前后侧受力比例----右脚
   ui->right_mean_up_lower->setText(QString::number(result.right_mean_up_lower,'f',2) + " %");
   // 前后侧受力比例----差值
   ui->diff_mean_up_lower->setText(QString::number(result.left_mean_up_lower-result.right_mean_up_lower,'f',2) + " %");

    //左右侧受力比例----左脚
    ui->left_mean_left_right->setText(QString::number(result.left_mean_left_right,'f',2) + " %");
    // 左右侧受力比例----右脚
    ui->right_mean_left_right->setText(QString::number(result.right_mean_left_right,'f',2) + " %");
    // 左右侧受力比例----差值
    ui->diff_mean_left_right->setText(QString::number(result.left_mean_left_right-result.right_mean_left_right,'f',2) + " %");


    // 平均偏移角度
    ui->left_ori->setText(QString::number(result.left_ori,'f',2) + "°");
    ui->right_ori->setText(QString::number(result.right_ori,'f',2) + "°");
    ui->diff_ori->setText(QString::number(result.left_ori-result.right_ori,'f',2) + "°");

    // 转换Mat为QImage
    QImage img = Mat2QImage(const_cast<cv::Mat&>(result.quanzhouqitu));

    // 核心修改：顺时针旋转90度（使用旋转矩阵 + 平滑变换）
    QImage rotatedImg = img.transformed(QTransform().rotate(90), Qt::SmoothTransformation);
    qDebug()<< "lable长宽： "     <<  ui->label_quanzhouqitu->width()<<
        ui->label_quanzhouqitu->height();
    // 缩放旋转后的图片（保持原有缩放逻辑）
    QImage scaledImg = rotatedImg.scaled(
         914,
         243,
        Qt::KeepAspectRatioByExpanding,  // 保持宽高比，扩展以覆盖目标尺寸
        Qt::SmoothTransformation         // 平滑插值（避免锯齿）
        );

    // 设置到Label显示
    ui->label_quanzhouqitu->setPixmap(QPixmap::fromImage(scaledImg));

    ui->label_busu->setText(QString::number(result.mean_speed*0.01,'f',2)+" m/s" );
    ui->label_bupin->setText(QString::number(result.bupin,'f',2)+" 步数/s" );
    ui->label_bukuan->setText(QString::number(result.bukuan,'f',2)+" cm" );
    ui->label_buchang->setText(QString::number(result.buchang,'f',2)+" cm" );
    ui->label_buchang_bianyi->setText(QString::number(result.buchang_bianyi*100,'f',2)+" %" );
    ui->label_bukuan_bianyi->setText(QString::number(result.bukuan_bianyi*100,'f',2)+" %" );
    ui->label_shuangzhicheng_bianyi->setText(QString::number(result.shuangzhicheng_bianyi*100,'f',2)+" %" );
}



QImage Form_blockdata::Mat2QImage(cv::Mat& image)
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
        // 第一步：将灰度图中值为0的像素（背景）替换为255（白色）
        cv::Mat whiteBgImage = image.clone();
        whiteBgImage.setTo(255, whiteBgImage == 0); // 0值（黑色）替换为255（白色）
        // 第二步：应用伪彩色映射
        cv::Mat coloredImage;
        cv::applyColorMap(image, coloredImage, cv::COLORMAP_JET); // 使用 JET 伪彩色


        // 第三步：将伪彩色图中对应原0值的位置（现在是伪彩色的背景色）再替换为白色
        // 找到原图像中0值的位置，将伪彩色图对应位置设为白色（BGR格式：255,255,255）
        cv::Mat mask = (image == 0);
        coloredImage.setTo(cv::Scalar(255, 255, 255), mask);

        image = coloredImage; // 将处理后的伪彩色图像覆盖到 image

    }

    QImage img;
    if (image.type() == CV_8UC3) { // 3通道 BGR
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        img = QImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);


    }
    //else if (image.type() == CV_8UC1) { // 单通道
    //    img = QImage(image.data, image.cols, image.rows, image.step, QImage::Format_Grayscale8);
    //}
    else {
        qDebug() << "Unsupported Mat type:" << image.type();
        img = QImage();
    }
    img = img.copy();

    return img;
}

// 用于更新图像到 QLabel 控件
void Form_blockdata::setLabelImage(QLabel* label, const cv::Mat& image) {
    if (image.empty()) {
        qDebug() << "Error: Image is empty.";
        return;
    }

    QImage img = Mat2QImage(const_cast<cv::Mat&>(image));
    if (img.isNull()) {
        qDebug() << "Error: Failed to convert Mat to QImage.";
        return;
    }
    // ------------------------------ 目标尺寸定义 ------------------------------
    const int targetWidth = 180;   // QLabel 固定宽度
    const int targetHeight = 180;  // QLabel 固定高度

    // ------------------------------ 计算缩放比例（保持宽高比） ------------------------------
    // 图像原始宽高
    int imgWidth = img.width();
    int imgHeight = img.height();

    // 计算宽高比（图像 vs 目标）
    double widthRatio = static_cast<double>(targetWidth) / imgWidth;
    double heightRatio = static_cast<double>(targetHeight) / imgHeight;
    double scaleRatio = qMax(widthRatio, heightRatio);  // 取较大的比例（确保覆盖目标尺寸）

    // 缩放后的图像尺寸（至少覆盖目标尺寸）
    int scaledWidth = static_cast<int>(imgWidth * scaleRatio);
    int scaledHeight = static_cast<int>(imgHeight * scaleRatio);

    // ------------------------------ 缩放图像（平滑变换） ------------------------------
    QImage scaledImg = img.scaled(
        scaledWidth,
        scaledHeight,
        Qt::KeepAspectRatioByExpanding,  // 保持宽高比，扩展以覆盖目标尺寸
        Qt::SmoothTransformation         // 平滑插值（避免锯齿）
        );

    // ------------------------------ 居中裁剪至目标尺寸 ------------------------------
    // 计算裁剪区域的起始坐标（居中）
    int cropX = (scaledWidth - targetWidth) / 2;
    int cropY = (scaledHeight - targetHeight) / 2;

    // 确保裁剪区域在图像范围内（避免越界）
    cropX = qMax(0, cropX);
    cropY = qMax(0, cropY);
    int actualCropWidth = qMin(targetWidth, scaledWidth - cropX);
    int actualCropHeight = qMin(targetHeight, scaledHeight - cropY);

    // 执行裁剪
    QImage croppedImg = scaledImg.copy(
        cropX,
        cropY,
        actualCropWidth,
        actualCropHeight
        );

    // ------------------------------ 设置到 QLabel ------------------------------
    // 设置 QLabel 固定尺寸（可选，根据需求决定是否固定）
    label->setFixedSize(targetWidth, targetHeight);

    // 设置 QPixmap（使用裁剪后的图像）
    label->setPixmap(QPixmap::fromImage(croppedImg));

    // 样式设置：蓝色背景（#00007B），文字居中（若需要显示文字）
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(R"(
        QLabel {
            background-color: #00007B;  /* 深蓝色背景 */
            border: 1px solid #000000;  /* 可选：添加细边框增强对比度 */
        }
    )");
    // // 设置 QLabel 的对齐方式为居中，蓝色背景#00007B
    // label->setAlignment(Qt::AlignCenter);
    // label->setStyleSheet("QLabel { background-color: #00007B; }");

    // // 设置 QLabel 的图像，按控件大小保持比例自适应，并确保缩放后图像质量更高
    // label->setPixmap(QPixmap::fromImage(img).scaled(label->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}

void Form_blockdata::printDongtaiResult(const Dongtai& result) {
    // 打印 left_mean_time 数组
    qDebug() << "left_mean_time: ";
    for (int i = 0; i < 4; ++i) {
        qDebug() << "left_mean_time[" << i << "]: " << result.left_mean_time[i];
    }

    // 打印 right_mean_time 数组
    qDebug() << "right_mean_time: ";
    for (int i = 0; i < 4; ++i) {
        qDebug() << "right_mean_time[" << i << "]: " << result.right_mean_time[i];
    }

    // 打印 left_mean_pressures 数组
    qDebug() << "left_mean_pressures: ";
    for (int i = 0; i < 4; ++i) {
        qDebug() << "left_mean_pressures[" << i << "]: " << result.left_mean_pressures[i];
    }

    // 打印 right_mean_pressures 数组
    qDebug() << "right_mean_pressures: ";
    for (int i = 0; i < 4; ++i) {
        qDebug() << "right_mean_pressures[" << i << "]: " << result.right_mean_pressures[i];
    }

    // 打印 left_mean_areas 数组
    qDebug() << "left_mean_areas: ";
    for (int i = 0; i < 4; ++i) {
        qDebug() << "left_mean_areas[" << i << "]: " << result.left_mean_areas[i];
    }

    // 打印 right_mean_areas 数组
    qDebug() << "right_mean_areas: ";
    for (int i = 0; i < 4; ++i) {
        qDebug() << "right_mean_areas[" << i << "]: " << result.right_mean_areas[i];
    }

    // 打印其他成员变量
    qDebug() << "left_mean_up_lower: " << result.left_mean_up_lower;
    qDebug() << "right_mean_up_lower: " << result.right_mean_up_lower;
    qDebug() << "left_mean_left_right: " << result.left_mean_left_right;
    qDebug() << "right_mean_left_right: " << result.right_mean_left_right;
    qDebug() << "left_ori: " << result.left_ori;
    qDebug() << "right_ori: " << result.right_ori;

    // 打印 centers
    qDebug() << "centers: ";
    for (const auto& point : result.centers) {
        qDebug() << "Center: (" << point.x << ", " << point.y << ")";
    }

    // 打印 cops（如果是 vector<vector<Point2f>>）
    // qDebug() << "left_cops:";
    // for (const auto& cop : result.left_cops) {
    //     for (const auto& point : cop) {
    //         qDebug() << "Point: (" << point.x << ", " << point.y << ")";
    //     }
    // }

    // qDebug() << "right_cops:";
    // for (const auto& cop : result.right_cops) {
    //     for (const auto& point : cop) {
    //         qDebug() << "Point: (" << point.x << ", " << point.y << ")";
    //     }
    // }

    // 打印 cop 如果是 cv::Mat 类型
    //qDebug() << "cop: " << result.cop;
}
//动态步态报告
void Form_blockdata::recordReportDongtai()
{
    //1、生成文件名   格式"2000-01-02_hh_mm_ss_Jingtai"
    QString fileName;

    QString strdate=QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss")+"_Dongtai";

    //在debug或release下的pdfadress文件夹内
    globaldata::reportAdress=QCoreApplication::applicationDirPath()+"/pdfadress";
    fileName =globaldata::reportAdress+"/"+strdate+".pdf";
    globaldata::reportPath_dongtai=fileName;
    //qDebug()<<fileName<<"fileNamebutai";
    LOG_DEBUG("该步态分析报告文件名："+fileName);
    //2、 创建pdf文件
    QFile pdfFile(fileName);
    pdfFile.open(QIODevice::WriteOnly);
    //3、创建生成pdf类，作为绘图设备
    QPdfWriter pPdfWriter (&pdfFile);
    pPdfWriter.setResolution(300);
    pPdfWriter.setPageSize(QPageSize::A4);
    pPdfWriter.setPageMargins(QMarginsF(0, 0, 0, 0));
    QPainter painter;
    if (!painter.begin(&pPdfWriter)) {
        qDebug() << "Painter初始化失败";
        pdfFile.close();
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    //4、绘制报告内容
    paintPdfDongtai(&painter);

    painter.end();
    pdfFile.close();
}

void Form_blockdata::paintPdfDongtai(QPainter *pPainter)
{

    // ========== 修复1：创建Painter并绑定设备（必须调用begin()） ==========
    if (!pPainter || !pPainter->isActive()) {
        qDebug() << "第一页Painter未激活";
        return;
    }
    //qDebug() << pPainter->viewport();
    // ========== 修复2：Painter激活后再获取viewport尺寸 ==========
    int nPdfWidth = pPainter->viewport().width();
    int nPdfHeight = pPainter->viewport().height();
    qDebug()<<"A4长宽111："<<nPdfWidth<<nPdfHeight;

    LOG_DEBUG("进入生成步态报告"+QString::number(nPdfWidth)+" "+QString::number(nPdfHeight));
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
    pPainter->drawText(QRect(0, y, nPdfWidth, 100), Qt::AlignCenter, tr("步态压力分析报告"));
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



    font.setPointSize(12);
    y = DPI_1 * 209 / DPI_2;

    x = DPI_1 * 174 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), Qt::AlignCenter | Qt::AlignVCenter,"结果");
    x = DPI_1 * 248 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), Qt::AlignCenter | Qt::AlignVCenter,"参考值");
    x = DPI_1 * 383 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2),Qt::AlignCenter | Qt::AlignVCenter, "结果");
    x = DPI_1 * 462 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), Qt::AlignCenter | Qt::AlignVCenter,"参考值");


    y = DPI_1 * 228 / DPI_2;

    x = DPI_1 * 62 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 61 / DPI_2, DPI_1 * 15 / DPI_2), "平均速度");
    x = DPI_1 * 174 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), Qt::AlignCenter | Qt::AlignVCenter,QString::number(result.mean_speed*0.01,'f',2));
      x = DPI_1 * 248 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), " 1.0-1.5m/s");

    x = DPI_1 * 312 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 37 / DPI_2, DPI_1 * 15 / DPI_2), "步长");
    x = DPI_1 * 383 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), Qt::AlignCenter | Qt::AlignVCenter,QString::number(result.buchang,'f',2));

     x = DPI_1 * 462 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), " 50-80cm");
    //第二行

    y = DPI_1 * 250 / DPI_2;

    x = DPI_1 * 62 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), "步长变异性");
    x = DPI_1 * 174 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), Qt::AlignCenter | Qt::AlignVCenter,QString::number(result.buchang_bianyi*100,'f',2));
  x = DPI_1 * 248 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), " 1%-5%");

    x = DPI_1 * 312 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 37 / DPI_2, DPI_1 * 15 / DPI_2), "步宽");
    x = DPI_1 * 383 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), Qt::AlignCenter | Qt::AlignVCenter,QString::number(result.bukuan,'f',2));

    x = DPI_1 * 462 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), " 4.2-12cm");

    //第三行

    y = DPI_1 * 275 / DPI_2;

    x = DPI_1 * 62 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), "步宽变异性");
    x = DPI_1 * 174 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), Qt::AlignCenter | Qt::AlignVCenter,QString::number(result.bukuan_bianyi*100,'f',2));
    x = DPI_1 * 248 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), " 10%-20%");

    x = DPI_1 * 312 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 37 / DPI_2, DPI_1 * 15 / DPI_2), "步频");
    x = DPI_1 * 383 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), Qt::AlignCenter | Qt::AlignVCenter,QString::number(result.bupin,'f',2));

    x = DPI_1 * 462 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 70 / DPI_2, DPI_1 * 15 / DPI_2), " 1.66-2.08步/秒");

    //第四行
    y = DPI_1 * 296 / DPI_2;

    x = DPI_1 * 62 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 70 / DPI_2, DPI_1 * 15 / DPI_2), "双支撑相变异性");
     x = DPI_1 * 174 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 300 / DPI_2, DPI_1 * 15 / DPI_2), Qt::AlignCenter | Qt::AlignVCenter,QString::number(result.shuangzhicheng_bianyi*100,'f',2));
    x = DPI_1 * 462 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 65 / DPI_2, DPI_1 * 15 / DPI_2), " 5%-15%");


    // 绘制表头

    y = DPI_1 * 346 / DPI_2;
    x = DPI_1 * 112 / DPI_2;
    QString headers_0[3] = {"左足", "右足","差值" };
    pPainter->drawText(QRect(x, y, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_0[0]);
    pPainter->drawText(QRect(DPI_1 * 174 / DPI_2, y, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_0[1]);
    pPainter->drawText(QRect(DPI_1 * 233 / DPI_2, y, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_0[2]);
    pPainter->drawText(QRect(DPI_1 * 390 / DPI_2, y, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_0[0]);
    pPainter->drawText(QRect(DPI_1 * 451 / DPI_2, y, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_0[1]);
    pPainter->drawText(QRect(DPI_1 * 512 / DPI_2, y, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_0[2]);



    QString headers[3] = {"平均时间", "平均面积","平均压力" };
    y = DPI_1 * 366 / DPI_2;
    x = DPI_1 * 28 / DPI_2;
    pPainter->drawText(QRect(x, y, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers[0]);
    pPainter->drawText(QRect( DPI_1 * 305 / DPI_2, y, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers[1]);
    pPainter->drawText(QRect(x, DPI_1 * 457 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers[2]);

    font.setPointSize(10);
    pPainter->setFont(font);
    QString headers_1[4] = {"双支撑相", "承重反应期","承重末期","摆动前期" };

   //平均时间
    x = DPI_1 * 45 / DPI_2;
    pPainter->drawText(QRect(x, DPI_1 * 383 / DPI_2,  DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_1[0]);
    pPainter->drawText(QRect(x, DPI_1 * 400 / DPI_2,  DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_1[1]);
    pPainter->drawText(QRect(x, DPI_1 * 417 / DPI_2,  DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_1[2]);
    pPainter->drawText(QRect(x, DPI_1 * 434 / DPI_2,  DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_1[3]);


    //zuo
    x = DPI_1 * 110 / DPI_2;
    pPainter->drawText(QRect(x, DPI_1 * 383 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_time[0], 'f', 2)+ "s");
    pPainter->drawText(QRect(x, DPI_1 * 400 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_time[1], 'f', 2)+"s");
    pPainter->drawText(QRect(x, DPI_1 * 417 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_time[2], 'f', 2)+"s");
    pPainter->drawText(QRect(x, DPI_1 * 434 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_time[3], 'f', 2)+"s");
    //you
    x = DPI_1 * 174 / DPI_2;
    pPainter->drawText(QRect(x, DPI_1 * 383 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_time[0], 'f', 2)+"s");
    pPainter->drawText(QRect(x, DPI_1 * 400 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_time[1], 'f', 2)+"s");
    pPainter->drawText(QRect(x, DPI_1 * 417 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_time[2], 'f', 2)+"s");
    pPainter->drawText(QRect(x, DPI_1 * 434 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_time[3], 'f', 2)+"s");
    //差值
    x = DPI_1 * 233 / DPI_2;
    pPainter->drawText(QRect(x, DPI_1 * 383 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_time[0]-result.right_mean_time[0], 'f', 2)+"s");
    pPainter->drawText(QRect(x, DPI_1 * 400 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_time[1]-result.right_mean_time[1], 'f', 2)+"s");
    pPainter->drawText(QRect(x, DPI_1 * 417 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_time[2]-result.right_mean_time[2], 'f', 2)+"s");
    pPainter->drawText(QRect(x, DPI_1 * 434 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_time[3]-result.right_mean_time[3], 'f', 2)+"s");

   //平均面积

    x = DPI_1 * 310 / DPI_2;
    pPainter->drawText(QRect(x, DPI_1 * 383 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_1[0]);
    pPainter->drawText(QRect(x, DPI_1 * 400 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_1[1]);
    pPainter->drawText(QRect(x, DPI_1 * 417 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_1[2]);
    pPainter->drawText(QRect(x, DPI_1 * 434 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_1[3]);
    //zuo
     x = DPI_1 * 390 / DPI_2;
    pPainter->drawText(QRect(x,DPI_1 * 383 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_areas[0], 'f', 2)+"cm²");
    pPainter->drawText(QRect(x,DPI_1 * 400 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_areas[1], 'f', 2)+"cm²");
    pPainter->drawText(QRect(x, DPI_1 * 417 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_areas[2], 'f', 2)+"cm²");
    pPainter->drawText(QRect(x, DPI_1 * 434 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_areas[3], 'f', 2)+"cm²");
    // //you
      x = DPI_1 * 451 / DPI_2;
    pPainter->drawText(QRect(x,DPI_1 * 383 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_areas[0], 'f', 2)+"cm²");
    pPainter->drawText(QRect(x,DPI_1 * 400 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_areas[1], 'f', 2)+"cm²");
    pPainter->drawText(QRect(x,DPI_1 * 417 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_areas[2], 'f', 2)+"cm²");
    pPainter->drawText(QRect(x,DPI_1 * 434 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_areas[3], 'f', 2)+"cm²");

    x = DPI_1 * 512 / DPI_2;
    pPainter->drawText(QRect(x,DPI_1 * 383 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_areas[0]-result.right_mean_areas[0], 'f', 2)+"cm²");
    pPainter->drawText(QRect(x,DPI_1 * 400 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_areas[1]-result.right_mean_areas[1], 'f', 2)+"cm²");
    pPainter->drawText(QRect(x,DPI_1 * 417 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_areas[2]-result.right_mean_areas[2], 'f', 2)+"cm²");
    pPainter->drawText(QRect(x,DPI_1 * 434 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_areas[3]-result.right_mean_areas[3], 'f', 2)+"cm²");

    //平均压力
    x = DPI_1 * 45 / DPI_2;
    pPainter->drawText(QRect(x, DPI_1 * 474 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_1[0]);
    pPainter->drawText(QRect(x, DPI_1 * 490 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_1[1]);
    pPainter->drawText(QRect(x, DPI_1 * 508 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_1[2]);
    pPainter->drawText(QRect(x, DPI_1 * 526 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), headers_1[3]);
     x = DPI_1 * 115 / DPI_2;
    //zuo
    pPainter->drawText(QRect(x, DPI_1 * 474 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_pressures[0], 'f', 2)+ "KPa");
    pPainter->drawText(QRect(x, DPI_1 * 490 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_pressures[1], 'f', 2)+ "KPa");
    pPainter->drawText(QRect(x, DPI_1 * 508 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_pressures[2], 'f', 2)+ "KPa");
    pPainter->drawText(QRect(x, DPI_1 * 526 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_pressures[3], 'f', 2)+ "KPa");
    // //you
    x = DPI_1 * 174 / DPI_2;
    pPainter->drawText(QRect(x, DPI_1 * 474 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_pressures[0], 'f', 2)+ "KPa");
    pPainter->drawText(QRect(x, DPI_1 * 490 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_pressures[1], 'f', 2)+ "KPa");
    pPainter->drawText(QRect(x,DPI_1 * 508 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_pressures[2], 'f', 2)+ "KPa");
    pPainter->drawText(QRect(x, DPI_1 * 526 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_pressures[3], 'f', 2)+ "KPa");
    x = DPI_1 * 233 / DPI_2;
    pPainter->drawText(QRect(x, DPI_1 * 474 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_pressures[0]-result.right_mean_pressures[0], 'f', 2)+ "KPa");
    pPainter->drawText(QRect(x, DPI_1 * 490 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_pressures[0]-result.right_mean_pressures[1], 'f', 2)+ "KPa");
    pPainter->drawText(QRect(x,DPI_1 * 508 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_pressures[0]-result.right_mean_pressures[2], 'f', 2)+ "KPa");
    pPainter->drawText(QRect(x, DPI_1 * 526 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_pressures[0]-result.right_mean_pressures[3], 'f', 2)+ "KPa");



    x = DPI_1 * 300 / DPI_2;
    pPainter->drawText(QRect(x, DPI_1 * 474 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), "平均偏移角度");
    pPainter->drawText(QRect(x, DPI_1 * 490 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), "前后侧受力比例");
    pPainter->drawText(QRect(x, DPI_1 * 508 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), "左右侧受力比例");

    //zuo
    x = DPI_1 * 390 / DPI_2;
    pPainter->drawText(QRect(x, DPI_1 * 474 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_left_right, 'f', 2)+"%");
    pPainter->drawText(QRect(x, DPI_1 * 490 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_up_lower, 'f', 2)+"%");
    pPainter->drawText(QRect(x, DPI_1 * 508 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_ori, 'f', 2)+"°");
      x = DPI_1 * 451 / DPI_2;
    //you
    pPainter->drawText(QRect(x, DPI_1 * 474 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_left_right, 'f', 2)+"%");
    pPainter->drawText(QRect(x, DPI_1 * 490 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_up_lower, 'f', 2)+"%");
    pPainter->drawText(QRect(x, DPI_1 * 508 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_ori, 'f', 2)+"°");

    //差值
       x = DPI_1 * 512 / DPI_2;
    pPainter->drawText(QRect(x, DPI_1 * 474 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.left_mean_left_right-result.right_mean_left_right, 'f', 2)+"%");
    pPainter->drawText(QRect(x, DPI_1 * 490 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_mean_up_lower-result.right_mean_up_lower, 'f', 2)+"%");
    pPainter->drawText(QRect(x, DPI_1 * 508 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), QString::number(result.right_ori-result.right_ori, 'f', 2)+"°");


     font.setPointSize(14);
        pPainter->setFont(font);
     pPainter->drawText(QRect(  x = DPI_1 * 28 / DPI_2, DPI_1 * 540 / DPI_2, DPI_1 * 200 / DPI_2, DPI_1 * 75 / DPI_2), Qt::AlignLeft, tr("全周期的最大压力图像"));

    // 转换Mat为QImage
    QImage img = Mat2QImage(const_cast<cv::Mat&>(result.quanzhouqitu));

    // 核心修改：顺时针旋转90度（使用旋转矩阵 + 平滑变换）
    QImage rotatedImg = img.transformed(QTransform().rotate(90), Qt::SmoothTransformation);
    qDebug()<< "lable长宽： "     <<  ui->label_quanzhouqitu->width()<<
        ui->label_quanzhouqitu->height();
    // 缩放旋转后的图片（保持原有缩放逻辑）
    QImage scaledImg = rotatedImg.scaled(
        DPI_1 * 520 / DPI_2,
        DPI_1 * 138 / DPI_2,
        Qt::KeepAspectRatioByExpanding,  // 保持宽高比，扩展以覆盖目标尺寸
        Qt::SmoothTransformation         // 平滑插值（避免锯齿）
        );

    pPainter->drawImage(DPI_1 * 37 / DPI_2, DPI_1 * 560 / DPI_2,scaledImg);


    x = DPI_1 * 35 / DPI_2;
    y = DPI_1 * 750 / DPI_2;
    QImage image_setiao("://new//prefix1//icon//setiao.png");
    image_setiao = image_setiao.scaled(DPI_1 * 528 / DPI_2, DPI_1 * 17 / DPI_2,Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pPainter->drawImage(x, y, image_setiao);

    x = DPI_1 * 50 / DPI_2;
    y = DPI_1 * 800 / DPI_2;
    pPainter->setFont(QFont("黑体", 12, 36));
    pPainter->drawText(QRect(100, 3330, 1000, 75),"此报告不作为临床诊断和治疗的依据");
    //结束
    LOG_DEBUG("结束步态报告绘制");

}

void Form_blockdata::initUI()
{


}
void Form_blockdata::on_main_return_clicked()
{

  emit  hideAndShowMain_block();
   this->close();

}

void Form_blockdata::on_main_restore_clicked()
{
    ui->main_restore->setVisible(false);
    ui->main_max->setVisible(true);
    setWindowState(Qt::WindowNoState);
    this->update();
}

void Form_blockdata::on_main_min_clicked()
{
    this->showMinimized();
    this->update();
}

void Form_blockdata::on_main_max_clicked()
{
    ui->main_restore->setVisible(true);
    ui->main_max->setVisible(false);
    this->setWindowState(Qt::WindowMaximized);
    this->showMaximized();
    setCursor(Qt::ArrowCursor);
    this->update();
}

void Form_blockdata::on_main_close_clicked()
{
    qDebug()<<"close";
   emit hideAndShowMain_block();
    this->close();

}

bool Form_blockdata::LeftBorderHit(const QPoint &pos)
{
    const QRect& rect = this->geometry();
    if(pos.x() >= rect.x() && pos.x() <= (rect.x() + CONST_DRAG_BORDER_SIZE)){
        return true;
    }

    return false;
}

bool Form_blockdata::RightBorderHit(const QPoint &pos)
{
    const QRect& rect = this->geometry();
    int nPosX = rect.x()+rect.width();
    if(pos.x() <= nPosX && pos.x() >= (nPosX - CONST_DRAG_BORDER_SIZE)){
        return true;
    }

    return false;
}

bool Form_blockdata::TopBorderHit(const QPoint &pos)
{
    const QRect& rect = this->geometry();
    if(pos.y() > rect.y() && pos.y() <= (rect.y()+CONST_DRAG_BORDER_SIZE)){
        return true;
    }

    return false;
}

bool Form_blockdata::BottomBorderHit(const QPoint &pos)
{
    const QRect& rect = this->geometry();
    int nPosY =rect.y()+rect.height();
    if(pos.y() < nPosY && pos.y() >= (nPosY-CONST_DRAG_BORDER_SIZE)){
        return true;
    }

    return false;
}



bool Form_blockdata::eventFilter(QObject *obj, QEvent *event)
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

void Form_blockdata::mousePressEvent(QMouseEvent *event)
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

void Form_blockdata::mouseReleaseEvent(QMouseEvent *event)
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

void Form_blockdata::checkBorderDragging(QMouseEvent *event)
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

void Form_blockdata::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape){
        if(this->isMaximized()){
            on_main_restore_clicked();
            return;
        }
    }
    QWidget::keyPressEvent(ev);
}
