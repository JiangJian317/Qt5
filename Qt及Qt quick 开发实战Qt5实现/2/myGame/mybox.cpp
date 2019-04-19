#include "mybox.h"
#include <QPainter>
#include <QKeyEvent>
#include <QTimer>
#include <QList>
#include <QDebug>

OneBox::OneBox(const QColor &color):brushColor(color){}

QRectF OneBox::boundingRect()const
{
    qreal penWidth =1;
    return QRectF(-10-penWidth/2,-10-penWidth/2,20+penWidth,20+penWidth);
}

void OneBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
 painter->drawPixmap(-10,-10,20,20,QPixmap(":/images/box.gif"));      //为小方块使用贴图
 painter->setBrush(this->brushColor);
 QColor penColor = brushColor;
 penColor.setAlpha(20);               //减小颜色透明度
 painter->setPen(penColor);
 painter->drawRect(-10,-10,20,20);
}


QPainterPath OneBox::shape()const
{
    QPainterPath path;
    path.addRect(-9.5,-9.5,19,19);          //形状比边框矩形小0.5像素，这样方块组中的小方块才不会发生碰撞
    return path;
}


BoxGroup::BoxGroup()
{
    setFlags(QGraphicsItem::ItemIsFocusable);              //设置所有Item可以接受焦点
    oldTransform = transform();                            //保存矩阵的原状态，使得可以还原矩阵
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(moveOneStep()));
    currentShape = RandomShape;
}


QRectF BoxGroup::boundingRect()const
{
    qreal penWidth = 1;
    return QRectF(-40-penWidth/2,-40-penWidth/2,80+penWidth,80+penWidth);
}


void BoxGroup::keyPressEvent(QKeyEvent *event)
{
  switch(event->key())
    {
  case Qt::Key_Down:
  this->moveBy(0,20);
  if(isColliding())
  {
  this->moveBy(0,-20);
  clearBoxGroup();                 //*************将小方块从方块组中移动到场景中
  emit needNewBox();                    //需要显示新的方块
  }
      break;
  case Qt::Key_Left:
      this->moveBy(-20,0);
      if(isColliding())
      this->moveBy(20,0);
      break;
  case Qt::Key_Right:
  this->moveBy(20,0);
  if(isColliding())
      moveBy(-20,0);
      break;
  case Qt::Key_Up:
//     this->setRotation(90);              //**********************
      setRotation(rotation()+90);
//      qDebug()<<rotation();
  if(isColliding())
  setRotation(rotation()-90);
      break;
  case Qt::Key_Space:              //空格键实现快速坠落
  moveBy(0,20);
  while(!isColliding())
      moveBy(0,20);
  moveBy(0,-20);
  clearBoxGroup();                       //*********************
  emit needNewBox();
      break;
    }
}



bool BoxGroup::isColliding()                 //碰撞检测函数
{
QList<QGraphicsItem*>itemList = childItems();           //itemList代表一个四个小方块组成的方块组
QGraphicsItem* item;
foreach (item, itemList)
{
if(item->collidingItems().count()>1)                    //如果与方块组碰撞的图形项数目超过1，则发生碰撞
    return true;
}
    return false;
}


void BoxGroup::clearBoxGroup(bool destroyBox)
{
   QList<QGraphicsItem*> itemList = this->childItems();
   QGraphicsItem* item;
   foreach (item, itemList)
   {
       removeFromGroup(item);
       if(destroyBox)
       {
           OneBox* box = (OneBox*)item;
           box->deleteLater();
       }
   }
}


void BoxGroup::createBox(const QPointF &point, BoxShape shape)               //创建方块,即在点point出创建一个编号为shape的图形
{
 static const QColor colorTable[7]={                               //七种颜色，每种方块对应一种颜色
     QColor(200,0,0,100),QColor(255,200,0,100),
     QColor(0,0,200,100),QColor(0,200,0,100),
     QColor(0,200,255,100),QColor(200,0,255,100),
     QColor(150,100,100,100)
 };
   int shapeID = shape;
   if(shape == RandomShape)
     shapeID = qrand()%7;                                        //产生0 - 6之间的随机数
   QColor color = colorTable[shapeID];
   QList<OneBox*>list;
     setRotation(0);                               //此项必须设置，保证新的方块组不受上一个所带来的影响
//   resetTransform();                            //效果图下面一行代码等同
   setTransform(oldTransform);                       //恢复方块组的变换矩阵,但必须知道，先前已经变换过的Item的改变不可挽回
   for(int i=0;i<4;++i)
   {
   OneBox* temp = new OneBox(color);
   list<<temp;
   this->addToGroup(temp);
   }

   switch(shapeID)
   {
   case IShape:
   currentShape = IShape;
   list.at(0)->setPos(-30,-10);
   list.at(1)->setPos(-10,-10);
   list.at(2)->setPos(10,-10);
   list.at(3)->setPos(30,-10);
   break;

   case JShape:
   currentShape = JShape;
   list.at(0)->setPos(10,-10);
   list.at(1)->setPos(10,10);
   list.at(2)->setPos(-10,30);
   list.at(3)->setPos(10,30);
   break;

   case LShape:
   currentShape = LShape;
   list.at(0)->setPos(-10,-10);
   list.at(1)->setPos(-10,10);
   list.at(2)->setPos(-10,30);
   list.at(3)->setPos(10,30);
   break;

   case OShape:
   currentShape = OShape;
   list.at(0)->setPos(-10,-10);
   list.at(1)->setPos(10,-10);
   list.at(2)->setPos(-10,10);
   list.at(3)->setPos(10,10);
   break;

   case SShape:
   currentShape = SShape;
   list.at(0)->setPos(10,-10);
   list.at(1)->setPos(30,-10);
   list.at(2)->setPos(-10,10);
   list.at(3)->setPos(10,10);
   break;

   case TShape:
   currentShape = TShape;
   list.at(0)->setPos(-10,-10);
   list.at(1)->setPos(10,-10);
   list.at(2)->setPos(30,-10);
   list.at(3)->setPos(10,10);
   break;

   case ZShape:
   currentShape = ZShape;
   list.at(0)->setPos(-10,-10);
   list.at(1)->setPos(10,-10);
   list.at(2)->setPos(10,10);
   list.at(3)->setPos(30,10);
   break;

   default:
   break;
   }
   this->setPos(point);          //设置位置
   if(isColliding())             //如果一开始就发生碰撞，说明游戏已经结束
   {
     stopTimer();
     emit gameFinished();          //发射游戏结束信号
   }
}



void BoxGroup::startTimer(int interval)            //开启定时器
{
    timer->start(interval);
}


void BoxGroup::moveOneStep()
{
    moveBy(0,20);
    if(isColliding())
    {
    moveBy(0,-20);
    clearBoxGroup();
    emit needNewBox();
    }
}


void BoxGroup::stopTimer()               //停止定时器
{
    timer->stop();
}












