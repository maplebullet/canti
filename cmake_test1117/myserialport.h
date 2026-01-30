/*
 * 说明：
*/
#ifndef MYSERIALPORT_H
#define MYSERIALPORT_H
#include <QTimer>
#include <QObject>
#include <QVector>
#include <QSerialPort>
#include<QtSerialPort/QSerialPortInfo>
#include <QThread>
#include <QMutex>
#include <QMetaType>

#include "commondata.h"

class mySerialPort : public QObject
{
    Q_OBJECT

public:
    // debug 使用
    bool m_useFileDebug = false;  //改
    int m_numData = 0;
    QVector<QString> m_debugData;
    //
    // mySerialPort(const mySerialPort&)=delete;
    // static  mySerialPort &getInstance(){
    //     static  mySerialPort instance;
    //     return instance;
    // }

    // 构造函数，初始化串口名称、波特率和索引--1031
    explicit mySerialPort(const QString &portName, int baudRate, int portIndex, QObject *parent = nullptr);

    ~mySerialPort();
    void initPort(PORTSET portSet);
    void saveData(bool save,QString dataPath = ""); // 保存数据
    bool getInitFlag() { return m_isInit; }

    void sendData(QString msg);     // 发送数据  获取模块发送数据请求
    //记录数据
    void isRecordData();
    void recordData( QVector<int> data);

    // 处理串口接收到的数据--1031
    void onReceiveData();
    bool openPort();
    void closePort();

public slots:
    void slotsCheckPort();
    void onReceiveData_2();  // 处理接收到的数据
    //void sendData(int m_pt_size);
    //void sendData_40();
    void sendData();

    void receivedIsOpenSerial(bool open,int num,QString modeType);
signals:
    void receiveData(QVector<int> data);

    // 数据接收完成后，将端口索引和数据传递给主窗口--1031
    void dataReady(int portIndex, const QVector<int> &data);
    void requestClose();

private:
    mySerialPort(QObject *parent = nullptr);
    QVector<int> qbyte2Vector(QByteArray data,QString splitStr= " "); //输入数据通过字符串分割 默认空格分割
    QString byteArrayToHexStr(const QByteArray &data);
    void recordData(QString data,QString dataPath);
    QByteArray hexStrToByteArray(const QString &data);
    char hexStrToChar(char data);
    bool m_isInit = false;
    bool m_saveData = false;
    bool m_needSendData = true;
    QString m_saveDataPath;
    QMutex *m_mutex; // 互斥锁，用于线程安全地操作缓冲区
//    QThread *m_portThread;  // 串口线程
    QSerialPort *m_port;    // 串口对象指针
    QTimer *m_timer;

    bool databegin=false;
    bool dataEnd=false;

    bool m_rec_dataHead;
    bool m_rec_dataEnd;
    int pt_size=0;
    bool m_isFinsh;
    int m_cur_dataNum;

    bool starTranstion=false;

    bool m_needRecord = false;
    QString m_recordFileName;
    QString m_typeModel;

    // 1031
    int m_portIndex;             // 串口的索引，用于标识串口编号
    QByteArray m_buffer;         // 缓冲区，用于存储接收到的数据
    qint64 m_nLastCheckTime;
    QMutex m_objPortCheckMutex; // 互斥锁，用于线程安全地操作缓冲区
};

#endif // MYSERIALPORT_H
