#include "showreport.h"
#include "ui_showreport.h"
#include"globaldata.h"
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

showReport::showReport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::showReport)
{
    ui->setupUi(this);
    setWindowTitle("查看报告");
    setWindowFlag(Qt::FramelessWindowHint);  //取消默认功能
    setAutoFillBackground(true);

}

showReport::~showReport()
{
    delete ui;
}

void showReport::reseiveShowReport(QString currentFilename,int index)
{

    // 检查文件路径有效性
    if (currentFilename.isEmpty()) {
        qWarning() << "Invalid PDF file path:" << currentFilename;
        return;  // 路径无效时直接返回
    }

    // 更新全局变量（根据实际需求评估是否必要）
    globaldata::currentPrintPDF = currentFilename;

    // 加载并显示PDF
    QPdfDocument* pdfDocument = new QPdfDocument(this);  // 父对象为当前窗口，自动管理内存
    //ui->pdfView->setDocument(nullptr);  // 清空视图
    pdfDocument->load(currentFilename);
    ui->pdfView->setDocument(pdfDocument);  // 显示PDF

    // 切换按钮激活状态（优化样式设置逻辑）
    const QString activeStyle = "QPushButton { background-color: #76CE6E; }";
    const QString inactiveStyle = "";

    switch (index) {
    case 0:
        ui->pushButton_zudi->setStyleSheet(activeStyle);
        ui->pushButton_butai->setStyleSheet(inactiveStyle);
        break;
    case 1:
        ui->pushButton_butai->setStyleSheet(activeStyle);
        ui->pushButton_zudi->setStyleSheet(inactiveStyle);
        break;
    default:  // 处理非法index值
        qWarning() << "Invalid index:" << index;
        break;
    }

}

void showReport::showReportFromHistory(QString filename)
{
    if(filename!=nullptr){
        QPdfDocument* pdfDocument=new QPdfDocument(this);
        pdfDocument->load(filename);
        ui->pdfView->setDocument(pdfDocument);


    }
}

void showReport::SlotPreviewPdfOnWidget(QPrinter *printer)
{
    qDebug() << "SlotPreviewPdfOnWidget start";


    //自定义纸张大小,且需要在创建QPainter之前设置大小，否则会不起作用
    //  printer->setPaperSize(QPrinter::A4);
    // printer->setPaperSize(QSizeF(2479, 3508),QPrinter::Point);//未其作用，一直是w,h(595,842)
    //    printer->setPaperSize(QSizeF(ui->centralwidget->height(), ui->centralwidget->width()),QPrinter::Point);
    printer->setPageSize(QPageSize::A4);
    printer->setResolution(300);
    printer->setPageMargins(QMarginsF(0,0,0,0));//必须设置为0

    QPainter painterPixmap(this);
    //   QPainter* painterPixmap=new QPainter(this);
    // printerPixmap.setPrinterName(printerName);

    painterPixmap.begin(printer);
}

void showReport::on_BtnZoomIn_clicked()
{
    ui->pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    ui->pdfView->setZoomFactor(ui->pdfView->zoomFactor()*0.8);
}


void showReport::on_BtnZoomOut_clicked()
{
    ui->pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    ui->pdfView->setZoomFactor(ui->pdfView->zoomFactor()*1.2);
}


void showReport::on_printReport_clicked()
{
    // 初始化QtPrintSupport模块
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog printDialog(&printer,this);
    // printer.setOutputFormat(QPrinter::PdfFormat);
    // printer.setOutputFileName("output.pdf");
    // printer.setPageSize(QPageSize::A4);

    // 打开PDF文件
    // QFile file(globaldata::reportAdress);
    // if (!file.open(QIODevice::ReadOnly)) {
    //     qDebug() << "Failed to open PDF file.";
    //     return ;
    // }




    QPdfDocument pdfDocument;
    pdfDocument.load(globaldata::currentPrintPDF);
    // 创建一个QPainter对象，将其绑定到打印机上

    // QRect m_rect=painter.viewport();
    // QImage image(globaldata::currentPrintPDF);


    if (printDialog.exec() == QDialog::Accepted) {
        QPainter painter;
        painter.begin(&printer)    ;
        const QRect pageRect = printer.pageRect(QPrinter::Unit::DevicePixel).toRect();
        const QSize pageSize = pageRect.size();
        for (int i = 0; i < pdfDocument.pageCount(); ++i) {
            if (i > 0)
                printer.newPage();
            const QImage &page = pdfDocument.render(i, pageSize);
            painter.drawImage(pageRect, page);
        }
        painter.end();
    }

}


void showReport::on_pushButton_clicked()
{
    emit  hideAndShowMain_report();
    this->close();
}


void showReport::on_pushButton_zudi_clicked()
{
    reseiveShowReport(globaldata::reportPath_jingtai,0);
}


void showReport::on_pushButton_butai_clicked()
{
    reseiveShowReport(globaldata::reportPath_dongtai,1);
}

