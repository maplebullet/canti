#ifndef FILESAVEWORKER_H
#define FILESAVEWORKER_H

#include <QObject>
#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QMutex>
#include <QMutexLocker>

class FileSaveWorker : public QObject {
    Q_OBJECT

public:
    explicit FileSaveWorker(const QString &fileName, QObject *parent = nullptr)
        : QObject(parent), file(fileName) {}

    bool openFile() {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Failed to open file for writing:" << file.fileName();
            return false;
        }
        return true;
    }

    void closeFile() {
        if (file.isOpen()) {
            /*QTextStream out(&file);
            for (const QString &line : dataBuffer) {
                out << line <<"\n";
            }
            dataBuffer.clear();*/
            file.close();
        }
    }

public slots:

    void saveData(const QStringList &dataBatch) {

        #ifdef QT_DEBUG
        qDebug() << "save data:" << dataBatch;
        #endif

        QMutexLocker locker(&mutex);
        if (!file.isOpen()) return;

        // 移除 dataBuffer，已在 MainWindow 中缓存了 15 秒的数据
        //dataBuffer.append(dataBatch);

        QTextStream out(&file);
        out.setRealNumberNotation(QTextStream::FixedNotation); // 使用固定小数点表示法
        out.setRealNumberPrecision(2); // 保留两位小数

        // 直接将 dataBatch 写入文件
        for (const QString &line : dataBatch) {
            out << line <<"\n";
        }
        //dataBuffer.clear();
    }

private:
    QFile file;
    QMutex mutex;
    //QStringList dataBuffer;
};

#endif // FILESAVEWORKER_H
