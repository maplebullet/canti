#ifndef COMMONDATA_H
#define COMMONDATA_H
#include <QString>
#include <QPointF>
#include <QVector>
#include<QtSerialPort/QSerialPortInfo>
#include<QSerialPort>
#include <QMap>
#include<QPushButton>
#include<QLineEdit>
//#include <QSqlDatabase>
//#include <QSqlQuery>
//#include<QSqlRecord>
#include"globaldata.h"
//#include"qcustomplot.h"
#include <QDebug>
struct APPConfigSt{
    QString sIP = "127.0.0.1";
    int nPort = 1883;
    QString sUser = "jy";
    QString sPassword = "123";
    QString sDevSub = "FROM_DEV,DATA";
    QString sDevPub = "TO_DEV";
};
struct PORTSET{
           QString  portText;       //串口名称
           QString  baudRate;       //波特率
           QString  dataBits;       //数据位
           QString  Parity;        //校验位
           QString  StopBits;       //停止位
           // TODO 流控制？
           QSerialPort::BaudRate getBaudRate(){
               QSerialPort::BaudRate qBaudRate = (QSerialPort::BaudRate)(this->baudRate.toInt());
               return qBaudRate;
           }
           QSerialPort::DataBits getDataBits(){
               QSerialPort::DataBits qDataBits = (QSerialPort::DataBits)(this->dataBits.toInt());
               return qDataBits;
           }
           QSerialPort::StopBits getStopBits(){
               QSerialPort::StopBits qStopBits = (QSerialPort::StopBits)(this->StopBits.toInt());
               return qStopBits;
           }
           QSerialPort::Parity getParity(){
               QSerialPort::Parity qParity = (QSerialPort::Parity)(this->Parity.toInt());
               return qParity;
           }

           QSerialPort::Parity  parity;        //校验位
           QSerialPort::StopBits  stopBits;       //停止位
           QSerialPort::FlowControl  flowControl; //流控制方式
       };


// 双足步态使用结构体 保存在步态文件中？
struct GRIDDATA{
    int col;
    int row;
    float viewCol;
    float viewRow;
    float distanceRow;
    float distanceCol;
    QPointF leftTopPt;
    void setRowCol(int _col,int _row,float _viewCol,float _viewRow){
        col = _col;
        row = _row;
        viewCol = _viewCol;
        viewRow = _viewRow;
        distanceCol = _viewCol/(_col+1);
        distanceRow = _viewRow/(_row+1);
        qDebug()<<distanceCol<<distanceRow<<"长宽";
//        distance = distanceC > distanceR ? distanceR : distanceC;
        leftTopPt.setX(distanceCol);
        leftTopPt.setY(distanceRow);
    }
    QPointF getPoint(int _col,int _row){
        QPointF currentPt(_col*distanceCol,_row*distanceRow);
       return leftTopPt+currentPt;
        //return currentPt;
    }
};
enum FOOT{ LEFT,RIGHT };
struct FOOTDATA{
    FOOT footName;  // 左右脚名称
    int sensorLoc;  // 极值在传感器发送数据的位置
    int viewRow;    // 极值在graphicsView的行
    int viewCol;    // 极值在graphicsView的列
    float sceneRow; // 在传感器中的行
    float sceneCol;
    QPointF viewPos;
    QVector<QPointF> clusterPos;
    double startTime;

};

struct  LineGradient{
    double lv1;
    double lv2;
    double lv3;
    double lv4;
    double lv5;
};

struct Point_commondata
{
    int posX;
    int posY;
    int radius;
    float count;
};


struct TITLE{
    QLineEdit* name;
    QLineEdit* gender;//性别
    QLineEdit* age;
    QLineEdit* num;

};

struct M_WIDGET{
    //QCustomPlot* maxAxis;
    //QCustomPlot* locationAxis;
    QImage* heatMap;
    QString suggestion;
};
//

#endif // COMMONDATA_H
