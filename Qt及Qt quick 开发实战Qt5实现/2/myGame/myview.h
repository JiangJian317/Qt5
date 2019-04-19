#ifndef MYVIEW_H
#define MYVIEW_H

#include <QGraphicsView>
#include <QMediaPlayer>

class BoxGroup;

class MyView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MyView(QWidget* parent = 0);
public slots:
    void startGame();                      //开始游戏
    void clearFullRows();                  //清除满行
    void moveBox();                        //下移满行上面的所有小方块
    void gameOver();                       //游戏结束
    void restartGame();                    //重新开始游戏
    void finishGame();                     //结束游戏
    void pauseGmae();                      //暂停游戏
    void returnGame();                     //返回游戏
    void aboutToFinish(QMediaPlayer::MediaStatus status);
protected:
    void keyPressEvent(QKeyEvent *event);
private:
    void initView();
    void initGame();
    void updateScore(const int fullrowNums = 0);                  //更新分数
private:
    BoxGroup* boxGroup;                       //当前方块组
    BoxGroup* nextBoxGroup;                    //提示方块组
    //游戏区域边框线
    QGraphicsLineItem* topLine;
    QGraphicsLineItem* bottomLine;
    QGraphicsLineItem* leftLine;
    QGraphicsLineItem* rightLine;
    qreal gameSpeed;                       //当前游戏速度
    QList<int>rows;                            //保存满行的位置
    QGraphicsTextItem* gameScoreText;         //显示分数
    QGraphicsTextItem* gameLevelText;            //显示等级
    QGraphicsWidget* maskWidget;              //遮罩面板
    //各种按钮
    QGraphicsWidget* startButton;
    QGraphicsWidget* finishButton;
    QGraphicsWidget* restartButton;
    QGraphicsWidget* pauseButton;
    QGraphicsWidget* optionButton;
    QGraphicsWidget* returnButton;
    QGraphicsWidget* helpButton;
    QGraphicsWidget* exitButton;
    QGraphicsWidget* showMenuButton;
    //各种文本
    QGraphicsTextItem* gameWelcomeText;
    QGraphicsTextItem* gamePauseText;
    QGraphicsTextItem* gameOverText;
    //声音播放
    QMediaPlayer* backgroundMusic;
    QMediaPlayer* clearRowSound;
};

#endif // MYVIEW_H





