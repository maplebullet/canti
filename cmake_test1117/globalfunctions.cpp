#include "globalfunctions.h"

void showAutoClosingMessageBox(QWidget *parent, const QString &title, const QString &text, int timeout) {
    QMessageBox *msgBox = new QMessageBox(parent);
    msgBox->setAttribute(Qt::WA_DeleteOnClose); // 确保消息框关闭时自动删除
    msgBox->setWindowTitle(title);
    msgBox->setText(text);
    msgBox->setStandardButtons(QMessageBox::NoButton); // 不显示标准按钮
    msgBox->show();

    // QTimer::singleShot(timeout, msgBox, SLOT(close())); // 设置定时器，timeout毫秒后关闭弹窗
    QTimer::singleShot(timeout, [msgBox]() {
#ifdef QT_DEBUG
        qDebug() << "automatic shutdown";
#endif
        //msgBox->close();
        msgBox->deleteLater();
    });

}

