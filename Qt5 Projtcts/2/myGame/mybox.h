#ifndef MYBOX_H
#define MYBOX_H

#include <QGraphicsItemGroup>
#include <QGraphicsObject>      //QGraphicsObject继承自QObject和QGraphicsItem
                                //要实现自定义的图形项，首先要创建一个继承自QGraphicsItem或者其子类的类
                                //然后重新实现它的两个纯虚函数boundingRect()和paint();
                                //前者返回要绘制图形项的矩形区域，后者用来执行实际的绘图操作
                                //要实现碰撞检测，则必须重新实现QGraphicsItem的shape()函数
class OneBox:public QGraphicsObject
{
public:
    OneBox(const QColor& color = Qt::red);
    QRectF boundingRect()const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPainterPath shape() const;
private:
    QColor brushColor;
};

//方块组类
class BoxGroup:public QObject,public QGraphicsItemGroup
{
Q_OBJECT
public:
    enum BoxShape{IShape,JShape,LShape,OShape,SShape,TShape,ZShape,RandomShape};
    BoxGroup();
    QRectF boundingRect()const;
    bool isColliding();
    void createBox(const QPointF& point = QPointF(0,0),BoxShape shape = RandomShape);
    void clearBoxGroup(bool destroyBox = false);
    BoxShape getCurrentShape(){return this->currentShape;}
public slots:
    void moveOneStep();
    void startTimer(int interval);
    void stopTimer();
protected:
    void keyPressEvent(QKeyEvent *event);
  signals:
void needNewBox();
void gameFinished();
private:
    BoxShape currentShape;
    QTransform oldTransform;
    QTimer* timer;
};


#endif // MYBOX_H
