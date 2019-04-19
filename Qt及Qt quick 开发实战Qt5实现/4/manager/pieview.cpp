#include <math.h>
#include <QtGui>
#include <QScrollBar>

#ifndef M_PI
#define M_PI
#endif

#include "pieview.h"

PieView::PieView(QWidget* parent):QAbstractItemView(parent)
{
horizontalScrollBar()->setRange(0,0);
verticalScrollBar()->setRange(0,0);

margin = 8;
totalSize = 230;
pieSize = totalSize - 2*margin;
validItems = 0;
totalValue = 0.0;
rubberBand = 0;
}

void PieView::dataChanged(const QModelIndex &topLeft,const QModelIndex &bottomRight)
{
    QAbstractItemView::dataChanged(topLeft,bottomRight);

    validItems = 0;
    totalValue = 0.0;

    for(int row = 0;row < model()->rowCount(rootIndex());++row)
    {
        QModelIndex index = model()->index(row,1,rootIndex());
        double value = model()->data(index).toDouble();

        if(value>0.0)
        {
            totalValue += value;
            validItems++;
        }
    }
    this->viewport()->update();
}

bool PieView::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event)
{
if(index.column() == 0)
  return QAbstractItemView::edit(index,trigger,event);
else
  return false;
}

QModelIndex PieView::indexAt(const QPoint &point) const  //返回视口中点point处的模型索引
{
    if(validItems == 0)
      return QModelIndex();

    int wx = point.x() + horizontalScrollBar()->value();
    int wy = point.y() + verticalScrollBar()->value();

    if(wx < totalSize)
    {
        double cx = wx - totalSize/2;
        double cy = totalSize/2 - wy;

        double d = pow(pow(cx,2) + pow(cy,2),0.5);

        if(d == 0 || d > pieSize/2)
          return QModelIndex();
       double angle = (180 / M_PI)* acos(cx/d);
        if(cy < 0)
        angle = 360 - angle;

        double startAngle = 0.0;

        for(int row = 0; row<model()->rowCount(rootIndex());++row)
        {
            QModelIndex index = model()->index(row,1,rootIndex());
            double value = model()->data(index).toDouble();

            if(value > 0.0)
            {
                double sliceAngle = 360*value/totalValue;

                if(angle >= startAngle && angle < (startAngle + sliceAngle))
                    return model()->index(row,1,rootIndex());

                startAngle += sliceAngle;
            }
        }
    }
        else
        {
     double itemHeight = QFontMetrics(viewOptions().font).height();
     int listItem = int((wy - margin) / itemHeight);
     int validRow = 0;

     for(int row = 0;row<model()->rowCount(rootIndex());++row)
         {
        QModelIndex index = model()->index(row,1,rootIndex());
        if(model()->data(index).toDouble() > 0.0)
          {
        if(listItem == validRow)
            return model()->index(row,0,rootIndex());
          validRow++;
          }
         }
        }
return QModelIndex();
}

bool PieView::isIndexHidden(const QModelIndex &) const
{
    return false;
}


QRect PieView::itemRect(const QModelIndex &index) const
{
    if(!index.isValid())       //如果模型索引无效
       return QRect();

    QModelIndex valueIndex;

    if(index.column() != 1)
      valueIndex = model()->index(index.row(),1,rootIndex());
    else
        valueIndex = index;

    if(model()->data(valueIndex).toDouble() > 0.0)
    {
        int listItem = 0;
        for(int row = index.row() - 1;row>=0;--row)
          {
        if(model()->data(model()->index(row,1,rootIndex())).toDouble() > 0.0)
          listItem++;
          }
 double itemHeight;
 switch(index.column())
      {
 case 0:
   itemHeight = QFontMetrics(viewOptions().font).height();
   return QRect(totalSize,int(margin + listItem*itemHeight),
                totalSize - margin,int(itemHeight));

 case 1:
     return viewport()->rect();
      }

    }
    return QRect();
}

QRegion PieView::itemRegion(const QModelIndex &index) const
{
if(!index.isValid())
  return QRegion();
if(index.column() != 1)
 return itemRect(index);
if(model()->data(index).toDouble() <= 0.0)
    return QRegion();

double startAngle = 0.0;
for(int row = 0;row < model()->rowCount(rootIndex());++row)
{
 QModelIndex sliceIndex = model()->index(row,1,rootIndex());
 double value = model()->data(sliceIndex).toDouble();

if(value > 0.0)
 {
 double angle = 360*value/totalValue;

 if(sliceIndex == index)
  {
  QPainterPath slicePath;
  slicePath.moveTo(totalSize/2,totalSize/2);
  slicePath.arcTo(margin,margin,margin+pieSize,margin+pieSize,startAngle,angle);
  slicePath.closeSubpath();

  return QRegion(slicePath.toFillPolygon().toPolygon());
  }
startAngle += angle;
 }

}
return QRegion();
}


int PieView::horizontalOffset()const
{
    return horizontalScrollBar()->value();
}

void PieView::mousePressEvent(QMouseEvent *event)
{
QAbstractItemView::mousePressEvent(event);
origin = event->pos();
if(!rubberBand)
  rubberBand = new QRubberBand(QRubberBand::Rectangle,viewport());
rubberBand->setGeometry(QRect(origin,QSize()));
rubberBand->show();
}


void PieView::mouseMoveEvent(QMouseEvent *event)
{
    if(rubberBand)
       rubberBand->setGeometry(QRect(origin,event->pos()).normalized());
   QAbstractItemView::mouseMoveEvent(event);
}


void PieView::mouseReleaseEvent(QMouseEvent *event)
{
    QAbstractItemView::mouseReleaseEvent(event);
    if(rubberBand)
        rubberBand->hide();
    viewport()->update();
}


QModelIndex PieView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers)
{
 QModelIndex current = currentIndex();
   switch(cursorAction)
     {
   case MoveLeft:
   case MoveUp:
      if(current.row() > 0)
       current = model()->index(current.row() - 1,current.column(),
                                rootIndex());
      else
        current = model()->index(0,current.column(),rootIndex());
       break;
     case MoveRight:
     case MoveDown:
        if(current.row() < rows(current) - 1)  //如果不在最后一行
           current = model()->index(current.row() + 1,current.column(),
                                    rootIndex());
        else
            current = model()->index(rows(current) - 1,current.column(),
                                     rootIndex());
       break;
   default:
       break;
     }
    viewport()->update();
    return current;
}


void PieView::paintEvent(QPaintEvent *event)
{
    QItemSelectionModel* selections = selectionModel();
    QStyleOptionViewItem option = viewOptions();

    QBrush background = option.palette.base();
    QPen foreground(option.palette.color(QPalette::WindowText));

    QPainter painter(viewport());  //指定绘画设备为视口
    painter.setRenderHint(QPainter::Antialiasing);  //抗锯齿

    painter.fillRect(event->rect(),background);
    painter.setPen(foreground);

    QRect pieRect = QRect(margin,margin,pieSize,pieSize);

//    if(validItems > 0)
//    {
      //绘制圆形饼状图
     painter.save();
     painter.translate(pieRect.x() - horizontalScrollBar()->value(),
                       pieRect.y() - verticalScrollBar()->value());
     painter.drawEllipse(0,0,pieSize,pieSize);
     double startAngle = 0.0;
     int row;

     for(row =0;row<model()->rowCount(rootIndex());++row)
        {
        QModelIndex index = model()->index(row,1,rootIndex());
        double value = model()->data(index).toDouble();

        if(value > 0.0)
           {
           double angle = 360*value/totalValue;

           QModelIndex colorIndex = model()->index(row,0,rootIndex());
           QColor color = QColor(model()->data(colorIndex,Qt::DecorationRole).toString());
           if(currentIndex() == index)
             painter.setBrush(QBrush(color,Qt::Dense4Pattern));
           else if(selections->isSelected(index))  //index所对应的item被选中
             painter.setBrush(QBrush(color,Qt::Dense3Pattern));
            else
               painter.setBrush(QBrush(color));
            painter.drawPie(0,0,pieSize,pieSize,int(startAngle*16),
                            int(angle*16));
           startAngle += angle;
           }
        }
     painter.restore();
     //绘制饼图旁边的图示
     int keyNumber = 0;

     for(row =0;row<model()->rowCount(rootIndex());++row)
     {
         QModelIndex index = model()->index(row,1,rootIndex());
          double value = model()->data(index).toDouble();

          if(value > 0.0)
          {
              QModelIndex labelIndex = model()->index(row,0,rootIndex());

              QStyleOptionViewItem option = viewOptions();
              option.rect = visualRect(labelIndex);
              if(selections->isSelected(labelIndex))
                  option.state |= QStyle::State_Selected;   //即按位或后赋值
              if(currentIndex() == labelIndex)
              option.state |= QStyle::State_HasFocus;
            itemDelegate()->paint(&painter,option,labelIndex);
            keyNumber++;
          }
     }
//  }
}


void PieView::resizeEvent(QResizeEvent *)
{
 updateGeometries();
}


int PieView::rows(const QModelIndex &index) const
{
    return model()->rowCount(model()->parent(index));
}

void PieView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    for(int row = start;row <= end;++row)
      {
        QModelIndex index = model()->index(row,1,rootIndex());
        double value = model()->data(index).toDouble();

        if(value > 0.0)
        {
            totalValue += value;
            validItems++;
        }
      }
 QAbstractItemView::rowsInserted(parent,start,end);
}


void PieView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    for(int row = start;row <= end;++row)
      {
      QModelIndex index = model()->index(row,1,rootIndex());
       double value = model()->data(index).toDouble();
       if(value > 0.0)
        {
        totalValue -= value;
        validItems--;
        }
      }
QAbstractItemView::rowsAboutToBeRemoved(parent,start,end);
}


void PieView::scrollContentsBy(int dx, int dy)
{
    viewport()->scroll(dx,dy);
}

void PieView::scrollTo(const QModelIndex &index, ScrollHint)
{
    QRect area = viewport()->rect();
    QRect rect = visualRect(index);

    if(rect.left() < area.left())
     horizontalScrollBar()->setValue(
                 horizontalScrollBar()->value() + rect.left() - area.left());
    else if(rect.right() > area.right())
        horizontalScrollBar()->setValue(
            horizontalScrollBar()->value() + qMin(
             rect.right()-area.right(),rect.left() - area.left()));
    if (rect.top() < area.top())
        verticalScrollBar()->setValue(
            verticalScrollBar()->value() + rect.top() - area.top());
    else if (rect.bottom() > area.bottom())
        verticalScrollBar()->setValue(
            verticalScrollBar()->value() + qMin(
                rect.bottom() - area.bottom(), rect.top() - area.top()));

    /*viewport()->*/update();                  //没有参数?????????
}

void PieView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    QRect contentsRect = rect.translated(horizontalScrollBar()->value(),
                                        verticalScrollBar()->value()).normalized();
    int rows = model()->rowCount(rootIndex());
    int columns = model()->columnCount(rootIndex());
    QModelIndexList indexes;

    for(int row = 0;row < rows;++row)
      {
      for(int column = 0;column < columns;++column)
        {
        QModelIndex index = model()->index(row,column,rootIndex());
        QRegion region = itemRegion(index);
        if(!region.intersected(contentsRect).isEmpty()) //**************
          indexes.append(index);
        }
      }
    if(indexes.size() > 0)
      {
      int firstRow = indexes[0].row();
      int lastRow = indexes[0].row();
      int firstColumn = indexes[0].column();
      int lastColumn = indexes[0].column();

      for(int i = 1;i<indexes.size();++i)
        {
        firstRow = qMin(firstRow,indexes[i].row());
        lastRow = qMax(lastRow,indexes[i].row());
        firstColumn = qMin(firstColumn,indexes[i].column());
        lastColumn = qMax(lastColumn,indexes[i].column());
        }
      QItemSelection selection(model()->index(firstRow,firstColumn,rootIndex()),
                               model()->index(lastRow,lastColumn,rootIndex()));
      selectionModel()->select(selection,command);
      }
    else
      {
      QModelIndex noIndex;
      QItemSelection selection(noIndex,noIndex);
      selectionModel()->select(selection,command);
      }
    /*viewport()->*/update();           //????????????
}


void PieView::updateGeometries()
{
    horizontalScrollBar()->setPageStep(viewport()->width());
    horizontalScrollBar()->setRange(0,qMax(0,2*totalSize - viewport()->width()));
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0,qMax(0,totalSize - viewport()->height()));
}


int PieView::verticalOffset()const
{
    return verticalScrollBar()->value();
}

QRect PieView::visualRect(const QModelIndex &index) const
{
    QRect rect = itemRect(index);
     if(rect.isValid())
       return QRect(rect.left() - horizontalScrollBar()->value(),
                    rect.top() - verticalScrollBar()->value(),
                    rect.width(),rect.height());
    else
         return rect;
}


QRegion PieView::visualRegionForSelection(const QItemSelection &selection) const
{
int ranges = selection.count();

if(ranges == 0)
  return QRect();

QRegion region;
for(int i = 0;i<ranges;++i)
  {
  QItemSelectionRange range = selection.at(i);
  for(int row = range.top();row <= range.bottom(); ++row)
   {
   for(int col = range.left();col <= range.right();++col)
      {
      QModelIndex index = model()->index(row,col,rootIndex());
       region += visualRect(index);
      }
   }
  }
return region;
}
























