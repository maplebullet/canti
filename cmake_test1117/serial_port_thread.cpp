
#include <string>
#include <regex>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QMutexLocker>
#include<QCoreApplication>
#include<QDebug>
#include "serial_port_thread.h"


CSerialPortThread::CSerialPortThread()
{
	
}
CSerialPortThread::~CSerialPortThread()
{
	
}
void CSerialPortThread::init()
{	
	m_pTimer = new QTimer(this);
	connect(m_pTimer, &QTimer::timeout, this, [=] {
                emit signalCheckPort(); // 定时触发所有监控器
    });
	
}

void CSerialPortThread::unInit()
{
	if(m_pTimer)
	{
		m_pTimer->stop();
		delete m_pTimer;
		m_pTimer = NULL;
	}
}
void CSerialPortThread::startCheck()
{
	m_pTimer->start(500);
}
void CSerialPortThread::stopCheck()
{
	m_pTimer->stop();
}
void CSerialPortThread::addPort(mySerialPort *pSerialPort)
{
	QMutexLocker locker(&m_objMutex);
	m_vecPorts.append(pSerialPort);
}
void CSerialPortThread::checkPort()
{
	
}
void CSerialPortThread::deletePort(mySerialPort *pSerialPort)
{
	QMutexLocker locker(&m_objMutex);
	for(size_t i = 0;i<m_vecPorts.size();i++)
	{
		if(m_vecPorts[i] == pSerialPort)
		{
            m_vecPorts.erase(m_vecPorts.begin() + i);
			break;			
		}
	}
}
void CSerialPortThread::deleteAll()
{
	QMutexLocker locker(&m_objMutex);
	m_vecPorts.clear();
}

