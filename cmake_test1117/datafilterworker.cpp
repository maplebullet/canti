#include "datafilterworker.h"
#include <QDebug>

DataFilterWorker::DataFilterWorker(QObject *parent) : QObject(parent)
{

}

// 将数值转换为压力
double DataFilterWorker::convertToPressure(int value) const {
    if (value > 0 && value <= 2170) {
        return -0.0854525 * value + 332.9351;
    } else if (value > 2170 && value <= 2380) {
        return -0.0174325 * value + 524.394462;
    } else if (value > 2380 && value <= 3780) {
        return -0.06659 * value + 265.145;
    } else if (value > 3780 && value <= 4102) {
        return -0.4414 * value + 180.954;
    } else {
        return 0; // 超出范围的数值返回0或适当的默认值
    }

    // if (value > 0 && value <= 1622) {
    //     return -0.2004 * value + 500;
    // } else if (value > 1623 && value <= 2377) {
    //     return -0.1656 * value + 443.64;
    // } else if (value > 2378 && value <= 4095) {
    //     return -0.0291 * value + 119.16;
    // }else {
    //     return 0; // 超出范围的数值返回0或适当的默认值
    // }
    // if (value > 0 && value <= 4095) {
    //     return -1 * value + 4095;
    // } else{
    //     return 0; // 超出范围的数值返回0或适当的默认值
    // }

     // if (value > 0 && value <= 3059) {
     //        return -0.10291 * value + 355.24177;
     //    } else if (value > 1623 && value <= 3346) {
     //        return -0.05914 * value + 221.35382;
     //    } else if (value > 2378 && value <= 4095) {
     //        return -0.03052 * value + 125;
     //    }else {
     //        return 0; // 超出范围的数值返回0或适当的默认值
     //    }
}

void DataFilterWorker::addDataPacket(const QVector<int> &dataPacket)
{
    // QMutexLocker locker(m_mutex); // 加锁以确保线程安全
#ifdef QT_DEBUG
    qDebug() << "11111111111111111";
#endif
/*  // 使用嵌套向量 (QVector<QVector<int>>)
    if (dataBuffer.isEmpty()) {
        dataBuffer.resize(10);
    }

    dataBuffer[packetCount] = dataPacket;
    packetCount++;

    if (packetCount == 10) {
        QVector<int> averagedData(dataPacket.size(), 0);

        // 计算平均值
        for (const auto &packet : dataBuffer) {
            for (int i = 0; i < packet.size(); i++) {
                averagedData[i] += packet[i];
            }
        }
        for (int &value : averagedData) {
            value /= 10; // 取平均
        }

        emit filteredDataReady(averagedData); // 发射信号，通知数据已计算完成
        packetCount = 0; // 重置计数器
    }
*/


    /*
     * 使用累加向量 (QVector<int>)
     *
     * 累加每个位置的数据，每次收到一包数据就加到对应的累加向量上。
     * 这样可以避免嵌套向量的内存开销，减少遍历次数。
     * 最后除以包数即可获得平均值。
    */
    if (cumulativeData.isEmpty()) {
        cumulativeData.resize(dataPacket.size());
        cumulativeData.fill(0);
    }

    // 确保 pressureData 大小与 dataPacket 一致
    if (pressureData.size() != dataPacket.size()) {
        pressureData.resize(dataPacket.size());
        pressureData.fill(0);
    }

    // 累加每个数据位置的值
    for (int i = 0; i < dataPacket.size(); i++) {
        cumulativeData[i] += dataPacket[i];
    }
    packetCount++;

    // 当收到10包数据后，计算平均值
    if (packetCount == 10) {
        QVector<int> averagedData(dataPacket.size());
        for (int i = 0; i < cumulativeData.size(); ++i) {
            averagedData[i] = static_cast<int>(cumulativeData[i] / 10);

            // int averagedValue = static_cast<int>(cumulativeData[i] / 10);
            // pressureData[i] = convertToPressure(averagedValue);
            // 将数值转换为压力
            // if (averagedValue > 0 && averagedValue <= 2170) {
            //     pressureData[i] = -0.0854525 * averagedValue + 332.9351;
            // } else if (averagedValue > 2170 && averagedValue <= 2380) {
            //     pressureData[i] = -0.0174325 * averagedValue + 524.394462;
            // } else if (averagedValue > 2380 && averagedValue <= 3780) {
            //     pressureData[i] = -0.06659 * averagedValue + 265.145;
            // } else if (averagedValue > 3780 && averagedValue <= 4102) {
            //     pressureData[i] = -0.4414 * averagedValue + 180.954;
            // } else {
            //     pressureData[i] = 0; // 默认值或错误处理
            // }

            // pressureData[i] = std::max(_maxCount, pressureData[i]);

        }

        // emit filteredDataReady(pressureData); // 发射信号，传递计算好的平均数据，转换为压力
        // emit filteredDataReady(averagedData); // 发射信号，传递计算好的平均数据

        // 重置累加向量和计数器
        cumulativeData.fill(0);
        packetCount = 0;
    }
    // 函数结束时，QMutexLocker对象销毁并解锁
}

void DataFilterWorker::addDataPacket_2(const QVector < QVector<int>> &dataBatch)
{
    qDebug()<<dataBatch.size()<<"addDataPacket_2";

    for (const auto& dataPacket : dataBatch) {
        QVector<double> pressureData;
        pressureData.resize(dataPacket.size());

        double tempMax = 0.0;

        // 遍历数据并批量处理转换
        for (int i = 0; i < dataPacket.size(); ++i) {
            double value = static_cast<double>(dataPacket[i]);
            // 将数值转换为压力

            // if (value > 0 && value <= 3059) {
            //     pressureData[i] =-0.10291 * value + 355.24177;
            // } else if (value > 1623 && value <= 3346) {
            //     pressureData[i] = -0.05914 * value + 221.35382;
            // } else if (value > 2378 && value <= 4095) {
            //     pressureData[i] =-0.03052 * value + 125;
            // }else {
            //     pressureData[i] = 0; // 超出范围的数值返回0或适当的默认值
            // }

            // if (value > 0 && value <= 1622) {
            //    pressureData[i] =-0.2004 * value + 500;
            // } else if (value > 1623 && value <= 2377) {
            //     pressureData[i] =-0.1656 * value + 443.64;
            // } else if (value > 2378 && value <= 4102) {
            //     pressureData[i] = -0.0291 * value + 119.16;
            // }else {
            //     pressureData[i] = 0; // 超出范围的数值返回0或适当的默认值
            // }
            // if (value > 0 && value <= 4095) {
            //     pressureData[i] = -1 * value + 4095;
            // } else{
            //     pressureData[i] = 0; // 超出范围的数值返回0或适当的默认值
            // }

            if (value > 0 && value <= 2170) {
                pressureData[i] =-0.0854525 * value + 332.9351;
            } else if (value > 2170 && value <= 2380) {
                pressureData[i] = -0.0174325 * value + 524.394462;
            } else if (value > 2380 && value <= 3780) {
                pressureData[i] =-0.06659 * value + 265.145;
            }else if(value > 3780 && value <= 4102) {
                pressureData[i] =-0.4414 * value + 180.954;
            }else{
                pressureData[i] = 0; // 超出范围的数值返回0或适当的默认值
            }
            // 如果值小于 0，将其设置为 0
            if (pressureData[i] < 0) {
                pressureData[i] = 0.0;
            }
            // 更新临时最大值
            if (pressureData[i] > tempMax) {
                tempMax = pressureData[i];
            }
        }
    //qDebug()<<pressureData.size()<<"pressureData.size";
        emit filteredDataReady(pressureData, tempMax); // 发射信号，传递转换为压力
    }
}
