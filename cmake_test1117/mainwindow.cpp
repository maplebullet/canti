#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>
#include<QPdfWriter>
#include<QtMath>
#include <QDataStream>
#include <QCheckBox>
#include <QProgressDialog>
#include <QVBoxLayout>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlag(Qt::FramelessWindowHint);  //取消默认功能
    QApplication::instance()->installEventFilter(this);
    ui->main_restore->setVisible(true);
    ui->main_max->setVisible(false);

    QString iconPath=QDir::currentPath();


    QImage img(iconPath+"/Icon/logo.png");
    if (img.isNull()) {
        qDebug() << "Failed to load image:" << iconPath + "/Icon/logo.png";
    }
    initData();

    m_data_init.resize(4800); //1600
    m_data_init_40.resize(1600);
    std::fill(m_data_init_40.begin(),m_data_init_40.end(),0);
    std::fill(m_data_init.begin(),m_data_init.end(),0);

    sendToSensor=true;

    currentTime = new QElapsedTimer();


    m_timerMain = new QTimer(this);

    ImgWidth_40 =  (x_num_40+1)*10;//
    ImgHeight_40 = (y_num_40+1)*10;//410
    m_draw_pt_40=intialPtSize(40,40,10,10);//足底压力测试
    mapper_40_=new heatMap(15,ImgWidth_40,ImgHeight_40,200,ui->widget);
    
    // 为widget设置布局，使热力图控件能够自适应父widget大小
    QVBoxLayout *widgetLayout = new QVBoxLayout(ui->widget);
    widgetLayout->setContentsMargins(0, 0, 0, 0);
    widgetLayout->addWidget(mapper_40_);

    ////qDebug()<<m_draw_pt<<"m_draw_pt";


    ImgWidth_main =  (x_num_main+1)*11; //300 121*11=1331
    ImgHeight_main = (y_num_main+1)*7; //41*7=287
    qDebug()<<ImgWidth_main<<ImgWidth_main<<"ImgWidth_main";
    m_draw_pt=intialPtSize(120,40,11,7);//205,,8 47  //步态测试  实际垫子尺寸3.9×0.86米
    mapper_ = new heatmapper(DEFAULT_RADIUS, DEFAULT_OPACITY, m_draw_pt, ImgWidth_main, ImgHeight_main, ui->widget_2);
    
    // 为widget_2设置布局，使热力图控件能够自适应父widget大小
    QVBoxLayout *widget2Layout = new QVBoxLayout(ui->widget_2);
    widget2Layout->setContentsMargins(0, 0, 0, 0);
    widget2Layout->addWidget(mapper_);
//widget_2
    LOG_DEBUG("界面上步态绘图区域长 宽："+QString::number(ui->widget_2->width())+" "+QString::number(ui->widget_2->height()));
    // 初始化绘图线程
    // setupDrawWorker();
    // 初始化，假设能容纳3个QVector 存储每个串口的数据，有3个元素来记录每个串口的状态
    receivedData.resize(3);
    receivedStatus.resize(3);
    receivedStatus.fill(false);
    // 创建过滤线程和过滤器对象
    filterThread = new QThread(this);
    dataFilterWorker = new DataFilterWorker();
    dataFilterWorker->moveToThread(filterThread);
    // 初始化保存线程和工作对象，但暂时不启动
    fileSaveThread = new QThread(this);
    fileSaveWorker = nullptr; // 只有在点击“保存文件”时才创建
    // 初始化批量保存的计时器
    dataSaveTimer = new QTimer(this);


    // 初始化 heatmapper 线程
    heatmapThread = new QThread(this);
    mapper_ ->moveToThread(heatmapThread);

    // 连接信号和槽

    connect(this, &MainWindow::newDataPacket, dataFilterWorker, &DataFilterWorker::addDataPacket_2); // 不计算平均值，仅转换为压力

    connect(dataFilterWorker, &DataFilterWorker::filteredDataReady, this, &MainWindow::handleFilteredData);


    connect(ui->recordReport,&QPushButton::clicked,this,&MainWindow::recordReportSlot);

    connect(this, &MainWindow::sendDrawParam, mapper_, &heatmapper::processAndDrawData, Qt::QueuedConnection);

    connect(heatmapThread, &QThread::finished, mapper_, &QObject::deleteLater);

    m_pSerialPortThread = new CSerialPortThread();
    m_pSerialPortThread->init();
    // 在完成相关信号槽连接之后，启动线程
    filterThread->start();
    heatmapThread->start();

}

MainWindow::~MainWindow()
{
    if(m_pSerialPortThread)
    {

        m_pSerialPortThread->quit();
        m_pSerialPortThread->unInit();
        delete m_pSerialPortThread;
        m_pSerialPortThread = NULL;
    }
    stopSerial();  // 确保关闭串口

    if (fileSaveWorker) {
        fileSaveWorker->closeFile(); // 确保剩余数据写入文件
        fileSaveThread->quit();
        fileSaveThread->wait();
        delete fileSaveWorker;
    }


    if (heatmapThread->isRunning()) {
        heatmapThread->quit();
        heatmapThread->wait();
    }

    filterThread->quit();
    filterThread->wait();
    delete dataFilterWorker;
    delete serialPort_JingTai;
    delete ui;

    delete m_timerMain;
}

void MainWindow::testTemp()
{

}

// 1031
void MainWindow::onCheckboxStateChanged()
{
    // 更新选中的串口数量
    selectedPortCount = 0;
    if (checkBoxPort1->isChecked()) ++selectedPortCount;
    if (checkBoxPort2->isChecked()) ++selectedPortCount;
    if (checkBoxPort3->isChecked()) ++selectedPortCount;
}

// 初始化并启动串口读取线程
void MainWindow::startSerial()
{

    QProgressDialog progressDialog(this);
    progressDialog.setWindowTitle("提示");
    progressDialog.setLabelText("设备连接中，请稍等...");
    progressDialog.setCancelButton(nullptr);
    progressDialog.setWindowModality(Qt::ApplicationModal);
    progressDialog.setMinimumDuration(0);
    progressDialog.setRange(0, 0);
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(progressDialog.layout());
    if (layout) layout->setAlignment(Qt::AlignCenter);
    progressDialog.show();
    QCoreApplication::processEvents();

    // 清理旧数据
    m_dongtaiPort1.clear();
    m_dongtaiPort2.clear();
    m_dongtaiPort3.clear();

    // 第一步：检测端口是否存在且匹配
    m_dongtaiPort1 = checkPort(m_dongtaiPort1Index);
    m_dongtaiPort2 = checkPort(m_dongtaiPort2Index);
    m_dongtaiPort3 = checkPort(m_dongtaiPort3Index);

    qDebug() << "动态1:" << m_dongtaiPort1 << "动态2:" << m_dongtaiPort2 << "动态3:" << m_dongtaiPort3;

    if (m_dongtaiPort1.isEmpty() || m_dongtaiPort2.isEmpty() || m_dongtaiPort3.isEmpty()) {
        progressDialog.close();
        showAutoClosingMessageBox(this, "提示", "未能找到全部三个有效设备，请检查硬件连接！", 2500);
        return;
    }

    // 第二步：尝试打开所有串口（必须全部成功）
    QVector<mySerialPort*> tempPorts;
    QVector<QThread*> tempThreads;

    bool allOpened = true;
    QVector<QString> portNames = {m_dongtaiPort1, m_dongtaiPort2, m_dongtaiPort3};

    for (int i = 0; i < portNames.size(); ++i) {
        mySerialPort* serialPort = new mySerialPort(portNames[i], 115200, i);
        if (!serialPort->openPort()) {
            // 打开失败：记录错误并标记失败
            qDebug() << "串口打开失败:" << portNames[i];
            allOpened = false;
            // 清理已创建的对象
            for (auto* p : tempPorts) delete p;
            for (auto* t : tempThreads) {
                t->quit();
                t->wait();
                delete t;
            }
            delete serialPort;
            break;
        }

        // 连接信号槽
        connect(m_pSerialPortThread, &CSerialPortThread::signalCheckPort,
                serialPort, &mySerialPort::slotsCheckPort, Qt::QueuedConnection);
        connect(serialPort, &mySerialPort::dataReady,
                this, &MainWindow::onDataReceived_2, Qt::QueuedConnection);

        QThread* thread = new QThread();
        serialPort->moveToThread(thread);

        connect(thread, &QThread::finished, serialPort, &QObject::deleteLater);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        thread->start();

        tempPorts.append(serialPort);
        tempThreads.append(thread);
    }

    if (!allOpened) {
        progressDialog.close();
        showAutoClosingMessageBox(this, "错误", "部分串口打开失败，请检查设备连接或权限！", 2500);
        return;
    }

    // 第三步：全部成功，提交到管理器
    m_pSerialPortThread->deleteAll();
    for (auto* port : tempPorts) {
        m_pSerialPortThread->addPort(port);
    }
    serialPorts = tempPorts;
    serialThreads = tempThreads;

    m_pSerialPortThread->startCheck();
    progressDialog.close();

    ui->pushButton_2->setText("正在动态检测中");
    recordDataDongtai(); // 开始记录

}

// 处理接收到的数据
void MainWindow::onDataReceived(int portIndex, const QVector<int> &data)
{
    QString timeStamp = QDateTime::currentDateTime().toString("[hh:mm:ss.zzz]"); // 获取时间戳
    // 在调试模式下进行日志记录，避免影响性能
    #ifdef QT_DEBUG
    qDebug() << "Received data size from port" << portIndex << ":" << data.size();
    #endif

    // 检查 portIndex 是否超出 receivedData 和 receivedStatus 范围
    if (portIndex < 0 || portIndex >= receivedData.size()) {
        qDebug() << "Error: portIndex out of range";
        return;
    }

    // 存储接收的数据并记录状态
    receivedData[portIndex] = data;
    receivedStatus[portIndex] = true;

    #ifdef QT_DEBUG
    qDebug() << "Current receivedStatus:" << receivedStatus;
    #endif

    // 检查是否所有串口都接收到数据
    if (receivedStatus[0] && receivedStatus[1] && receivedStatus[2]) {
        QVector<int> combineData;


        // 在已知 QVector 或 QList 大小时使用 .reserve(size)，避免不必要的内存重新分配。
        combineData.reserve(receivedData[0].size() + receivedData[1].size() + receivedData[2].size());
        for (const auto &d : receivedData) {
            combineData += d;
        }

        #ifdef QT_DEBUG
        qDebug() << "Combined data size:" << combineData.size();
        #endif

        if (combineData.size() == m_draw_pt.size()) {
            #ifdef QT_DEBUG
            qDebug() << timeStamp << "drawPt_160 is called here,combineData:" << combineData;
            #endif

            // drawPt_160(combineData); // 绘制组合数据
            // emit dataReadyForDraw(combineData, m_draw_pt, _maxCount);

            // 检查保存功能是否开启
            if (!_recordFileName.isEmpty() && outputFile.isOpen()) {
                // 将数据包格式化为字符串并加入缓冲区
                QString dataLine = timeStamp;
                for (int value : combineData) {
                    dataLine += " " + QString::number(value);
                }
                dataBuffer.append(dataLine);

                // 每10包数据批量写入文件
                if (dataBuffer.size() >= batchSize) {
                    QTextStream out(&outputFile);
                    for (const QString &line : dataBuffer) {
                        out << line << "\n"; // 将缓冲区中的每一行数据写入文件
                    }
                    dataBuffer.clear(); // 清空缓冲区
                }
            }


        } else {
            qDebug() << "Mismatch in data size and m_draw_pt size.";
        }

        // 重置状态
        receivedStatus.fill(false);
    }
}

void MainWindow::onDataReceived_2(int portIndex, const QVector<int> &data)
{
    // 在调试模式下进行日志记录，避免影响性能
    #ifdef QT_DEBUG
    qDebug() << "Received data size from port" << portIndex << ":" << data.size();
    #endif

    // 检查 portIndex 是否超出 receivedData 和 receivedStatus 范围
    if (portIndex < 0 || portIndex >= receivedData.size()) {
        qDebug() << "Error: portIndex out of range";
        return;
    }

    // 存储接收的数据并记录状态
    receivedData[portIndex] = data;
    receivedStatus[portIndex] = true;

    #ifdef QT_DEBUG
    qDebug() << "Current receivedStatus:" << receivedStatus;
    #endif

    // 计算已接收到数据的串口数量
    int receivedPortCount = 0;
    for (int i = 0; i < receivedStatus.size(); ++i) {
        if (receivedStatus[i]) {
            ++receivedPortCount;
        }
    }
    qDebug()<<receivedPortCount<<"已接收到数据的串口数量";
    // 判断是否所有已选中的串口都接收到数据
    if (receivedPortCount >= selectedPortCount) {
        QVector<int> combineData;
        combineData.reserve(receivedPortCount * receivedData[0].size()); // 预分配所需的容量

        // 合并所有选中串口的数据
        for (int i = 0; i < receivedData.size(); ++i) {
            qDebug()<<i<<"个数";
            if (receivedStatus[i]) {
                combineData += receivedData[i];
            }
        }

        #ifdef QT_DEBUG
        //QString timeStamp = QDateTime::currentDateTime().toString("[hh:mm:ss.zzz]");
        qDebug() << "Combined data size:" << combineData.size();
        //qDebug() << timeStamp << "combineData:" << combineData;
        #endif

        // 将合并的数据添加到缓冲队列中
        dataQueue.enqueue(combineData);
        // 仅在数据达到批量大小时才发送到 DataFilterWorker
        if (dataQueue.size() >= 1) { // 10 根据发送频率调整
            QVector<QVector<int>> dataBatch;
            while (!dataQueue.isEmpty()) {
                dataBatch.push_back(dataQueue.dequeue());
            }
            emit newDataPacket(dataBatch); // 批量发送数据
        }

        //emit newDataPacket(combineData);  // 将数据发送到过滤线程

        // 重置状态
        receivedStatus.fill(false);
    }

}

void MainWindow::onDataReceived_Jingtai(int portIndex,const QVector<int> &data)
{
    qDebug()<<portIndex<<"onDataReceived_Jingtai";
    if(portIndex==1){

        QList<Point_commondata> posList;
        posList.reserve(1600);
        posList.clear();
        static std::vector<float>convertedPressure;
        convertedPressure.clear();
        convertedPressure.resize(1600); // 预分配空间
        // 2. 生成并绘制1600个点
        const int pointsPerRow = 40;
        const int spacing = 10;
         qDebug()<<  posList.size()<<"posList.size";
        for (int i = 0; i < data.size(); i++) {
            Point_commondata pos;
            pos.posX = 10 + (i % pointsPerRow) * spacing;
            pos.posY= 10 + (i / pointsPerRow) * spacing;
            pos.radius = 15;
           pos.count=convertData(data[i]);
           //pos.count=4095-data[i];
            convertedPressure[i] = std::max(pos.count, 0.0f);
            posList.push_back(pos);
        }
        mapper_40_->updateData(posList);
        recordDataJingTai(convertedPressure);

    }else{
        showAutoClosingMessageBox(this, "提示", "串口不匹配，请检查", 2500);

    }
}

void MainWindow::handleFilteredData(const QVector<double> &filteredData, double maxCount)
{


    emit sendDrawParam(filteredData, maxCount);


    QString timeStamp = QDateTime::currentDateTime().toString("[hh:mm:ss.zzz]");

    #ifdef  QT_DEBUG

    qDebug() << timeStamp << "filtered data:" << filteredData.size();

    #endif

    // 检查保存功能是否开启
    if (!_needRecord) return;

    // 将数据包格式化为字符串并加入缓冲区
    QString dataLine = timeStamp;
    for (double value : filteredData) {
        dataLine += " " + QString::number(value, 'f', 2); // 保留两位小数
    }
    dataBuffer.append(dataLine);


}


void MainWindow::drawPt_160_2(const QVector<double> &data)
{
    if (data.size() != m_draw_pt.size()) {
        qDebug() << "Error: Data size and m_draw_pt size do not match!";
        return;
    }

    QList<Point_commondata> posList;
    posList.reserve(data.size());
    float maxCount = 0.0;

    for (int i = 0; i < data.size(); ++i) {
        Point_commondata pos;
        pos.posX = m_draw_pt[i].x() + ImgWidth_main / 2;
        pos.posY = m_draw_pt[i].y() + ImgHeight_main / 2;
        pos.radius = 8;
        pos.count = data[i];
        maxCount = std::max(maxCount, pos.count);
        posList.append(pos);
    }

    emit sendDrawData(posList, maxCount * 0.8); // 发射信号，将绘图数据传递到 heatmapper
}

void MainWindow::onDrawCompleted(const QImage &image)
{

}

// 将缓冲区中的所有剩余数据写入文件
void MainWindow::flushDataBuffer() {
    if (!dataBuffer.isEmpty()) { //  && outputFile.isOpen()
        QTextStream out(&outputFile);
        for (const QString &line : dataBuffer) {
            out << line << "\n";
        }
        dataBuffer.clear();
    }
}

// 批量保存数据槽函数
void MainWindow::batchSaveDataToFile() {
    if (dataBuffer.isEmpty()) return; // 如果缓冲区为空则跳过

    emit saveDataToFile(dataBuffer); // 发送信号，将数据传递到保存线程
    dataBuffer.clear(); // 清空缓冲区
}

void MainWindow::onJingTaiCheckboxStateChanged()
{
    isJiangTaiTest=true;
}

void MainWindow::recordDataJingTai(std::vector<float> data)
{

    QString value;
    for(int i=0;i<data.size();i++){
        value.append(QString::number(data[i]));
        value.append(" ");
    }
    QString time_str =QDateTime::currentDateTime().toString("[HH:mm:ss:zzz]");

    //qDebug()<<m_recordProFileName<<"m_recordFileName";
    QFile file( globaldata::dataRecordPath_jingtai);

    if(!file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
        // QMessageBox::warning(this,"Waring","can't open",QMessageBox::Yes);
    }
    QTextStream stream(&file);
    stream<<time_str+" "+value<<"\n";
    file.close();

}

void MainWindow::stopRecordDataJingTai()
{
    if (fileSaveWorker) {
        fileSaveWorker->closeFile();
        fileSaveThread->quit();
        fileSaveThread->wait();
        delete fileSaveWorker;
        fileSaveWorker = nullptr;
    }else {
        qDebug()<<"关闭保存足底压力数据出错：保存线程未打开";
    }
}

void MainWindow::recordDataDongtai()
{
    // 选择文件路径
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy_MM_dd_hh_mm_ss");
    globaldata::dataRecordPath_dongtai.clear();

    globaldata::dataRecordPath_dongtai=QCoreApplication::applicationDirPath()+"/data_dongtai/"+current_date+"_dataRecord_Dongtai.txt";

    qDebug()<< globaldata::dataRecordPath_dongtai<<" globaldata::dataRecordPath_dongtai";

    if (!globaldata::dataRecordPath_dongtai.isEmpty()) {

        fileSaveWorker = new FileSaveWorker(globaldata::dataRecordPath_dongtai);
        fileSaveWorker->moveToThread(fileSaveThread);

        // 打开文件
        if (fileSaveWorker->openFile()) {
            connect(this, &MainWindow::saveDataToFile, fileSaveWorker, &FileSaveWorker::saveData);
            connect(dataSaveTimer, &QTimer::timeout, this, &MainWindow::batchSaveDataToFile);

            fileSaveThread->start();
            dataSaveTimer->start(5000); // 每 15 秒触发

            _needRecord = true;

            // recordFile->setText("关闭保存");
            // recordFile->setStyleSheet("QPushButton { background-color: #F19A38; }"); // 背景色为橙色；color: white;

        } else {
            delete fileSaveWorker;
            fileSaveWorker = nullptr;
        }
    }else{

        qDebug()<<"步态数据保存错误，文件名："<<globaldata::dataRecordPath_dongtai;
    }
}

void MainWindow::stopRecordDataDongtai()
{
    if (fileSaveWorker) {
        fileSaveWorker->closeFile();
        fileSaveThread->quit();
        fileSaveThread->wait();
        delete fileSaveWorker;
        fileSaveWorker = nullptr;
    }else {
        qDebug()<<"关闭保存步态数据出错：保存线程未打开";
    }
}

/**
 * @brief MainWindow::checkPort - 检测特定设备连接的串口
 * @param indexPort - 要匹配的设备标识符
 * @return 找到的串口名称，若未找到则返回空字符串
 *
 * 此函数遍历系统可用串口，通过发送特定指令或识别特定数据模式
 * 来定位与indexPort参数匹配的设备所在串口
 */
QString MainWindow::checkPort(int indexPort)
{
    // 获取系统所有可用串口信息
    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    // 遍历所有可用串口
    for(const QSerialPortInfo &port : ports) {
        // 使用智能指针管理串口对象，避免内存泄漏[1](@ref)
        QScopedPointer<QSerialPort> check_serial(new QSerialPort(this));

        // 配置串口参数
        check_serial->setPort(port); // 直接使用QSerialPortInfo对象设置端口信息
        check_serial->setBaudRate(QSerialPort::Baud115200);
        check_serial->setDataBits(QSerialPort::Data8);
        check_serial->setParity(QSerialPort::NoParity);
        check_serial->setStopBits(QSerialPort::OneStop);
        check_serial->setFlowControl(QSerialPort::NoFlowControl);

        // 尝试以读写方式打开串口
        if (!check_serial->open(QIODevice::ReadWrite)) {
            qDebug() << "无法打开串口:" << port.portName() << "错误:" << check_serial->errorString();
            continue; // 打开失败则跳过当前串口
        }

        // 设置超时时间（毫秒）
        const int timeout_ms =500;
        QElapsedTimer timer;
        timer.start();

        // 清空缓冲区，确保读取最新数据
        check_serial->clear();

        // 异步等待数据到达（非阻塞方式）
        while (timer.elapsed() < timeout_ms) {
            if (check_serial->waitForReadyRead(50)) { // 较小的时间片减少阻塞
                QByteArray data = check_serial->readAll();
                qDebug()<<port.portName()<<data.size()<<"串口数据量";
                LOG_DEBUG(QString("当前串口名： %1 ,当前串口接收到的数据量： %2 ").arg(port.portName()).arg(QString::number(data.size())));
                // 检查数据长度和起始标志
                // LOG_DEBUG(QString("当前串口名： %1 ,当前串口起始数据data[0]： %2 ,"
                //                  "当前串口起始数据data[1]： %3 ,当前串口起始数据data[2]： %4 ,当前串口起始数据data[3]： %5")
                //              .arg(port.portName()).arg(data.at(0));d
                qDebug()<<static_cast<unsigned char>(data[0])<<static_cast<unsigned char>(data[1])<<static_cast<unsigned char>(data[2])<<
                    static_cast<unsigned char>(data[3])<<"串口数据量qish";

                if (data.size() >= 3206 && data.startsWith("\x5A\x01\x95\x6C")) {

                    int value=data[5];
                    qDebug()<<value<<indexPort<<"串口索引";
                    LOG_DEBUG(QString("当前串口名： %1 ,当前串口接收到的索引： %2 ，当前设置的串口索引： %3 ").arg(value).arg(indexPort));
                    // 检查是否匹配目标设备
                    if (indexPort == value) {
                        check_serial->close();
                        qDebug()<<port.portName()<<"可用串口";
                        LOG_DEBUG(QString("[mainwindow]: 找到匹配设备，返回串口名,串口名： %1， 串口索引：  ").arg(port.portName()).arg(indexPort));
                        return port.portName(); // 找到匹配设备，返回串口名
                    }
                }
            }
            QCoreApplication::processEvents(); // 保持UI响应性[6](@ref)
        }

        // 超时或未匹配，关闭当前串口
        check_serial->close();
    }

    return QString(); // 未找到匹配设备，返回空字符串
}

bool MainWindow::validateFrame(const QByteArray &frame)
{
    // 实现数据验证逻辑
    return frame.size() >= 3206 && frame.startsWith("\x5A\x01\x95\x6C");
}

void MainWindow::showMinwindow()
{
    this->show();
}



void MainWindow::stopSerial()
{
    for (int i = 0; i < serialPorts.size(); ++i) {
        auto *port = serialPorts[i];
        auto *thread = serialThreads[i];

        if (port) {
            //port->closePort();
            // 使用信号请求关闭串口，确保串口的关闭操作在正确的线程中完成
            QMetaObject::invokeMethod(port, "requestClose", Qt::QueuedConnection);
        }

        if (thread) {
            thread->quit(); // 请求线程退出
            thread->wait(); // 等待线程完成停止
            delete thread; // 手动释放线程
        }

        //delete port; // 手动释放资源
        //delete thread; // 手动释放线程
    }

    serialPorts.clear();
    serialThreads.clear();
    receivedStatus.fill(false);
    stopRecordDataDongtai();//关闭保存动态数据

    showAutoClosingMessageBox(this, "提示", "串口已关闭，即将自动关闭提示框", 1500);
}
// ...

void MainWindow::test2()
{
}

void MainWindow::test3(int n)
{
    qDebug() << "================================================callback n:"<<n;
}


void MainWindow::searchSerialPort()
{
    PortBox1->clear();
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {

        PortBox1->addItem(info.portName());// 添加计算机中的端
    }
}

void MainWindow::searchSerialPort2()
{
    PortBox2->clear();
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {

        PortBox2->addItem(info.portName());// 添加计算机中的端
    }
}

void MainWindow::searchSerialPort3()
{
    PortBox3->clear();
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {

        PortBox3->addItem(info.portName());// 添加计算机中的端
    }
}

void MainWindow::searchSerialPort4()
{
    PortBox4->clear();
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {

        PortBox4->addItem(info.portName());// 添加计算机中的端
    }
}

void MainWindow::openSerialPort()
{
    bool ok;
    if (openSerial->text() == QString("打开串口")) {
        QString portName = PortBox1->currentText();
        int baudRate = BaudBox->currentText().toInt(&ok, 10); // 假设ok是bool变量
        if (!ok || baudRate <= 0) {
            QMessageBox::warning(this, "错误", "无效的波特率设置");
            return;
        }

        canshuInit(); // 假设这个函数是初始化一些与串口通信相关的参数
        //currentTime->start(); // 假设这是一个用于计时的QTimer

        m_com = new QSerialPort(); // 注意：将m_com设置为MainWindow的子对象，以便自动管理内存
        m_com->setPortName(portName);
        m_com->setBaudRate(baudRate);
        m_com->setDataBits(QSerialPort::Data8);
        m_com->setStopBits(QSerialPort::OneStop);
        m_com->setParity(QSerialPort::NoParity);
        m_com->setFlowControl(QSerialPort::NoFlowControl);

        if (m_com->open(QIODevice::ReadWrite)) {
            sendToSensor = true;
            sendData(); // 假设这是发送数据的函数,2024/10/1


            // 禁用UI元素
            PortBox1->setEnabled(false);
            BaudBox->setEnabled(false);
            openSerial->setText("关闭串口");
        } else {
            QMessageBox::warning(this, "提示", "串口打开失败");
            delete m_com; // 如果open失败，则手动删除m_com（但在这里更推荐设置为nullptr并在析构函数中处理）
            m_com = nullptr; // 防止野指针
            return;
        }
    } else {
        m_com->close();
        // 假设m_timerMain是用于主循环的QTimer，如果需要的话可以在这里停止它
       m_timerMain->stop();

        // 清理资源
        m_com->deleteLater(); // 或者设置为nullptr并在MainWindow的析构函数中删除
        m_com = nullptr; // 防止野指针

        // 启用UI元素
        PortBox1->setEnabled(true);
        BaudBox->setEnabled(true);
        openSerial->setText("打开串口");
        sendToSensor = false; // 重置发送标志

        // 更新UI（如果需要的话）
         this->update(); // 通常不需要在这里调用，除非有特定的绘图需求
    }
}



QString MainWindow::byteArrayToHexStr(const QByteArray &data)
{
    QString temp = "";
    QString hex = data.toHex();
    for (int i = 0; i < hex.length(); i = i + 2) {
        temp += hex.mid(i, 2) + " ";
    }

    return temp.trimmed().toUpper();
}

QByteArray MainWindow::hexStrToByteArray(const QString &data)
{
    QByteArray senddata;
    int hexdata, lowhexdata;
    int hexdatalen = 0;
    int len = data.length();
    senddata.resize(len / 2);
    char lstr, hstr;

    for (int i = 0; i < len;) {
        hstr = data.at(i).toLatin1();
        if (hstr == ' ') {
            i++;
            continue;
        }

        i++;
        if (i >= len) {
            break;
        }
        lstr = data.at(i).toLatin1();
        hexdata = hexStrToChar(hstr);
        lowhexdata = hexStrToChar(lstr);

        if ((hexdata == 16) || (lowhexdata == 16)) {
            break;
        } else {
            hexdata = hexdata * 16 + lowhexdata;
        }

        i++;
        senddata[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }

    senddata.resize(hexdatalen);
    return senddata;
}

char MainWindow::hexStrToChar(char data)
{
    if ((data >= '0') && (data <= '9')) {
        return data - 0x30;
    } else if ((data >= 'A') && (data <= 'F')) {
        return data - 'A' + 10;
    } else if ((data >= 'a') && (data <= 'f')) {
        return data - 'a' + 10;
    } else {
        return (-1);
    }
}

void MainWindow::recordData(QString data)
{
    QDateTime da_time;

    QString time_str = da_time.currentDateTime().toString("[yyyy-MM-dd HH:mm:ss]");

    QString runPath = QCoreApplication::applicationDirPath();
    QFile file(_recordFileName);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
        QMessageBox::warning(this,"Waring","can't open",QMessageBox::Yes);
    }
    QTextStream stream(&file);
    stream<<time_str+":"+data<<"\n";
    file.close();
}

void MainWindow::sendData()
{
    m_com->clear();
    dataToSensor("5A");
    m_timerMain->start(50);//0827:50
}

void MainWindow::dataToSensor(QString data)
{
    if (m_com == 0 || !m_com->isOpen()) {
        return;
    }

    //短信猫调试
    if (data.startsWith("AT")) {
        data += "\r";
    }

    QByteArray buffer;

    buffer = hexStrToByteArray(data);

    //qDebug()<<buffer<<"nbisdf";
    m_com->write(buffer);
}


void MainWindow::recordReportSlot()
{
    m_showreport=new showReport();


    connect(m_showreport, &showReport::hideAndShowMain_report,
            this, &MainWindow::showMinwindow, Qt::QueuedConnection);
    m_showreport->show();
    this->hide();
    m_showreport->reseiveShowReport(globaldata::reportPath_jingtai,0);

}

void MainWindow::onSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::DeviceNotFoundError) {
        // 设备未找到，可能是串口已断开
        //qDebug() << "串口设备未找到，尝试重新连接...";
        // 在这里关闭串口（如果它仍然打开的话），然后重新打开
        if (m_com->isOpen()) {
            m_com->close();
        }
        // 重新配置并尝试打开串口（可能需要一些延迟）
        // ...
    }
    // 处理其他错误...
}





void MainWindow::drawPt_160(QVector<int> data)
{
#ifdef QT_DEBUG
    qDebug() << "data size:" << data.size() << ", m_draw_pt size:" << m_draw_pt.size();
#endif
    // 确保 data 和 m_draw_pt 的大小匹配--1031
    if (data.size() > m_draw_pt.size()) {
        qDebug() << "Error: Data size and m_draw_pt size do not match!";
        return;
    }

    static bool isDrawing = false;
    if (isDrawing) return; // 如果正在绘图，跳过新的绘图请求

    isDrawing = true;

    //0827测试  40*120

    _maxCount=0.0;
    QList<Point_commondata> posList;
    QString recorddata;
    posList.reserve(data.size()); // 在已知所需大小时，提前使用 QVector::reserve--1101

#ifdef QT_DEBUG
    qDebug()<<data.length()<<"changdu";
#endif

    for (int i = 0; i < data.size(); i++) {

        // 调试--1031
        if (i >= m_draw_pt.size()) {
            qDebug() << "Index out of range:" << i << "for m_draw_pt";
            return;
        }

        Point_commondata pos;
        pos.posX = m_draw_pt[i].x()+ImgWidth_main/2 ;
        pos.posY = m_draw_pt[i].y()+ImgHeight_main/2 ;
        pos.radius = 8;

        if(data[i]!=0){

            if(0<data[i]&&data[i]<=2170){
                pos.count=-0.0854525*data[i]+332.9351;
            }else if(2171<=data[i]&&data[i]<=2380){
                pos.count=-0.0174325*data[i]+524.394462;
            }else if(2381<=data[i]&&data[i]<=3780){
                pos.count=-0.06659*data[i]+265.145;
            }else if(3781<=data[i]&&data[i]<=4102){
                pos.count=-0.4414*data[i]+180.954;
            }



            _maxCount = std::max(_maxCount, pos.count);
        }
        else{
            pos.count=0;
        }


        posList.push_back(pos);
        recorddata.append(QString::number(pos.count)).append(" ");

    }

    // 是否需要保存文件
    if(_needRecord){
        recordData(recorddata);
    }

    _maxCount=_maxCount*0.8;

    //发送信号，将 posList 和 _maxCount 传递出去--1030
    emit dataReceived(posList, _maxCount);

    isDrawing = false;

    // mapper_->drawImage(posList,_maxCount);
    // this->update();
}

// 数据转换逻辑--1101
float MainWindow::convertData(int value)
{
    if (value > 0 && value <= 3059) {
        return -0.10291 * value + 355.24177;
    } else if (value > 1623 && value <= 3346) {
        return -0.05914 * value + 221.35382;
    } else if (value > 2378 && value <= 4095) {
        return -0.03048 * value + 125.55904;
    }else {
        return 0; // 超出范围的数值返回0或适当的默认值
    }

}

QVector<QPoint> MainWindow::intialPtSize(int x, int y, int x_dis, int y_rad)
{
    //intialPtSize(120,40,11,7)
    QVector<QPoint> draw_pt;
    int n_x_sum = 0;
    int n_y_sum = 0;
    for (int i = 0; i < y; i++) {
        // Point mid_pt;
        for (int j = 0; j < x; j++) {
            draw_pt.push_back(QPoint(j*x_dis, i*y_rad));
            n_x_sum += j*x_dis;
            n_y_sum += i*y_rad;
        }
    }
    // //qDebug()<<n_x_sum<<n_y_sum<<"n_y_sum";
    float n_x_ = n_x_sum / (x*y);
    float n_y_ = n_y_sum / (x*y);
    for (int i = 0; i < draw_pt.size(); i++) {
        draw_pt[i].setX(draw_pt[i].x() - n_x_);
        draw_pt[i].setY(draw_pt[i].y() - n_y_);
    }

    qDebug() << "Initialized m_draw_pt size:" << draw_pt.size(); // 调试--1031

    return draw_pt; //m_draw_pt：对应坐标点
}

void MainWindow::canshuInit()
{
    sum_left=0;  //该测试过程中脚的个数
    sum_right=0;

    max_mmghValue_left=0.0;
    min_mmghValue_left=30.0;

    max_mmghValue_right=0.0;
    min_mmghValue_right=30.0;

    aveNum_left=0.0;
    aveNum_right=0.0;
    sumNum_left=0.0;
    sumNum_right=0.0;
    num_left=0;
    num_right=0;
    num_all=0;
    area_all=0;
    foot_log=0; //0：右脚 1：左脚
    //左右脚的步数
    left_num=0;
    right_num=0;

    //步频
    frequentcy=0.0;
    //单步时间
    singleStepTime=0;
}

void MainWindow::initData()
{
    //uiInit
    // ui->label_busu->setText("步速：" );
    // ui->label_bupin->setText("步频：" );
    // ui->label_bukuan->setText("步宽：" );
    // ui->label_buchang->setText("步长：");
    // ui->label_yichangzhi->setText("步长变异性：");
    // ui->label_bukuanbianyi->setText("步宽变异性：" );
    // ui->label_shuangxiangbianyi->setText("双向支撑变异性：" );

    QString m_onnxfile=QCoreApplication::applicationDirPath()+"/model.onnx";
    globaldata::m_onnx=m_onnxfile.toStdString();
    qDebug()<<globaldata::m_onnx<<"globaldata::m_onnx";
}





void MainWindow::on_pushButton_clicked()
{
    //202508三合一步态需求 增加足底压力实施显示功能
    if(ui->pushButton->text()=="开始静态测试"){

        // 创建进度提示框
        QProgressDialog progressDialog(this);
        progressDialog.setWindowTitle("提示"); // 设置窗口标题
        progressDialog.setLabelText("设备连接中，请稍等...");
        progressDialog.setCancelButton(nullptr); // 不允许取消
        progressDialog.setWindowModality(Qt::ApplicationModal); // 设置为模态
        progressDialog.setMinimumDuration(0); // 立即显示
        progressDialog.setRange(0, 0); // 设置不确定的进度范围
        // 手动设置布局以居中对齐
        QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(progressDialog.layout());
        if (layout) {
            layout->setAlignment(Qt::AlignCenter); // 设置垂直居中
        }
        progressDialog.show();
        // 手动触发事件循环以更新提示框
        QCoreApplication::processEvents();


        QString jingtaiPortName= checkPort(m_jingtaiPortIndex);
        if (!jingtaiPortName.isEmpty()) {
            qDebug()<<"静态测试串口："<<jingtaiPortName;
            // 创建串口对象
            //m_pSerialPortThread->deleteAll();   静态id  01

            mySerialPort *JingTaiserialPort = new mySerialPort(jingtaiPortName, 115200, 1);
            serialPort_JingTai=JingTaiserialPort;
            if(!serialPort_JingTai->openPort()){

                showAutoClosingMessageBox(this, "提示", "未打开有效串口，即将自动关闭提示框", 1500);
            }
            // connect(m_pSerialPortThread, &CSerialPortThread::signalCheckPort, serialPort_JingTai, &mySerialPort::slotsCheckPort, Qt::QueuedConnection);
            // m_pSerialPortThread->addPort(serialPort_JingTai);

            // 接收数据后将数据和序号传递给处理函数
            connect(serialPort_JingTai, &mySerialPort::dataReady, this, &MainWindow::onDataReceived_Jingtai, Qt::QueuedConnection);
           // ui->pushButton->setStyleSheet("QPushButton { background-color: #76CE6E; }"); // 背景色为绿色
            ui->pushButton->setText("正在检测中");
           // m_pSerialPortThread->startCheck();
        }
        // 关闭提示框
        progressDialog.close();

        // 选择文件路径
        QDateTime current_date_time = QDateTime::currentDateTime();
        QString current_date = current_date_time.toString("yyyy_MM_dd_hh_mm_ss");
        globaldata::dataRecordPath_jingtai.clear();
        globaldata::dataRecordPath_jingtai=QCoreApplication::applicationDirPath()+"/data_jingtai/"+current_date+"_dataRecord_Jingtai.txt";
        //qDebug()<<globaldata::dataRecordPath_jingtai<<"globaldata::dataRecordPath_jingtai";
         LOG_DEBUG("点击开始静态测试时生成的保存数据的文件地址"+globaldata::dataRecordPath_jingtai);


    }
    else{
        ui->pushButton->setText("开始静态测试");

      //  ui->pushButton->setStyleSheet("QPushButton { background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,stop:0.4 rgb(87, 154, 243),stop:0.9 rgb(154,78, 255));}"); // 背景色为绿色
        QMetaObject::invokeMethod(serialPort_JingTai, "requestClose", Qt::QueuedConnection);
        showAutoClosingMessageBox(this, "提示", "串口已关闭，即将自动关闭提示框", 1500);

    }

}


void MainWindow::on_pushButton_2_clicked()
{
    // this->testTemp();
    if(ui->pushButton_2->text()=="开始动态测试"){
          startSerial(); // 打开串口--1031
    }else if(ui->pushButton_2->text()=="正在动态检测中"){

        ui->pushButton_2->setText("开始动态测试");
        m_pSerialPortThread->stopCheck();
        m_pSerialPortThread->deleteAll();
        stopSerial();  // 关闭串口--1031
        //ui->pushButton_2->setStyleSheet("");
        //2025_08_15_12_22_20_dataRecord_Dongtai   //君乐采集的实际数据2025_08_15_14_27_33_dataRecord_Dongtai  //串口模拟数据 2025_08_14_21_58_16_dataRecord_Dongtai
      //globaldata::dataRecordPath_dongtai= QCoreApplication::applicationDirPath()+"/data_dongtai/"+"2025_08_15_14_27_33_dataRecord_Dongtai.txt";
        //3m数据 来源于张鑫
     // globaldata::dataRecordPath_dongtai= QCoreApplication::applicationDirPath()+"/data_dongtai/"+"2025_09_23_11_12_46_dataRecord_Dongtai.txt";
          // qDebug()<<globaldata::dataRecordPath_dongtai<<"解析globaldata::dataRecordPath_jingtai";
        LOG_DEBUG("进行步态数据解析的数据文件路径"+globaldata::dataRecordPath_dongtai);

        if (!QFile::exists(globaldata::dataRecordPath_dongtai)) {
            QMessageBox::warning(this, "错误", "动态数据文件不存在：\n" + globaldata::dataRecordPath_dongtai);
            return;
        }
        if (globaldata::m_onnx.empty()) {
            QMessageBox::warning(this, "错误", "ONNX模型路径未配置！");
            return;
        }
        try {

            m_result = dongtai_analyce(globaldata::dataRecordPath_dongtai.toStdString());


        } catch (const std::exception &e) {
            QMessageBox::critical(this, "错误", "数据解析失败：\n" + QString::fromStdString(e.what()));

            return;
        }

        if (!m_result.backward.success) {
            QMessageBox::critical(this, "错误", "动态数据解析返回空结果！");
            return;
        }else{
            isResult=true;
            QString points_left;
            points_left.clear();

            for(int  i=0;i<m_result.lefts.size();i++){
                points_left.append(QString("  第  %d  步: ").arg(i));
                for(int n=0;n<m_result.lefts.at(i).cop.size();n++){
                    QString st;
                    st.append("( ");
                    st.append(QString::number(m_result.lefts.at(i).cop.at(n).x)+" ,");
                    st.append(QString::number(m_result.lefts.at(i).cop.at(n).y));
                    st.append(") ");
                    points_left.append(st);
                }
         }
            //qDebug()<<m_result.cop_area<<points<<"动态中心坐标points";

            LOG_DEBUG("步态cop轨迹坐标："+points_left);
        }

        }

}



//解析动态步态数据

void MainWindow::on_pushButton_4_clicked()
{
    //测试cop数据
    isResult=true;
   //globaldata::dataRecordPath_dongtai= QCoreApplication::applicationDirPath()+"/data_dongtai/"+"2025_09_23_11_12_46_dataRecord_Dongtai.txt";
    if (!globaldata::dataRecordPath_dongtai.isEmpty()) {

        // 读取输入框数据
        globaldata::user_id = ui->lineEdit_id->text().trimmed();
        globaldata::user_name = ui->lineEdit_name->text().trimmed();
        globaldata::user_gender = ui->lineEdit_gender->text().trimmed();
        int height = ui->lineEdit_height->text().toInt();  // 身高（QString -> int）
        int weight = ui->lineEdit_kg->text().toInt();  // 体重（QString -> int）
        int age = ui->lineEdit_age->text().toInt();        // 年龄（QString -> int）

        // 赋值给全局变量（globaldata 中这三个变量为 int 类型）
        globaldata::user_height = height;
        globaldata::user_weight = weight;
        globaldata::user_age = age;
        if (globaldata::user_height > 0) {
            double heightM = globaldata::user_height / 100.0;  // int -> double（关键：用 100.0 确保浮点数除法）
            globaldata::user_bmi = static_cast<float>(
                globaldata::user_weight / (heightM * heightM)   // int / double -> float
                );
        }

      // globaldata::dataRecordPath_dongtai= QCoreApplication::applicationDirPath()+"/data_dongtai/"+"2025_09_23_11_12_46_dataRecord_Dongtai.txt";
       //  qDebug()<<globaldata::dataRecordPath_dongtai<<"解析globaldata::dataRecordPath_jingtai";
        LOG_DEBUG("进行步态数据解析的数据文件路径"+globaldata::dataRecordPath_dongtai);
        if (!globaldata::dataRecordPath_dongtai.isEmpty()) {
            m_result = dongtai_analyce(globaldata::dataRecordPath_dongtai.toStdString());
            isResult=true;
            QString points_left;
            points_left.clear();

            // for(int  i=0;i<m_result.lefts.size();i++){
            //     points_left.append(QString("  第  %1  步: ").arg(i));
            //     qDebug()<<"m_result.lefts.at(i).cop.size():"<< m_result.lefts.at(i).cop.size()<<
            //         m_result.rights.at(i).cop.size();
            //     for(int n=0;n<m_result.lefts.at(i).cop.size();n++){
            //         QString st;
            //         st.append("( ");
            //         st.append(QString::number(m_result.lefts.at(i).cop.at(n).x)+" ,");
            //         st.append(QString::number(m_result.lefts.at(i).cop.at(n).y));
            //         st.append(") ");
            //         qDebug()<<st<<"st";
            //         points_left.append(st);
            //     }
            // }
            // qDebug()<<points_left<<"动态中心坐标points";

            // LOG_DEBUG("步态cop轨迹坐标："+points_left);

        }

        main_Form_blockdata=new Form_blockdata();
        connect(main_Form_blockdata, &Form_blockdata::hideAndShowMain_block,
                this, &MainWindow::showMinwindow, Qt::QueuedConnection);
        main_Form_blockdata->readBlockData(m_result);
        main_Form_blockdata->show();
        this->hide();

        main_Form_blockdata->recordReportDongtai();

    }else{
            QMessageBox::warning(this, "警告", "未选择有效数据文件！");
            return;
    }

}

//解析静态数据
void MainWindow::on_pushButton_5_clicked()
{
    // === 1. 输入校验 ===
    // QString id = ui->lineEdit_id->text().trimmed();
    // QString name = ui->lineEdit_name->text().trimmed();
    // QString gender = ui->lineEdit_gender->text().trimmed();

   bool okHeight, okWeight, okAge;
   int height = ui->lineEdit_height->text().toInt(&okHeight);
     int weight = ui->lineEdit_kg->text().toInt(&okWeight);
    int age = ui->lineEdit_age->text().toInt(&okAge);

    // if (id.isEmpty() || name.isEmpty() || gender.isEmpty()) {
    //     QMessageBox::warning(this, "输入错误", "请填写完整用户信息！");

    //     return;
    // }

    // if (!okHeight || !okWeight || !okAge || height <= 0 || weight <= 0 || age <= 0) {
    //     QMessageBox::warning(this, "输入错误", "身高、体重、年龄必须为正整数！");

    //     return;
    // }



    // 赋值给全局变量（globaldata 中这三个变量为 int 类型）


    globaldata::user_id=ui->lineEdit_id->text().trimmed();
    globaldata::user_name=ui->lineEdit_name->text().trimmed();
    globaldata::user_gender=ui->lineEdit_gender->text().trimmed();

    globaldata::user_height = height;
    globaldata::user_weight = weight;
    globaldata::user_age = age;
    if (globaldata::user_height > 0) {
        double heightM = globaldata::user_height / 100.0;  // int -> double（关键：用 100.0 确保浮点数除法）
        globaldata::user_bmi = static_cast<float>(
            globaldata::user_weight / (heightM * heightM)   // int / double -> float
            );
    }

    //获取足底压力图像

    globaldata::image_zudi=mapper_40_->m_mainCanvas;
    //20250813 HC自动读取数据及模型文件 2024_11_18_18_05_05
   //globaldata::dataRecordPath_jingtai= QCoreApplication::applicationDirPath()+"/data_jingtai/"+"2024_11_18_18_05_05.txt";
        // qDebug()<<globaldata::dataRecordPath_jingtai<<"解析globaldata::dataRecordPath_jingtai";
    LOG_DEBUG("进行足底压力数据解析的数据文件路径"+globaldata::dataRecordPath_jingtai);
    if (!globaldata::dataRecordPath_jingtai.isEmpty()) {
        QFileInfo fileInfo(globaldata::dataRecordPath_jingtai);

        if (globaldata::m_onnx.empty()) {
            QMessageBox::warning(this, "警告", "未选择模型文件！");
            return;
        }

        // 创建进度提示框
        QProgressDialog progressDialog(this);
        progressDialog.setWindowTitle("提示"); // 设置窗口标题
        progressDialog.setLabelText("数据处理中，请稍等...");
        progressDialog.setCancelButton(nullptr); // 不允许取消
        progressDialog.setWindowModality(Qt::ApplicationModal); // 设置为模态
        progressDialog.setMinimumDuration(0); // 立即显示
        progressDialog.setRange(0, 0); // 设置不确定的进度范围
        // 手动设置布局以居中对齐
        QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(progressDialog.layout());
        if (layout) {
            layout->setAlignment(Qt::AlignCenter); // 设置垂直居中
        }
        progressDialog.show();
        // 手动触发事件循环以更新提示框
        QCoreApplication::processEvents();

        try {
            m_JingtaiResult=jingtai_analyce(globaldata::dataRecordPath_jingtai.toStdString(), globaldata::m_onnx);
            //  analyzeSuccess = true;
        } catch (const std::exception &e) {
            // 关闭提示框
            progressDialog.close();
            QMessageBox::critical(this, "错误", "静态数据解析失败：\n" + QString::fromStdString(e.what()));

            return;
        }

        if (!m_JingtaiResult.backward.success) {
            // 关闭提示框
            progressDialog.close();
            QMessageBox::critical(this, "错误", "静态数据解析返回空结果！");
            return;
        }else{
            main_Form_processdata=new Form_processdata();

            //hideAndShowMain_process
            // 连接信号槽
            connect(main_Form_processdata, &Form_processdata::hideAndShowMain_process,
                    this, &MainWindow::showMinwindow, Qt::QueuedConnection);
            //    main_Form_processdata->readProcessdata(
            //        "C:\\Users\\Lenovo\\Desktop\\cmakeTest\\data\\2024_11_18_18_05_05.txt","C:\\Users\\Lenovo\\Desktop\\cmakeTest\\cmake_test1117\\model.onnx");

            //main_Form_processdata->readProcessdata("C:\\Projects\\data\\2024_11_18_18_05_05.txt", "C:\\Projects\\data\\model.onnx"); // zhw


            main_Form_processdata->readProcessdata(m_JingtaiResult);
            // 关闭提示框
            progressDialog.close();

            main_Form_processdata->show();
             this->hide();
            main_Form_processdata-> recordReportJingtai();


        }


    }else{

        QMessageBox::warning(this, "警告", "未选择有效数据文件！");
        return;
    }


}

void MainWindow::on_main_restore_clicked()
{
    ui->main_restore->setVisible(false);
    ui->main_max->setVisible(true);
    setWindowState(Qt::WindowNoState);
    this->update();
}


void MainWindow::on_main_min_clicked()
{
    this->showMinimized();
    this->update();
}


void MainWindow::on_main_max_clicked()
{
    ui->main_restore->setVisible(true);
    ui->main_max->setVisible(false);
    this->setWindowState(Qt::WindowMaximized);
    this->showMaximized();
    setCursor(Qt::ArrowCursor);
    this->update();
}


void MainWindow::on_main_close_clicked()
{
     this->close();
}



bool MainWindow::LeftBorderHit(const QPoint &pos)
{
    const QRect& rect = this->geometry();
    if(pos.x() >= rect.x() && pos.x() <= (rect.x() + CONST_DRAG_BORDER_SIZE)){
        return true;
    }

    return false;
}

bool MainWindow::RightBorderHit(const QPoint &pos)
{
    const QRect& rect = this->geometry();
    int nPosX = rect.x()+rect.width();
    if(pos.x() <= nPosX && pos.x() >= (nPosX - CONST_DRAG_BORDER_SIZE)){
        return true;
    }

    return false;
}

bool MainWindow::TopBorderHit(const QPoint &pos)
{
    const QRect& rect = this->geometry();
    if(pos.y() > rect.y() && pos.y() <= (rect.y()+CONST_DRAG_BORDER_SIZE)){
        return true;
    }

    return false;
}

bool MainWindow::BottomBorderHit(const QPoint &pos)
{
    const QRect& rect = this->geometry();
    int nPosY =rect.y()+rect.height();
    if(pos.y() < nPosY && pos.y() >= (nPosY-CONST_DRAG_BORDER_SIZE)){
        return true;
    }

    return false;
}



bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseMove){
        if(isMaximized()){
            return QWidget::eventFilter(obj,event);
        }
        QMouseEvent* pMouse = dynamic_cast<QMouseEvent*>(event);
        if(pMouse){
            checkBorderDragging(pMouse);
            if(obj == ui->frame && m_bMousePressedTitle){
                if(cursor() == Qt::ArrowCursor){
                    move(m_wndPos + (pMouse->globalPosition().toPoint() - m_mousePos));
                }
            }
        }
    }else if(event->type() == QEvent::MouseButtonPress && obj == ui->frame){
        QMouseEvent* pMouse = dynamic_cast<QMouseEvent*>(event);
        if(pMouse){
            this->m_bMousePressedTitle = true;
            this->m_mousePos = pMouse->globalPosition().toPoint();
            this->m_wndPos = this->pos();
        }
    }else if(event->type() == QEvent::MouseButtonRelease && obj == ui->frame){
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
    }else if(event->type() == QEvent::MouseButtonDblClick && obj == ui->frame){
        if(windowState().testFlag(Qt::WindowNoState)){
            this->on_main_max_clicked();
        }else if(windowState().testFlag(Qt::WindowMaximized)){
            this->on_main_restore_clicked();
        }
    }


    return QWidget::eventFilter(obj,event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
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

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
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

void MainWindow::checkBorderDragging(QMouseEvent *event)
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

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape){
        if(this->isMaximized()){
            on_main_restore_clicked();
            return;
        }
    }
    QWidget::keyPressEvent(ev);
}




