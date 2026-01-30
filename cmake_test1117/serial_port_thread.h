/*
 * 说明：
*/
#ifndef SERIALPORTTHREAD_H
#define SERIALPORTTHREAD_H
#include <QTimer>
#include <QObject>
#include <QVector>
#include <QSerialPort>
#include<QtSerialPort/QSerialPortInfo>
#include <QThread>
#include <QMutex>
#include <QMetaType>
#include <QMutexLocker>
#include <QThread>

#include "commondata.h"
#include "myserialport.h"



class CSerialPortThread : public QThread
{
    Q_OBJECT
public:
    CSerialPortThread();
	~CSerialPortThread();
public:
	void init();
	void unInit();
	void startCheck();
	void stopCheck();
    void addPort(mySerialPort *pSerialPort);
	void checkPort();
	void deletePort(mySerialPort *pSerialPort);
	void deleteAll();
signals:
	void signalCheckPort();
private:
	QVector<mySerialPort*> m_vecPorts;
    QTimer *m_pTimer;
	QMutex m_objMutex;
};

#endif // MYSERIALPORT_H
