#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include<QTimer>
#include <QMainWindow>
 
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
//#include"showimage.h"
#include<QComboBox>
#include<QLabel>
#include<QDateTime>
#include<QPushButton>
#include<QFileDialog>
#include"heartmap.h"
#include<QElapsedTimer>
#include<QMessageBox>
#include "mapper_40.h"
//#include "include/algodll.h"
#include"process.h"

// 1030
#include <QFile>
#include <QThread>
#include <QVector>
#include <QList>
#include <QImage>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QQueue>
#include "myserialport.h"
#include "globalfunctions.h"
// #include "drawworker.h"
#include "datafilterworker.h"
#include "filesaveworker.h"
#include"form_blockdata.h"
#include"form_processdata.h"
#include "log.h"
#include"showreport.h"
#include "serial_port_thread.h"
class showReport;
class Form_blockdata;
class heatmapper;
class mapper_40;
QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr, double scaleFactor = 1.0);
    ~MainWindow();
    
    // 分辨率适配缩放比例
    double m_scaleFactor;
    
    QAction* setPercentageAction;
    QSerialPort* serial;
    QPushButton* portBox1;
    QComboBox* PortBox1;
    QPushButton* portBox2;
    QComboBox* PortBox2;
    QPushButton* portBox3;
    QComboBox* PortBox3;
    QPushButton* portBox4; //静态测试
    QComboBox* PortBox4;
    QLabel* baudBox;
    QComboBox* BaudBox;
    QPushButton* openSerial;
    //QPushButton* recordFile;
   // QPushButton* recordReport;
    bool m_comOK;
    bool sendToSensor;

    enum {
        CANVAS_WIDTH  = 800,
        CANVAS_HEIGHT = 800,
        DEFAULT_RADIUS = 15,
        DEFAULT_OPACITY = 255,
        DEFAULT_WIDTH = 255
    };

    //mapper_1
    int CANVAS_WIDTH_1=210;
    int CANVAS_HEIGHT_1=850;

    QTimer *m_timerMain;
    QSerialPort *m_com;
    QVector<int> m_data_init;
    QVector<int> m_data_init_40;
    QVector<int> m_data;
    //保存文件
    bool _needRecord = false;
    QString _recordFileName;
    int m_pt_size=1600;   //传感器数据点数
    int m_cur_dataNum;
    bool m_isFinsh;

    float trimIndex=0.00 ; // 前10%的索引
    float endTrimIndex=0.00;
    float  _maxCount=0.00;
    int  x_num_main=120;  //传感点120*40
    int  y_num_main=40;
    int  m_rad_main=8;
    int  m_dis_main=20;

    int  x_num_40=40;  //传感点40*40
    int  y_num_40=40;

    QElapsedTimer* currentTime;
    int sum_left=0;  //该测试过程中脚的个数
    int sum_right=0;

    qreal max_mmghValue_left=0.0;
    qreal min_mmghValue_left=30.0;
    QPoint maxValueLocation_left;
    QPoint minValueLocation_left;

    qreal max_mmghValue_right=0.0;
    qreal min_mmghValue_right=30.0;
    QPoint maxValueLocation_right;
    QPoint minValueLocation_right;

    qreal aveNum_left=0.0;
    qreal aveNum_right=0.0;
    qreal sumNum_left=0.0;
    qreal sumNum_right=0.0;
    int num_left=0;
    int num_right=0;
    int num_all=0;
    float area_all=0;

    int foot_log=0; //0：右脚 1：左脚
    //左右脚的步数
    int left_num=0;
    int right_num=0;

    //步频
    float frequentcy=0.0;
    //单步时间
    float singleStepTime=0.0;
    QVector<QPoint> m_draw_pt;
    QVector<QPoint> m_draw_pt_40;
    Point_commondata Point_main;

    QMap<int,double> left_clusterNumStartTime; //每一个脚印开始的时间
    QMap<int,double> right_clusterNumStartTime; //每一个脚印开始的时间
    qreal time_left=0.0;
    qreal time_right=0.0;

    //固定宽高的演示
    int ImgWidth_main;
    int ImgHeight_main;
    int ImgWidth_40;
    int ImgHeight_40;
    qreal maxValue_all=0.0;
    QDateTime datatime;
    QString strdate;
    QString date_;

    QVector<QString> m_debugData;
    int m_numData = 0;
    Dongtai m_result;
    bool isResult=false;
    Jingtai m_JingtaiResult;


    void searchSerialPort();
    void searchSerialPort2();
    void searchSerialPort3();
    void searchSerialPort4();
    void openSerialPort();
    // void readData();
    QString byteArrayToHexStr(const QByteArray &data);
    QByteArray hexStrToByteArray(const QString &data);
    char hexStrToChar(char data);

    void recordData(QString data);
    void sendData();
    void dataToSensor(QString data);
    void recordDataSlot();
    void recordReportSlot();
    void onSerialError(QSerialPort::SerialPortError error);
    void paintPdf(QPagedPaintDevice *device);


    void drawPt_160(QVector<int>data);
    float convertData(int value); // 将数据转换为绘图所需的格式--1101
    QVector<QPoint>  intialPtSize(int x, int y, int dis, int rad);
    void canshuInit();
    void initData();


    //========
    void testTemp();
    void test2();
    void test3(int n);

    void onDataReceived(int portIndex, const QVector<int>& data); // 数据接收的槽函数--1031
    void onDataReceived_2(int portIndex, const QVector<int> &data);
     void onDataReceived_Jingtai( int portIndex,const QVector<int> &data); //20250811
    void handleFilteredData(const QVector<double> &filteredData, double maxCount);
    // void handleFilteredData(const QVector<int> &filteredData);
    void flushDataBuffer();
    void setupDrawWorker();
    void onDrawCompleted(const QImage &image);
    void startSerial();
    void stopSerial();
    void handlePortActivation(int state, int portIndex);
    void openSelectedPorts();
    void closeAllPorts();
    void openSerialPort(int portIndex);
    void closeSerialPort(int portIndex);
    void onCheckboxStateChanged();
    void batchSaveDataToFile();
    void onJingTaiCheckboxStateChanged();

    void recordDataJingTai(std::vector<float> data);
    void stopRecordDataJingTai();

    void recordDataDongtai();
    void stopRecordDataDongtai();

    //查询校验串口

    QString checkPort(int indexPort);
    bool validateFrame(const QByteArray &data);


signals:
    void dataReceived(QList<Point_commondata> posList, qreal maxNum); // 新增信号--1030
    void dataReadyForDraw(const QVector<int> &data, const QVector<QPoint> &drawPts, qreal maxCount);
    void newDataPacket(const QVector<QVector<int>> & dataBatch);
    void saveDataToFile(const QStringList &dataBatch);  // 向文件线程发送数据的信号
    void sendDrawData(QList<Point_commondata> posList, qreal maxCount); // 用于绘图数据传递的信号
    void sendDrawParam(const QVector<double> &data, double maxCount);
public slots:
    void showMinwindow();
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
   // void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_main_restore_clicked();

    void on_main_min_clicked();

    void on_main_max_clicked();

    void on_main_close_clicked();






private:
    Ui::MainWindow *ui;


    // 绘图对象指针
    heatmapper *mapper_ ;
    // 用于显示输出的图像
    QImage *canvas_;

    // 绘图对象指针
    heatMap *mapper_40_ ;
    // 用于显示输出的图像
    QImage *canvas_40;

   showReport  *m_showreport;

    //  MultiThreadComDataHandler *myMultiThreadComDataHandler = nullptr;  //HC 1117注释

    // QMutex mutexSerial;
    // QThread *m_pThreadSerial = nullptr;       // 通信线程1

    // MainFunction * myMainFunction2 = nullptr;
    // QMutex mutexSerial2;
    // QThread *m_pThreadSerial2 = nullptr;       // 通信线程1

    // MainFunction * myMainFunction3 = nullptr;
    // QMutex mutexSerial3;
    // QThread *m_pThreadSerial3 = nullptr;       // 通信线程1

    QPlainTextEdit *plainTextEdit; // 声明控件指针--1030
    QVector<mySerialPort*> serialPorts; // 用于存储各个串口对象的指针
    mySerialPort* serialPort_JingTai;
    QVector<QThread*> serialThreads; // 用于存储各个串口线程的指针
    QVector<QVector<int>> receivedData; // 用于存储每个串口接收到的数据
    QVector<bool> receivedStatus; // 用于存储每个串口接收数据的状态
    QFile outputFile; // 用于保存数据的文件对象
    QStringList dataBuffer; // 用于存储批量数据
    int batchSize = 50; // 批量写入行数（可调整）
    // QThread *drawThread;
    // DrawWorker *drawWorker;
    QThread *filterThread;
    DataFilterWorker *dataFilterWorker;
    QThread *fileSaveThread;
    FileSaveWorker *fileSaveWorker;
    QTimer* dataSaveTimer; // 数据保存定时器
    QThread *heatmapThread;

    QCheckBox *checkBoxPort1;
    QCheckBox *checkBoxPort2;
    QCheckBox *checkBoxPort3;
    QCheckBox *checkBoxPort4;
    int selectedPortCount = 3; // 记录勾选的串口数量

    QQueue<QVector<int>> dataQueue;  // 数据包队列

    void drawPt_160_2(const QVector<double> &data);

    Form_blockdata* main_Form_blockdata= nullptr;
    Form_processdata*  main_Form_processdata=nullptr;
    QString lastDongDirectory = QDir::homePath(); // 保存上一次的动态数据目录路径，初始为用户目录
    QString lastJingDirectory = QDir::homePath(); // 保存上一次的静态数据目录路径，初始为用户目录

    std::string ports;

    bool isJiangTaiTest=false;


    //界面移动相关
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

    CSerialPortThread *m_pSerialPortThread;
    QString m_jingtaiPort;
    QString m_dongtaiPort1;
    QString m_dongtaiPort2;
    QString m_dongtaiPort3;

    int m_jingtaiPortIndex=2;
    int m_dongtaiPort1Index=3;
    int m_dongtaiPort2Index=4;
    int m_dongtaiPort3Index=5;



private:
    bool LeftBorderHit(const QPoint &pos);
    bool RightBorderHit(const QPoint &pos);
    bool TopBorderHit(const QPoint &pos);
    bool BottomBorderHit(const QPoint &pos);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void checkBorderDragging(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent* ev);
};

#endif // MAINWINDOW_H
