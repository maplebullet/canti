#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

#include <QMessageBox>
#include <QTimer>
#include <QDebug>

void showAutoClosingMessageBox(QWidget *parent, const QString &title, const QString &text, int timeout = 3000);

#endif // GLOBALFUNCTIONS_H
