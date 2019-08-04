#ifndef MYLRC_H
#define MYLRC_H

#include <QLabel>

class MyLrc : public QLabel
{
    Q_OBJECT
public:
    explicit MyLrc(QWidget* parent = 0);
    void startLrcMask(qint64 intervalTime);
    void stopLrcMask();
protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
private slots:
    void timeout();
private:
    QLinearGradient linearGradient;                  //线性渐变
    QLinearGradient maskLinearGradient;
    QFont font;
    QTimer* timer;
    qreal lrcMaskWidth;                          //遮罩宽度

    //每次歌词遮罩增加的宽度
    qreal lrcMaskWidthInterval;
    QPoint offset;
};

#endif // MYLRC_H
