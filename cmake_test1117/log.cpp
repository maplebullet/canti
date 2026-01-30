#include "log.h"
#include <QTextStream>
#include <QApplication>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDir>
void log::setLogFile(const QString &filename)
{
    QMutexLocker locker(&mutex);
    logFile = filename;
    //clearLog();
}

void log::writeLog(LogLevel level, const QString &message)
{
    if (level < minLogLevel) return;  // 过滤低于设定级别的日志

    QMutexLocker locker(&mutex);

    QString levelStr = levelToString(level);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logEntry = QString("[%1] [%2] %3").arg(timestamp, levelStr, message);


    // 输出到文件
    QFile file(logFile);
    if (file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append)) {
        QTextStream stream(&file);
        stream << logEntry << Qt::endl;
        file.close();
    }

    // 输出到控制台
    if (consoleOutput) {
        QTextStream out(stdout);
        out << logEntry << Qt::endl;
    }

}

void log::clearLog()
{
    QMutexLocker locker(&mutex);

    QFile file(logFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "============== Log Started ==============\n";
        stream << "Application: " << QCoreApplication::applicationName() << "\n";
        stream << "Version: " << QCoreApplication::applicationVersion() << "\n";
        stream << "Timestamp: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
        stream << "=========================================\n\n";
        file.close();
    }
}

log::log() {
    qDebug()<<logFile<<"logFile";
    // 确保日志目录存在
    QDir logDir = QFileInfo(logFile).dir();
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }

    clearLog(); // 启动时清空旧日志
}

QString log::levelToString(LogLevel level) const
{
    switch(level) {
    case DEBUGLevel:    return "DEBUG";
    case INFOLevel:     return "INFO";
    case WARNINGLevel:  return "WARN";
    case ERRORLevel:    return "ERROR";
    case CRITICALLevel: return "CRITICAL";
    default:       return "UNKNOWN";
    }
}

QColor log::levelToColor(LogLevel level) const
{
    switch(level) {
    case DEBUGLevel:    return Qt::darkGray;
    case INFOLevel:     return Qt::darkBlue;
    case WARNINGLevel:  return Qt::darkYellow;
    case ERRORLevel:    return Qt::red;
    case CRITICALLevel: return Qt::darkRed;
    default:       return Qt::black;
    }
}

log::~log()
{

}
