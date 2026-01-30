#ifndef DATAFILTERWORKER_H
#define DATAFILTERWORKER_H

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>

class DataFilterWorker : public QObject
{
    Q_OBJECT

public:
    explicit DataFilterWorker(QObject *parent = nullptr);

signals:
    void filteredDataReady(const QVector<double> &filteredData, double maxCount);
    // void filteredDataReady(const QVector<int> &filteredData);

public slots:
    void addDataPacket(const QVector<int> &dataPacket);
    //void addDataPacket_2(const QVector<int> &dataPacket);
    void addDataPacket_2(const QVector<QVector<int>> &dataBatch);

private:
    double convertToPressure(int value) const; // 压力转换函数声明

    QMutex *m_mutex; // 互斥锁，用于线程安全地操作缓冲区
    QVector<QVector<int>> dataBuffer; // 嵌套向量，缓存10包数据
    QVector<int> cumulativeData; // 累加向量，用于累积每个位置的数据
    QVector<double> pressureData; // 声明为 double 类型来存储压力值
    // int averagedValue = 0;
    int packetCount = 0; // 计数器，记录已接收的数据包数量
    qreal _maxCount = 0.00;
};

#endif // DATAFILTERWORKER_H
