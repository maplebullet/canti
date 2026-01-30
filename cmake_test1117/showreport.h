#ifndef SHOWREPORT_H
#define SHOWREPORT_H

#include <QWidget>
#include <QPdfWriter>
#include <QFile>
#include <QTextOption>
#include <QDateTime>

#include <QPixmap>
#include <QFont>
#include <QPainter>
#include <QPen>
#include <QPrintPreviewWidget>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include<QPdfWriter>
#include<QtPdf>
#include<QPdfDocument>
#include<QMessageBox>
#include<QtMath>
#include<qlcdnumber.h>
#include<qdebug.h>

#include<QPrintPreviewWidget>
#include<QPrintDialog>

class globaldata;
namespace Ui {
class showReport;
}

class showReport : public QWidget
{
    Q_OBJECT

public:
    explicit showReport(QWidget *parent = nullptr);
    ~showReport();

    void m_printDocument();
signals:
    void hideAndShowMain_report();
public slots:
    void reseiveShowReport(QString currentFilename,int index);
    void showReportFromHistory(QString filename);

protected:
    virtual void printDocument(QPrinter *) const {};

private slots:
    void SlotPreviewPdfOnWidget(QPrinter *printer);
    void on_BtnZoomIn_clicked();

    void on_BtnZoomOut_clicked();

    void on_printReport_clicked();

    void on_pushButton_clicked();

    void on_pushButton_zudi_clicked();

    void on_pushButton_butai_clicked();

private:
    Ui::showReport *ui;
    QPrintPreviewDialog *printPreview = nullptr;
    QPrintPreviewWidget *printPreviewWidget = nullptr;//打印预览控件
    QPrinter *pdfPrinter = nullptr;//pdf绘图printer
};

#endif // SHOWREPORT_H
