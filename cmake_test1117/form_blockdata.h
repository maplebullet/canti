#ifndef FORM_BLOCKDATA_H
#define FORM_BLOCKDATA_H
#include "process.h"
#include <QWidget>
#include <QLabel>
#include<QMouseEvent>
#include<QDateTime>
#include"globaldata.h"
#include<QFile>
#include<QPdfWriter>
#include<QPainter>
#include "log.h"
namespace Ui {
class Form_blockdata;
}

class Form_blockdata : public QWidget
{
    Q_OBJECT

public:
    explicit Form_blockdata(QWidget *parent = nullptr);
    ~Form_blockdata();


    //void readBlockData(cv::String str);  Dongtai
    void readBlockData(Dongtai str);
    
    QImage Mat2QImage(cv::Mat& image);
    void setLabelImage(QLabel* label, const cv::Mat& image);
    void printDongtaiResult(const Dongtai& result);
    void recordReportDongtai();//生成类
    void paintPdfDongtai(QPainter *pPainter); //绘制报告
    void initUI();

signals:
    void hideAndShowMain_block();
private:
    bool LeftBorderHit(const QPoint &pos);
    bool RightBorderHit(const QPoint &pos);
    bool TopBorderHit(const QPoint &pos);
    bool BottomBorderHit(const QPoint &pos);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void checkBorderDragging(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent* ev);
private slots:
    void on_main_return_clicked();
    void on_main_restore_clicked();

    void on_main_min_clicked();

    void on_main_max_clicked();

    void on_main_close_clicked();

private:
    Ui::Form_blockdata *ui;
    QPoint mousePoint;
    bool mouse_press;
    QPoint m_mousePos;
    QPoint m_wndPos;
    bool m_bMousePressedTitle = false;
    bool m_bMousePressed = false;
    bool m_bDragTop = false;
    bool m_bDragLeft = false;
    bool m_bDragRight = false;
    bool m_bDragBottom = false;
    QRect m_StartGeometry;
    const int CONST_DRAG_BORDER_SIZE = 5;
    Dongtai result;
};

#endif // FORM_BLOCKDATA_H
