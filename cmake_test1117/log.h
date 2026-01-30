#ifndef LOG_H
#define LOG_H
#include <QDateTime>
#include <QObject>
#include <QFile>
#include <QMutex>
#include <QTextStream>

#include <QCoreApplication>

enum LogLevel {
    DEBUGLevel = 0,
    INFOLevel = 1,
    WARNINGLevel = 2,
    ERRORLevel = 3,
    CRITICALLevel = 4
};

class log:public QObject
{
    Q_OBJECT
public:
        //   log();

    log();
    ~log();
    log(const log&) = delete;
    log& operator=(const log&) = delete;

    static log& instance() {
        static log logger;
        return logger;
    }

    void setLogFile(const QString &filename);
    void writeLog(LogLevel level, const QString &message);
    void clearLog();


// 日志宏定义
#define LOG_DEBUG(message)    log::instance().writeLog(DEBUGLevel, message)
#define LOG_INFO(message)     log::instance().writeLog(INFOLevel, message)
#define LOG_WARNING(message)  log::instance().writeLog(WARNINGLevel, message)
#define LOG_ERROR(message)    log::instance().writeLog(ERRORLevel, message)
#define LOG_CRITICAL(message) log::instance().writeLog(CRITICALLevel, message)


private:


    QString levelToString(LogLevel level) const;
    QColor levelToColor(LogLevel level) const;

    mutable QMutex mutex;

    QString time_str =QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss");
    QString logFile = QCoreApplication::applicationDirPath() +"/logDir"+"/"+time_str+ ".txt";

    bool consoleOutput = true;
    LogLevel minLogLevel = DEBUGLevel;
};

#endif // LOG_H
