#include "myview.h"
#include "mybox.h"
#include <QIcon>
#include <QPropertyAnimation>
#include <QGraphicsBlurEffect>
#include <QTimer>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <QApplication>
#include <QLabel>
#include <QFileInfo>
#include <QDebug>
#include <QSlider>

static const qreal INITSPEED = 500;     //初始游戏速度
static const QString SOUNDPATH = "../myGame/sounds/";


MyView::MyView(QWidget* parent):QGraphicsView(parent)
{
initView();
}


void MyView::initView()                           //初始化游戏界面
{
  this->setRenderHint(QPainter::Antialiasing);           //抗锯齿渲染
  this->setCacheMode(CacheBackground);         //***************
  this->setWindowTitle(tr("MyBox方块游戏"));
  this->setWindowIcon(QIcon(":/images/icon.png"));
  this->setMinimumSize(810,510);
  this->setMaximumSize(810,510);

    //设置场景
    QGraphicsScene* scene = new QGraphicsScene;
    scene->setSceneRect(5,5,800,500);             //设置场景矩形
    scene->setBackgroundBrush(QPixmap(":/images/background.png"));
    this->setScene(scene);
    //方块可移动区域的4条边界线
    topLine = scene->addLine(197,47,403,47);
    bottomLine = scene->addLine(197,453,403,453);
    leftLine = scene->addLine(197,47,197,453);
    rightLine = scene->addLine(403,47,403,453);

    //当前方块组和提示方块组
    boxGroup = new BoxGroup;
    connect(boxGroup,SIGNAL(needNewBox()),this,SLOT(clearFullRows()));
    connect(boxGroup,SIGNAL(gameFinished()),this,SLOT(gameOver()));
    scene->addItem(boxGroup);
    nextBoxGroup = new BoxGroup;
    scene->addItem(nextBoxGroup);

    //得分文本
    gameScoreText = new QGraphicsTextItem();//************
    scene->addItem(gameScoreText);//*************
    gameScoreText->setPos(650,350);
    gameScoreText->setFont(QFont("Times",20,QFont::Bold));


    //级别文本
    gameLevelText = new QGraphicsTextItem();
    scene->addItem(gameLevelText);
    gameLevelText->setPos(20,150);
    gameLevelText->setFont(QFont("Times",30,QFont::Bold));

    //设置初始状态边框为隐藏状态
    topLine->hide();
    bottomLine->hide();
    leftLine->hide();
    rightLine->hide();
    gameScoreText->hide();
    gameLevelText->hide();

//黑色遮罩
    QWidget* mask = new QWidget;
    mask->setAutoFillBackground(true);
    mask->setPalette(QPalette(QColor(0,0,0,80)));
    mask->resize(900,600);
    maskWidget = scene->addWidget(mask);
    maskWidget->setPos(-50,-50);
    //设置其Z值为1，这样可以处于Z值为0的图形项的上面
    maskWidget->setZValue(1);

    //选项面板
    QWidget* option = new QWidget;
    QPushButton* optionCloseButton = new QPushButton(tr("关   闭"), option);
    QPalette palette;
    palette.setColor(QPalette::ButtonText,Qt::black);
    optionCloseButton->setPalette(palette);
    optionCloseButton->move(120,300);
    connect(optionCloseButton,SIGNAL(clicked(bool)),option,SLOT(hide()));       //*************
    option->setAutoFillBackground(true);
    option->setPalette(QPalette(QColor(0,0,0,180)));
    option->resize(300,400);
    QGraphicsWidget* optionWidget = scene->addWidget(option);
    optionWidget->setPos(250,50);
    optionWidget->setZValue(3);
    optionWidget->hide();

    //帮助面板
    QWidget* help = new QWidget;
    QPushButton* helpCloseButton = new QPushButton(tr("关闭"),help);
    helpCloseButton->setPalette(palette);
    helpCloseButton->move(120,300);
    connect(helpCloseButton,SIGNAL(clicked(bool)),help,SLOT(hide()));
    help->setAutoFillBackground(true);
    help->setPalette(QPalette(QColor(0,0,0,180)));
    help->resize(300,400);
    QGraphicsWidget* helpWidget = scene->addWidget(help);
    helpWidget->setPos(250,50);
    helpWidget->setZValue(3);
    helpWidget->hide();


    QLabel* helpLabel = new QLabel(help);
    helpLabel->setText(tr("<h1><font color=white>JiangJian作品"
                          "<br>https://github.com/JiangJian317/Qt.git</font></h1>"));
    helpLabel->setAlignment(Qt::AlignCenter);
    helpLabel->move(30,150);


    //游戏欢迎文本
    gameWelcomeText = new QGraphicsTextItem();  //*******************
    scene->addItem(gameWelcomeText);
    gameWelcomeText->setHtml(tr("<font color=white>MyBox方块游戏</font>"));
    gameWelcomeText->setFont(QFont("Times",30,QFont::Bold));
    gameWelcomeText->setPos(250,100);
    gameWelcomeText->setZValue(2);

    //游戏暂停文本
    gamePauseText = new QGraphicsTextItem();
    scene->addItem(gamePauseText);
    gamePauseText->setHtml(tr("<font color=white>游戏暂停中!</font>"));
    gamePauseText->setFont(QFont("Times",30,QFont::Bold));
    gamePauseText->setPos(300,100);
    gamePauseText->setZValue(2);
    gamePauseText->hide();          //初始化游戏时隐藏该文本

    //游戏结束文本
    gameOverText = new QGraphicsTextItem();
    scene->addItem(gameOverText);
    gameOverText->setHtml(tr("<font color=white>游戏结束!</font>"));
    gameOverText->setFont(QFont("Times",30,QFont::Bold));
    gameOverText->setPos(320,100);
    gameOverText->hide();


    //游戏中使用的按钮
    QPushButton* button1 = new QPushButton(tr("开    始"));
    QPushButton* button2 = new QPushButton(tr("选    项"));
    QPushButton* button3 = new QPushButton(tr("帮    助"));
    QPushButton* button4 = new QPushButton(tr("退    出"));
    QPushButton* button5 = new QPushButton(tr("重新开始"));
    QPushButton* button6 = new QPushButton(tr("暂    停"));
    QPushButton* button7 = new QPushButton(tr("主 菜 单"));
    QPushButton* button8 = new QPushButton(tr("返回游戏"));
    QPushButton* button9 = new QPushButton(tr("结束游戏"));

    //连接各个按钮与相应的槽
    connect(button1,SIGNAL(clicked(bool)),this,SLOT(startGame()));
    connect(button2,SIGNAL(clicked(bool)),option,SLOT(show()));
    connect(button3,SIGNAL(clicked(bool)),help,SLOT(show()));
    connect(button4,SIGNAL(clicked(bool)),qApp,SLOT(quit()));
    connect(button5,SIGNAL(clicked(bool)),this,SLOT(restartGame()));
    connect(button6,SIGNAL(clicked(bool)),this,SLOT(pauseGmae()));
    connect(button7,SIGNAL(clicked(bool)),this,SLOT(finishGame()));
    connect(button8,SIGNAL(clicked(bool)),this,SLOT(returnGame()));
    connect(button9,SIGNAL(clicked(bool)),this,SLOT(finishGame()));


    startButton = scene->addWidget(button1);
    optionButton = scene->addWidget(button2);
    helpButton = scene->addWidget(button3);
    exitButton = scene->addWidget(button4);
    restartButton = scene->addWidget(button5);
    pauseButton = scene->addWidget(button6);
    showMenuButton = scene->addWidget(button7);
    returnButton = scene->addWidget(button8);
    finishButton = scene->addWidget(button9);



    startButton->setPos(370,200);
    optionButton->setPos(370,250);
    helpButton->setPos(370,300);
    exitButton->setPos(370,350);
    restartButton->setPos(600,150);
    pauseButton->setPos(600,200);
    showMenuButton->setPos(600,250);
    returnButton->setPos(370,200);
    finishButton->setPos(370,250);

    startButton->setZValue(2);
    optionButton->setZValue(2);
    helpButton->setZValue(2);
    exitButton->setZValue(2);
    restartButton->setZValue(2);
    returnButton->setZValue(2);
    finishButton->setZValue(2);

    restartButton->hide();
    finishButton->hide();
    pauseButton->hide();
    showMenuButton->hide();
    returnButton->hide();

    backgroundMusic = new QMediaPlayer(this);
    clearRowSound = new QMediaPlayer(this);
    QSlider* volume1 = new QSlider(Qt::Horizontal,option);
    QSlider* volume2 = new QSlider(Qt::Horizontal,option);
    volume1->setMaximum(100);
    volume1->setValue(50);
    volume2->setMaximum(100);
    volume2->setValue(50);
    QLabel* volumeLabel1 = new QLabel(tr("音乐:"),option);
    QLabel* volumeLabel2 = new QLabel(tr("音效:"),option);
    volume1->move(100,100);
    volume2->move(100,200);
    volumeLabel1->move(60,105);
    volumeLabel2->move(60,205);
    connect(volume1,SIGNAL(valueChanged(int)),backgroundMusic,SLOT(setVolume(int)));
    connect(volume2,SIGNAL(valueChanged(int)),clearRowSound,SLOT(setVolume(int)));
    connect(backgroundMusic,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this,SLOT(aboutToFinish(QMediaPlayer::MediaStatus)));
    //因为行销毁时的背景音乐在播放完毕时会进入暂停状态，再调用play()将无法进行播放
    //需要在播放完毕后使其进入停止状态
    connect(clearRowSound,SIGNAL(stateChanged(QMediaPlayer::State)),clearRowSound,SLOT(stop()));
    backgroundMusic->setMedia(QUrl::fromLocalFile(SOUNDPATH+"background01.mp3"));
    backgroundMusic->play();
    clearRowSound->setMedia(QUrl::fromLocalFile(SOUNDPATH+"clearRow.mp3"));

}


void MyView::startGame()                      //开始游戏
{
    gameWelcomeText->hide();
    startButton->hide();
    optionButton->hide();
    helpButton->hide();
    exitButton->hide();
    maskWidget->hide();

    initGame();
}


void MyView::initGame()                      //初始化游戏
{
    boxGroup->createBox(QPointF(300,70));           //创建当前方块组
    boxGroup->setFocus();                          //设置当前方块组获得焦点
    boxGroup->startTimer(INITSPEED);
    gameSpeed = INITSPEED;
    nextBoxGroup->createBox(QPointF(500,70));            //创建提示方块组

    this->scene()->setBackgroundBrush(QPixmap(":/images/background01.png"));
    gameScoreText->setHtml(tr("<font color=red>0</font>"));
    gameLevelText->setHtml(tr("<font color=white>青<br>铜<br>级</font>"));

    restartButton->show();
    pauseButton->show();
    showMenuButton->show();
    gameScoreText->show();
    gameLevelText->show();
    topLine->show();
    bottomLine->show();
    leftLine->show();
    rightLine->show();
    //了能以前返回主菜单时隐藏了boxGroup
    boxGroup->show();
    backgroundMusic->setMedia(QUrl::fromLocalFile(SOUNDPATH+"background01.mp3"));
    backgroundMusic->play();

}


void MyView::clearFullRows()                        //清空满行
{
    //获取比一行方格较大的矩形中包含的所有小方块
    for(int y=429;y>50;y-=20)   //由for循环可知y的可取值范围是[69,429],因为scene最顶层的那一行不可能存在方块，所以游戏未结束时
    {                            //方块y轴的最小取值是50+20，因此当y=69时刚好可以覆盖所有的方块
    QList<QGraphicsItem*> list = this->scene()->items(199,y,202,22,
                                               Qt::ContainsItemShape,Qt::DescendingOrder);
    if(list.count() == 10)            //如果该行已满
    {
        foreach(QGraphicsItem* item,list)
        {
      OneBox* box = (OneBox*)item;
//       box->deleteLater();
      QGraphicsBlurEffect* blurEffect = new QGraphicsBlurEffect;            //模糊图形效果
      box->setGraphicsEffect(blurEffect);
      QPropertyAnimation* animation = new QPropertyAnimation(box,"scale");
      animation->setEasingCurve(QEasingCurve::OutBounce);
      animation->setDuration(250);                           //动画持续205ms
      animation->setStartValue(4);                           //开始时scale值为4，结束时为0.25
      animation->setEndValue(0.25);
      animation->start(QAbstractAnimation::DeleteWhenStopped);
      connect(animation,SIGNAL(finished()),box,SLOT(deleteLater()));
        }
        rows<<y;                //保存满行的位置
    }
    }
  //如果有满行，下移满行上面的各行再出现新的方块组，没有满行则直接出现新的方块组
    if(rows.count()>0)
    {
//        moveBox();
        clearRowSound->play();         //播放满行销毁时音效
        QTimer::singleShot(400,this,SLOT(moveBox()));          //销毁一行之后0.4s再下移
    }
    else
    {
        boxGroup->createBox(QPointF(300,70),nextBoxGroup->getCurrentShape());
        nextBoxGroup->clearBoxGroup(true);           //清空并销毁提示方块组中的所有小方块
        nextBoxGroup->createBox(QPointF(500,70));
    }
}


void MyView::moveBox()                     //下移满行上面的所有小方块
{
    for(int i=rows.count();i>0;--i)         //从位置最靠上的满行开始
    {
        int row=rows.at(i-1);
     foreach(QGraphicsItem* item,scene()->items(199,49,202,row - 47,Qt::ContainsItemShape,Qt::DescendingOrder))
     {
       item->moveBy(0,20);
     }
    }
updateScore(rows.count());                 //更新分数
rows.clear();                            //将满行列表清空为0
//等所有行下移以后再出现新的方块组
boxGroup->createBox(QPointF(300,70),nextBoxGroup->getCurrentShape());
nextBoxGroup->clearBoxGroup(true);
nextBoxGroup->createBox(QPointF(500,70));
}



void MyView::updateScore(const int fullrowNums)    //更新分数       //更新分数
{
//int score = fullrowNums*100;
    int score=0;
    switch(fullrowNums)
    {
    case 1: score=100;  break;
    case 2: score=300;  break;
    case 3: score=400;  break;
    case 4: score=600;  break;
    }
int currentScore = gameScoreText->toPlainText().toInt();
currentScore += score;
gameScoreText->setHtml(tr("<font color = red>%1</font>").arg(currentScore));
//判断级别
if(currentScore<500){}                              //第一级，什么也不做
else if(currentScore<1000)                          //第二级
{
gameLevelText->setHtml(tr("<font color = white>白<br>银<br>级</font>"));            //<br>标签强制句子在一行结束
this->scene()->setBackgroundBrush(QPixmap(":/images/background02.png"));
this->gameSpeed = 400;
boxGroup->stopTimer();
boxGroup->startTimer(gameSpeed);
if(backgroundMusic->currentMedia()!= QUrl::fromLocalFile(SOUNDPATH+"background03.mp3"))
{
    backgroundMusic->setMedia(QUrl::fromLocalFile(SOUNDPATH+"background03.mp3"));
    backgroundMusic->play();
}
}
else if(currentScore<1500)
{
gameLevelText->setHtml(tr("<font color = darkorange>黄<br>金<br>级</font>"));
this->scene()->setBackgroundBrush(QPixmap(":/images/background.png"));
this->gameSpeed = 300;
boxGroup->stopTimer();
boxGroup->startTimer(gameSpeed);
if(backgroundMusic->currentMedia()!= QUrl::fromLocalFile(SOUNDPATH+"background02.mp3"))
{
    backgroundMusic->setMedia(QUrl::fromLocalFile(SOUNDPATH+"background02.mp3"));
    backgroundMusic->play();
}
}
else if(currentScore<2500)
{
 gameLevelText->setHtml(tr("<font color = blue>钻<br>石<br>级</font>"));
 this->scene()->setBackgroundBrush(QPixmap(":/images/background03.png"));
 this->gameSpeed = 250;
 boxGroup->stopTimer();
 boxGroup->startTimer(gameSpeed);
 if(backgroundMusic->currentMedia()!= QUrl::fromLocalFile(SOUNDPATH+"background01.mp3"))
 {
     backgroundMusic->setMedia(QUrl::fromLocalFile(SOUNDPATH+"background01.mp3"));
     backgroundMusic->play();
 }
}
else
{
    gameLevelText->setHtml(tr("<font color = yellow>王<br>者<br>级</font>"));
    this->scene()->setBackgroundBrush(QPixmap(":/images/background04.png"));
    this->gameSpeed = 200;
    boxGroup->stopTimer();
    boxGroup->startTimer(gameSpeed);
    if(backgroundMusic->currentMedia()!= QUrl::fromLocalFile(SOUNDPATH+"background.mp3"))
    {
        backgroundMusic->setMedia(QUrl::fromLocalFile(SOUNDPATH+"background.mp3"));
        backgroundMusic->play();
    }
}
}


void MyView::gameOver()                      //游戏结束
{
pauseButton->hide();
showMenuButton->hide();
maskWidget->show();
gameOverText->show();
restartButton->setPos(370,200);
finishButton->show();
}

// 重新开始游戏
void MyView::restartGame()
{
maskWidget->hide();
gameOverText->hide();
finishButton->hide();
restartButton->setPos(600,150);

//销毁提示方块组和当前方块移动区域中的所有小方块
nextBoxGroup->clearBoxGroup(true);
boxGroup->clearBoxGroup();          /////////?????
boxGroup->hide();          //下面循环会把boxGroup给delete
foreach(QGraphicsItem* item,this->scene()->items(199, 49, 202, 402, Qt::ContainsItemShape,Qt::DescendingOrder))
{
    // 先从场景中移除小方块，因为使用deleteLater()是在返回主事件循环后才销毁
    // 小方块的，为了在出现新的方块组时不发生碰撞，所以需要先从场景中移除小方块
this->scene()->removeItem(item);
OneBox* box = (OneBox*)item;
box->deleteLater();
}
initGame();
}

// 结束当前正在进行的游戏
void MyView::finishGame()
{
gameOverText->hide();
finishButton->hide();
restartButton->setPos(600,150);
restartButton->hide();
pauseButton->hide();
showMenuButton->hide();
gameScoreText->hide();
gameLevelText->hide();

topLine->hide();
bottomLine->hide();
leftLine->hide();
rightLine->hide();

nextBoxGroup->clearBoxGroup(true);
boxGroup->clearBoxGroup();
boxGroup->hide();

foreach (QGraphicsItem *item, scene()->items(199, 49, 202, 402, Qt::ContainsItemShape,Qt::DescendingOrder))
{
    OneBox *box = (OneBox*) item;
    box->deleteLater();
}
//可能是正在进行游戏时按下主菜单按钮
maskWidget->show();
gameWelcomeText->show();
startButton->show();
optionButton->show();
helpButton->show();
exitButton->show();
this->scene()->setBackgroundBrush(QPixmap(":/images/background.png"));

backgroundMusic->setMedia(QUrl::fromLocalFile(SOUNDPATH+"background01.mp3"));
backgroundMusic->play();

}


// 暂停游戏
void MyView::pauseGmae()
{
boxGroup->stopTimer();
restartButton->hide();
pauseButton->hide();
showMenuButton->hide();
maskWidget->show();
gamePauseText->show();
returnButton->show();
}

// 返回游戏，处于暂停状态时
void MyView::returnGame()
{
    returnButton->hide();
    gamePauseText->hide();
    maskWidget->hide();
    restartButton->show();
    pauseButton->show();
    showMenuButton->show();
    boxGroup->startTimer(gameSpeed);

}

void MyView::keyPressEvent(QKeyEvent *event)
{
//如果正在进行游戏，当键盘按下时总是方块组获得焦点
if(pauseButton->isVisible())
boxGroup->setFocus();
else
boxGroup->clearFocus();
QGraphicsView::keyPressEvent(event);
}

void MyView::aboutToFinish(QMediaPlayer::MediaStatus status)
{
 if(status == QMediaPlayer::EndOfMedia)
    backgroundMusic->play();
}















