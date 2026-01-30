#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H
#include <QGraphicsView>

class myGraphicsView:public QGraphicsView
{
    Q_OBJECT
public:
    explicit myGraphicsView(QWidget *parent = 0);
    ~myGraphicsView();

protected:
    void mousePressEvent(QMouseEvent *event);
    bool eventFilter(QObject *object, QEvent *event);

private:

};

#endif // MYGRAPHICSVIEW_H
