#include "myserialport.h"
#include <string>
#include <regex>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QMutexLocker>
#include<QCoreApplication>
#include<QDebug>
#include "log.h"
#define DNO_DATA_CONFIRM_TIME   2000
// mySerialPort::mySerialPort(QObject *parent)
//     : QObject{parent}
// {
// //    m_portThread = new QThread();
//     //    this->moveToThread(m_portThread);
//     m_timer = new QTimer(this);
//     m_mutex = new QMutex();
//     m_needSendData = true;
//     connect(m_timer,SIGNAL(timeout()),this,SLOT(sendData()));
//  //  connect(m_port, SIGNAL(readyRead()), this, SLOT(onReceiveData()), Qt::QueuedConnection); //Qt::DirectConnection
// }

// 构造函数--1031
mySerialPort::mySerialPort(const QString &portName, int baudRate, int portIndex, QObject *parent)
    : QObject(parent),
    m_port(new QSerialPort(this)),
    m_portIndex(portIndex),
    m_mutex(new QMutex()),
    m_buffer()
{
    m_port->setPortName(portName);
    m_port->setBaudRate(baudRate);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setFlowControl(QSerialPort::NoFlowControl);

    // 添加信号槽连接，用于关闭串口
    connect(this, &mySerialPort::requestClose, this, [this]() {
        if (m_port->isOpen()) {
            m_port->close();
        }
        delete m_port;
        m_port = nullptr; // 避免悬挂指针
        }, Qt::QueuedConnection);
}
// ...

QVector<int> mySerialPort::qbyte2Vector(QByteArray data ,QString splitStr)
{
    QVector<int> result;
    std::regex reg(splitStr.toStdString());		// 匹配split
    std::string str = data.toStdString();
    str.replace(str.find("\r\n"),1,"");
    std::sregex_token_iterator pos(str.begin(), str.end(), reg, -1);
    decltype(pos) end;              // 自动推导类型
    for (; pos != end; ++pos)
    {
        //TODO:是否需要加上越界容错判断？
        int num = std::stoul(pos->str(), 0,16);
        result.push_back(num);
    }
    return result;
}

QString mySerialPort::byteArrayToHexStr(const QByteArray &data)
{
    QString temp = "";
    QString hex = data.toHex();
    for (int i = 0; i < hex.length(); i = i + 2) {
        temp += hex.mid(i, 2) + " ";
    }

    return temp.trimmed().toUpper();
}

void mySerialPort::recordData(QString data,QString dataPath)
{
    QDateTime da_time;
    QString time_str = da_time.currentDateTime().toString("[yyyy-MM-dd HH:mm:ss]");

//    QDir *DataFile = new QDir(runPath);
//    bool exist = DataFile->exists("DataFile");
//    if(!exist)
//    {
//        bool isok = DataFile->mkdir("DataFile"); // 新建文件夹
//            if(!isok)
//                QMessageBox::warning(this,"Waring","can't mkdir",QMessageBox::Yes);
//    }
//    QString fileName = runPath+"/DataFile/"+"Data.txt";

    QFile file(dataPath);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
//      QMessageBox::warning(this,"Waring","can't open",QMessageBox::Yes);
    }
    QTextStream stream(&file);
    stream<<time_str+":"+data<<"\n";
    file.close();
}

QByteArray mySerialPort::hexStrToByteArray(const QString &data)
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

char mySerialPort::hexStrToChar(char data)
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
mySerialPort::~mySerialPort()
{
    m_port->close();
    m_port->deleteLater();
//    m_portThread->quit();
//    m_portThread->wait();
//    m_portThread->deleteLater();
    closePort();
}

void mySerialPort::initPort(PORTSET portSet)
{
  //  qDebug()<<"进入串口初始化";
    if(!m_isInit){
        if(m_useFileDebug){
            QFile file("D:\\WS\\gitee\\shuju\\1.txt");
            if(!file.open(QIODevice::ReadOnly)){
                return;
            }
            QTextStream stream(&file);
            while(!stream.atEnd()){
                QString line = stream.readLine();
                line = line.mid(22,-1);
                m_debugData.push_back(line);
            }
            file.close();
        }
        qRegisterMetaType<QVector<int>>("QVector<int>");
      //  m_timer->start(100);  //HC 改
        m_port = new QSerialPort();

        m_port->setPortName(portSet.portText);
       // qDebug()<<m_port->portName()<<"<m_port->portName()";
                //串口名 windows下写作COM1
        m_port->setBaudRate(portSet.getBaudRate());                           //波特率
        m_port->setDataBits(portSet.getDataBits());             //数据位
        m_port->setStopBits(portSet.getStopBits());           //停止位
        m_port->setParity(portSet.getParity());            //奇偶校验
        m_port->setFlowControl(QSerialPort::NoFlowControl);  //流控制
        if (!m_port->open(QIODevice::ReadWrite))
        {
           //
            //qDebug() << "open it failed";
            return;
        }
        //    m_port->moveToThread(m_portThread);
        //    m_portThread->start();
      // qDebug()<<" 串口初始化 ";

        m_isInit = true;
    }else{
    //  qDebug()<<" 串口已经初始化 ";
    }
}

//void mySerialPort::sendData(int m_pt_size)
//{
////     sendData("5A 04 6D 02 11 08"); // 获取数据字符串
//////    if(m_pt_size==400){
////           sendData("5A 04 6D 02 11 08"); // 获取数据字符串
//////    }
//////    else if(m_pt_size==1600){
//////        sendData("5A"); //获取数据字符串
//////    }
////    pt_size=m_pt_size;

//}

void mySerialPort::sendData()
{
    if(pt_size==400){

        sendData("5A"); // 获取数据字符串

    }
    else if (pt_size==1600) {

        sendData("5A"); // 获取数据字符串

    }

}

void mySerialPort::slotsCheckPort()
{
    QMutexLocker locker(&m_objPortCheckMutex);
    if((QDateTime::currentMSecsSinceEpoch() - m_nLastCheckTime) >= DNO_DATA_CONFIRM_TIME)
    {
        m_nLastCheckTime = QDateTime::currentMSecsSinceEpoch();
        m_port->close();
        QThread::msleep(100);
        if(!m_port->open(QIODevice::ReadWrite))
        {
            qDebug() << "reopen serial failed"<<m_port->portName();
            LOG_DEBUG( "reopen serial failed" + m_port->portName());
        }
        qDebug() << "slotsCheckPort  not receive data"<<m_port->portName();
        LOG_DEBUG( "slotsCheckPort  not receive data" + m_port->portName());
    }
}
// 1031
// 在串口有数据可读时被触发，用于从串口读取数据并解析
void mySerialPort::onReceiveData_2()
{
    qDebug()<<m_portIndex<<"串口索引";
    // QMutexLocker locker(m_mutex);
    QMutexLocker locker(&m_objPortCheckMutex);
    m_nLastCheckTime = QDateTime::currentMSecsSinceEpoch();

    m_mutex->lock();
    m_buffer.append(m_port->readAll());
    m_mutex->unlock();
    qDebug()<<m_buffer.size()<<"buff.size";
    // 检查缓冲区是否包含完整的3208字节数据帧
    while (m_buffer.size() >= 3208) {
        m_mutex->lock();
        // 检查帧头，确保帧头格式正确 // 且第6个字节匹配当前串口索引
        if (m_buffer.startsWith("\x5A\x01\x95\x6C")) { //  && m_buffer[5] == static_cast<char>(m_portIndex)

            QVector<int> dataVec;
            // 去掉前6个字节的帧头和最后2个字节的帧尾，取出3200字节的有效数据
            QByteArray data = m_buffer.mid(6, 3200);
            m_buffer.remove(0, 3208);  // 移除已处理的数据帧
            m_mutex->unlock();

            // 将每两个字节转换为无符号整数，按照低位在前高位在后的顺序
            for (int i = 0; i < data.size(); i += 2) {
                int value = (static_cast<unsigned char>(data[i + 1]) << 8) | static_cast<unsigned char>(data[i]);
                dataVec.push_back(value);
            }

            // 发送信号，将串口索引和数据传递给 MainWindow
            emit dataReady(m_portIndex, dataVec);
        } else {
            // 帧头不匹配，丢弃第一个字节，继续寻找有效帧
            m_buffer.remove(0, 1);
            m_mutex->unlock();
        }
    }

}

bool mySerialPort::openPort()
{
    if(m_port == NULL)
    {
        return false;
    }
    if (m_port->open(QIODevice::ReadWrite)) {
        connect(m_port, &QSerialPort::readyRead, this, &mySerialPort::onReceiveData_2);
    } else {
        qDebug() << "Failed to open port" << m_port->portName();
        LOG_DEBUG( "Failed to open port" + m_port->portName());
        return false;
    }
    qDebug() << "open port succeed" << m_port->portName();
    LOG_DEBUG( "open port succeed" + m_port->portName());
    m_nLastCheckTime = QDateTime::currentMSecsSinceEpoch() ;
    return true;
}
void mySerialPort::closePort()
{
    if (m_port) {
        if (m_port->isOpen()) {
            m_port->close();
        }
        delete m_port;
        m_port = nullptr; // 避免悬挂指针
    }
}

// ...


void mySerialPort::onReceiveData()
{

    QByteArray data = m_port->readAll();
    qDebug()<<"串口类接收数据: "<<data;
    QVector<int> dataVec;
    int dataLen = data.length();
 qDebug()<<dataLen<<"datalength";
    if (dataLen <= pt_size) {
         m_needSendData = true;
        return;
    }
    m_rec_dataEnd = true;
    QString buffer = byteArrayToHexStr(data);
   qDebug()<<buffer<<buffer.length()<<"buffer";

    buffer = buffer.simplified();
    QStringList lists = buffer.split(" ");
  // qDebug()<<lists<<"lists";
    if(lists[0]=="5A"&&lists[1]=="01"&&
            lists[2]=="95"&&lists[3]== "6C"
            &&lists[4]=="00"&&lists[5]== "02")
    {
        //qDebug()<<"将进入窗口欧数据处理";
       // databegin=true;
        for(int i =0;i<6;i++)
        {
            lists.removeAt(0);
        }
        dataVec.resize(pt_size);
        m_rec_dataHead = true; //HC:收到表头
        m_rec_dataEnd = false;
        bool f;
        m_isFinsh=false;
        for(int i =0;i<lists.size();i++)
        {

            if(i>=pt_size)
            {
                //pt_size = i;
                m_cur_dataNum = i;
                m_isFinsh = true;
                break;
            }
             m_cur_dataNum = i;
          // dataVec.push_back(lists[i].toInt(&f,16));
            dataVec[m_cur_dataNum]=lists[i].toInt(&f,16);

        }

    }else{
        QByteArray data_clean = m_port->readAll();
        data_clean.clear();
    }
   //   qDebug()<<dataVec<<dataVec.length()<<"dataVec";

    if(m_isFinsh){
           qDebug()<<dataVec<<dataVec.length()<<"dataVec";
         m_needSendData = true;
         if(m_needRecord){
         recordData(dataVec);

         }
        emit receiveData(dataVec);

         m_isFinsh=false;
    }
}

void mySerialPort::sendData(QString msg)
{
    //  qDebug()<<"msg"<<msg;
    QMutexLocker locker(m_mutex);
    if(m_needSendData){
        QByteArray msgByte = hexStrToByteArray(msg);
    //   qDebug()<<msgByte<<"msgByte";
        m_port->write(msgByte);   //发送“data”字符
        m_needSendData = false;
    }
    else {
        onReceiveData();

    }
}

void mySerialPort::isRecordData()
{

    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy_MM_dd")+"_"+m_typeModel;
   if(m_typeModel=="grip"){

       m_recordFileName=QCoreApplication::applicationDirPath()+"/originData_grit_"+current_date+".txt";
     //  qDebug()<<m_recordFileName<<"m_recordFileName";
   }else if (m_typeModel=="gaitModel") {
        m_recordFileName=QCoreApplication::applicationDirPath()+"/originData_gaitModel_"+current_date+".txt";
}else if (m_typeModel=="stand") {
       m_recordFileName=QCoreApplication::applicationDirPath()+"/originData_stand_"+current_date+".txt";
     }else if (m_typeModel=="sitUp") {
         m_recordFileName=QCoreApplication::applicationDirPath()+"/originData_sitUp_"+current_date+".txt";
     }
    m_needRecord = true;
}

void mySerialPort::recordData( QVector<int> data)
{
    QDateTime da_time;
    QString time_str = da_time.currentDateTime().toString("[yyyy-MM-dd HH:mm:ss]");
    QString runPath = QCoreApplication::applicationDirPath();
    QFile file(m_recordFileName);
    if(file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append)){
                   QTextStream stream(&file);
                    stream<<time_str+":";
                    for(int i=0;i<data.count();i++){
                    stream<<data.at(i)<<" ";
                    }
                    stream<<"\n";
                    file.close();
    }
}


void mySerialPort::receivedIsOpenSerial(bool isOpen, int num,QString typeModel)
{
   starTranstion=isOpen;
   pt_size=num;
    m_typeModel=typeModel;
   if(starTranstion){

       if(!m_needRecord){
           isRecordData();
       }
       if(num==1600){
           sendData();
           m_timer->start(500);
       }else{
//           m_timer->start(100); //100
}
   }
   else {
        m_timer->stop();
        QByteArray data_clean = m_port->readAll();
        data_clean.clear();
        m_needRecord=false;
   }
}

void mySerialPort::saveData(bool save,QString dataPath)
{
    m_saveData = save;
    m_saveDataPath = dataPath;
}
