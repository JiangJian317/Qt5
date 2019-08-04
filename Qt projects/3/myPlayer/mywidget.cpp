#include "mywidget.h"
#include "ui_mywidget.h"
#include <QLabel>
#include <QToolBar>
#include <QVBoxLayout>
#include <QTime>
#include <QAction>
#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>
#include "myplaylist.h"
#include <QFileDialog>
#include <QDesktopServices>
#include <QStringList>
#include <QStandardPaths>
#include "mylrc.h"
#include <QMenu>
#include <QCloseEvent>

MyWidget::MyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyWidget)
{
    ui->setupUi(this);
    initPlayer();
}

MyWidget::~MyWidget()
{
    delete ui;
}


void MyWidget::initPlayer()             //初始化播放器
{
    //设置主界面标题、图标和大小
    setWindowTitle(tr("MyPlayer音乐播放器"));
    setWindowIcon(QIcon(":/images/icon.png"));
    setMinimumSize(320,160);
    setMaximumSize(320,160);
    //创建媒体图
    mediaObject = new QMediaPlayer(this);
      // 关联媒体对象的positionChanged(qint64)信号来更新播放时间的显示
    connect(mediaObject,SIGNAL(positionChanged(qint64)),this,SLOT(updateTime(qint64)));
    //创建顶部标签，用于显示一些信息
    topLabel = new QLabel(tr("<a href = \" http://www.yafeilinux.com \">欢迎使用SWORD MUSIC PLAYER</a>"));
    topLabel->setTextFormat(Qt::RichText);   //富文本
    topLabel->setOpenExternalLinks(true);
    topLabel->setAlignment(Qt::AlignCenter);
    //创建控制播放进度的滑块
    seekSlider = new QSlider(Qt::Horizontal,this);
    connect(mediaObject,&QMediaPlayer::positionChanged,this,&MyWidget::updatePosition);

   //创建包含播放列表图标、显示时间标签和桌面歌词图标的工具栏
    QToolBar* widgetBar = new QToolBar(this);
    //显示播放时间的标签
    timeLabel = new QLabel(tr("00:00 / 00:00"),this);
    timeLabel->setToolTip(tr("当前时间 / 总时间"));
    timeLabel->setAlignment(Qt::AlignCenter);
    timeLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    //创建图标，用于控制是否显示列表
    QAction* PLAction = new QAction(tr("PL"),this);
    PLAction->setShortcut(QKeySequence("F4"));
    PLAction->setToolTip(tr("播放列表F4"));
    connect(PLAction,SIGNAL(triggered(bool)),this,SLOT(setPlaylistShown()));
    //创建图标，用于控制是否显示卓面歌词
    QAction* LRCAction = new QAction(tr("LRC"),this);
    LRCAction->setShortcut(QKeySequence("F2"));
    LRCAction->setToolTip(tr("桌面歌词(F2)"));
    connect(LRCAction,SIGNAL(triggered(bool)),this,SLOT(setLrcShown()));

    //添加到工具栏
    widgetBar->addAction(PLAction);
    widgetBar->addSeparator();       //添加分隔符
    widgetBar->addWidget(timeLabel);
    widgetBar->addSeparator();
    widgetBar->addAction(LRCAction);

    //创建播放控制动作工具栏
    QToolBar* toolBar = new QToolBar(this);
    //播放动作
   playAction = new QAction(this);
   playAction->setIcon(QIcon(":/images/play.png"));
   playAction->setText(tr("播放(F5)"));
   playAction->setShortcut(QKeySequence(tr("F5")));
   connect(playAction,SIGNAL(triggered(bool)),this,SLOT(setPaused()));
   //停止动作
   stopAction = new QAction(this);
   stopAction->setIcon(QIcon(":/images/stop.png"));
   stopAction->setText(tr("停止(F6)"));
   stopAction->setShortcut(QKeySequence(tr("F6")));
   connect(stopAction,SIGNAL(triggered(bool)),mediaObject,SLOT(stop()));
   //跳转到上一首动作
   skipBackwardAction = new QAction(this);
   skipBackwardAction->setIcon(QIcon(":/images/skipBackward.png"));
   skipBackwardAction->setText(tr("上一首(Ctrl+Left)"));
   skipBackwardAction->setShortcut(QKeySequence(tr("Ctrl+Left")));
   connect(skipBackwardAction,SIGNAL(triggered(bool)),this,SLOT(skipBackward()));
   //跳转到下一首动作
   skipForwardAction = new QAction(this);
   skipForwardAction->setIcon(QIcon(":/images/skipForward.png"));
   skipForwardAction->setText(tr("下一首(Ctrl+Right)"));
   skipForwardAction->setShortcut(QKeySequence(tr("Ctrl+Right")));
   connect(skipForwardAction,SIGNAL(triggered(bool)),this,SLOT(skipforward()));
   //打开文件动作
   QAction* openAction = new QAction(this);
   openAction->setIcon(QIcon(":/images/open.png"));
   openAction->setText(tr("播放文件(Ctrl+O)"));
   openAction->setShortcut(QKeySequence(tr("Ctrl+O")));
   connect(openAction,SIGNAL(triggered(bool)),this,SLOT(openFile()));
   //音量控制部件
   QSlider* volumeSlider = new QSlider(Qt::Horizontal,this);
   volumeSlider->setMaximum(100);
   volumeSlider->setValue(50);
   connect(volumeSlider,SIGNAL(valueChanged(int)),mediaObject,SLOT(setVolume(int)));
   volumeSlider->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
   //添加到工具栏
   toolBar->addAction(playAction);
   toolBar->addSeparator();
   toolBar->addAction(stopAction);
   toolBar->addSeparator();
   toolBar->addAction(skipBackwardAction);
   toolBar->addSeparator();
   toolBar->addAction(skipForwardAction);
   toolBar->addSeparator();
   toolBar->addWidget(volumeSlider);
   toolBar->addSeparator();
   toolBar->addAction(openAction);

   //创建主界面布局管理器
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(topLabel);
  mainLayout->addWidget(seekSlider);
  mainLayout->addWidget(widgetBar);
  mainLayout->addWidget(toolBar);
  this->setLayout(mainLayout);

//  mediaObject->setMedia(QUrl::fromLocalFile("../myPlayer/music.mp3"));
  connect(mediaObject,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
          this,SLOT(mediaStatusChange(QMediaPlayer::MediaStatus)));
  connect(mediaObject,SIGNAL(stateChanged(QMediaPlayer::State)),
          this,SLOT(stateChanged(QMediaPlayer::State)));
  connect(mediaObject,static_cast<void(QMediaPlayer::*)(QMediaPlayer::Error)>(&QMediaPlayer::error),
          this,&MyWidget::showError);
//创建播放列表
playlist = new MyPlaylist(this);
connect(playlist,SIGNAL(cellClicked(int,int)),
        this,SLOT(tableClicked(int)));
connect(playlist,SIGNAL(playlistClean()),this,
        SLOT(clearSources()));
//创建用来解析媒体信息的元信息解析器
metaInformationResolver = new QMediaPlayer(this);
connect(metaInformationResolver,SIGNAL(stateChanged(QMediaPlayer::State)),
        this,SLOT(metaStateChanged(QMediaPlayer::State)));
connect(metaInformationResolver,static_cast<void(QMediaPlayer::*)(QMediaPlayer::Error)>(&QMediaPlayer::error),
        this,&MyWidget::metaShowError);
connect(metaInformationResolver,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
        this,SLOT(metaStatusChanged(QMediaPlayer::MediaStatus)));
connect(mediaObject,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),  //这个信号和槽必须有，因为设置媒体对象如果不出错的话只有
        this,SLOT(aboutToFinish(QMediaPlayer::MediaStatus)));//媒体正在加载和加载完成两种信号，必须在这个槽函数中设置当前媒体对象和解析元数据并且插入到播放列表中
connect(mediaObject,SIGNAL(currentMediaChanged(QMediaContent)),this,SLOT(sourceChanged(QMediaContent)));

//初始化动作图标的状态
playAction->setEnabled(false);
stopAction->setEnabled(false);
skipBackwardAction->setEnabled(false);
skipForwardAction->setEnabled(false);
topLabel->setFocus();

//创建歌词部件
lrc = new MyLrc(this);

//创建系统托盘图标
trayIcon = new QSystemTrayIcon(QIcon(":/images/icon.png"),this);
trayIcon->setToolTip(tr("MyPlayer音乐播放器 - - - SWORD作品"));
//创建菜单
QMenu* menu = new QMenu;
QList<QAction*>actions;
actions<<playAction<<stopAction<<skipBackwardAction<<skipForwardAction;
menu->addActions(actions);
menu->addSeparator();
menu->addAction(PLAction);
menu->addAction(LRCAction);
menu->addSeparator();
menu->addAction(tr("退出"),qApp,SLOT(quit()));
trayIcon->setContextMenu(menu);
//托盘图标被激活后进行处理
connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
//显示托盘图标
trayIcon->show();

}


void MyWidget::updateTime(qint64 time)   //更新时间
{
    qint64 totalTimeValue = mediaObject->duration();            //获得歌曲时间，单位毫秒
    QTime totalTime(0,(totalTimeValue/60000)%60,(totalTimeValue/1000)%60);       //时分秒
    QTime currentTime(0,(time/60000)%60,(time/1000)%60);
    QString str = currentTime.toString("mm:ss")+"/"+totalTime.toString("mm:ss");
    timeLabel->setText(str);
    //获取当前时间对应的歌词
    if(!lrcMap.isEmpty())
    {
//获取当前时间在歌词中的前后两个时间点，即当前时间前后有歌词的两个时间点
qint64 previous = 0;
qint64 later = 0;
foreach(qint64 value,lrcMap.keys())
 {
 if(time >= value)
    previous = value;
 else
    {
    later = value;
     break;
    }
 }
//如果当前本来已经播放到歌词最后一行,将later设置为歌曲总时间
if(later ==0)
  later = totalTimeValue;
//获取当前时间所对应的歌词内容
QString currentLrc = lrcMap.value(previous);
//没有内容时
if(currentLrc.length()<2)
currentLrc = tr("MyPlayer音乐播放器 - - - SWORD作品");
//如果是新的一行歌词，那么重新显示歌词遮罩
if(currentLrc != lrc->text())
{
lrc->setText(currentLrc);
topLabel->setText(currentLrc);
qint64 intervalTime = later - previous;
lrc->startLrcMask(intervalTime);
qDebug()<<"新的一行歌词";
}
   }
 else            //如果没有歌词文件，则在顶部标签中显示歌曲标题
    topLabel->setText(QFileInfo(mediaObject->currentMedia().canonicalUrl().fileName()).baseName());
}

void MyWidget::updatePosition(qint64 position)   //更新滑块位置
{
seekSlider->setMaximum(mediaObject->duration()/1000);
seekSlider->setValue(position/1000);
//qDebug()<<mediaObject->duration();
}

//播放或者暂停
void MyWidget::setPaused()
{
    //如果先前处于播放状态，那么暂停播放，否则，开始播放
 if(mediaObject->state() == QMediaPlayer::PlayingState)
    mediaObject->pause();
 else
 {
//  qDebug()<<mediaObject->currentMedia().canonicalUrl();
    mediaObject->play();
 }
}


void MyWidget::skipBackward()
{
    lrc->stopLrcMask();
    int index = sources.indexOf(mediaObject->currentMedia());
    mediaObject->setMedia(sources.at(index - 1));
    mediaObject->play();
}

void MyWidget::skipforward()
{
    lrc->stopLrcMask();
    int index = sources.indexOf(mediaObject->currentMedia());
    mediaObject->setMedia(sources.at(index + 1));
    mediaObject->play();
}


void MyWidget::openFile()            //打开文件
{
//从系统音乐目录打开多个音乐文件
QStringList list = QFileDialog::getOpenFileNames(this,tr("打开音乐文件"),     //可以选中多个文件
/*"../myPlayer/sounds"*/QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

//QDesktopServices::storageLocation(QDesktopServices::MusicLocation)         //Qt5中已经不能使用
qDebug()<<list.size();
if(list.isEmpty())
{
//QMessageBox::warning(this,tr("打开文件错误"),mediaObject->errorString());
return;
}
//获取当前媒体源列表的大小
int index = sources.size();

//将打开的音乐文件添加到媒体元列表后
foreach(QString string,list)
{
    qDebug()<<string;
    QMediaContent source(QUrl::fromLocalFile(string));
    sources.append(source);
}
//如果媒体源列表不为空，则将新加入的第一个媒体源作为解析器的当前媒体源
if(!sources.isEmpty())
    metaInformationResolver->setMedia(sources.at(index));
qDebug()<<tr("打开的媒体源个数:")<<sources.size();
}


void MyWidget::metaStateChanged(QMediaPlayer::State state)
{
    //如果既不处于停止状态也不处于暂停状态，则直接返回
    if(state != QMediaPlayer::StoppedState && state != QMediaPlayer::PausedState)
    return;
//    if(state == QMediaPlayer::InvalidMedia)
//    return;
//   metaDataResolver();
}


void MyWidget::metaStatusChanged(QMediaPlayer::MediaStatus status)
{
//    if(status == QMediaPlayer::LoadingMedia)
//    QMessageBox::warning(this,tr("加载媒体..."),metaInformationResolver->errorString());
    if(status == QMediaPlayer::LoadedMedia)
        metaDataResolver();
//    QMessageBox::warning(this,tr("媒体加载完成"),metaInformationResolver->errorString());
    if(status == QMediaPlayer::StalledMedia)
    QMessageBox::warning(this,tr("媒体停滞..."),metaInformationResolver->errorString());
    if(status == QMediaPlayer::BufferingMedia)
    QMessageBox::warning(this,tr("媒体正在缓冲..."),metaInformationResolver->errorString());
    if(status == QMediaPlayer::BufferedMedia)
    QMessageBox::warning(this,tr("媒体缓冲完成"),metaInformationResolver->errorString());
    if(status == QMediaPlayer::InvalidMedia)
    QMessageBox::warning(this,tr("媒体无法播放"),metaInformationResolver->errorString());
}

void MyWidget::metaShowError(QMediaPlayer::Error error)
{
//错误状态，则从媒体源列表中除去新添加的媒体源
    if(error == QMediaPlayer::ResourceError||error == QMediaPlayer::AccessDeniedError||
       error == QMediaPlayer::FormatError||error == QMediaPlayer::ServiceMissingError)
    {
        QMessageBox::warning(this,tr("打开文件时出错"),metaInformationResolver->errorString());
        while(!sources.isEmpty()&&sources.takeLast()!=metaInformationResolver->currentMedia())
        {};
       return;
    }
//  metaDataResolver();
}


void MyWidget::setPlaylistShown()         //展示歌曲列表
{
if(playlist->isHidden())
{
    playlist->move(frameGeometry().topRight());
    playlist->show();
}
else
playlist->hide();
}


void MyWidget::setLrcShown()                //显示歌词
{
if(lrc->isHidden())
  lrc->show();
else
lrc->hide();
}

void MyWidget::mediaStatusChange(QMediaPlayer::MediaStatus state)
{
    if(state == QMediaPlayer::BufferingMedia)
        QMessageBox::warning(this,tr("正在缓冲"),mediaObject->errorString());
}

void MyWidget::stateChanged(QMediaPlayer::State state)
{
    switch(state)
    {
case QMediaPlayer::PlayingState:
        stopAction->setEnabled(true);
        playAction->setIcon(QIcon(":/images/pause.png"));
        playAction->setText(tr("暂停(F5)"));
        topLabel->setText(QFileInfo(mediaObject->currentMedia().canonicalUrl().fileName()).baseName());
        qDebug()<<tr("播放状态");
        resolveLrc(mediaObject->currentMedia().canonicalUrl().fileName());
        break;
case QMediaPlayer::StoppedState:
        stopAction->setEnabled(false);
        playAction->setIcon(QIcon(":/images/play.png"));
        playAction->setText(tr("播放(F5)"));
        topLabel->setText(tr("<a href = \" http://www.yafeilinux.com \">欢迎使用 SWORD MUSIC PLAYER</a>"));
        timeLabel->setText(tr("00:00 / 00:00"));
        qDebug()<<tr("停止状态");
        lrc->stopLrcMask();
        lrc->setText(tr("MyPlayer音乐播放器 - - - SWORD作品"));
        break;
case QMediaPlayer::PausedState:
        stopAction->setEnabled(true);
        playAction->setIcon(QIcon(":/images/play.png"));
        playAction->setText(tr("播放(F5)"));
        topLabel->setText(QFileInfo(mediaObject->currentMedia().canonicalUrl().fileName()).baseName()+tr(" 已暂停!"));
        if(!lrcMap.isEmpty())
        {
         qDebug()<<tr("暂停状态");
         lrc->stopLrcMask();
         lrc->setText(topLabel->text());
        }
        break;
    default:
        ;
    }
}

void MyWidget::showError(QMediaPlayer::Error error)
{
  switch(error)
  {
  case QMediaPlayer::ResourceError:
  QMessageBox::warning(this,tr("资源解析"),mediaObject->errorString());
  break;
  case QMediaPlayer::FormatError:
    QMessageBox::warning(this,tr("媒体格式"),mediaObject->errorString());
    break;
  case QMediaPlayer::NetworkError:
    QMessageBox::warning(this,tr("网络"),mediaObject->errorString());
    break;
  case QMediaPlayer::AccessDeniedError:
    QMessageBox::warning(this,tr("播放权限"),mediaObject->errorString());
    break;
  case QMediaPlayer::ServiceMissingError:
    QMessageBox::warning(this,tr("播放服务"),mediaObject->errorString());
    break;
  default:
      ;
  }
}

void MyWidget::metaDataResolver()          //元对象解析数据
{
    //获取标题，如果为空，则使用文件名
    QString title = metaInformationResolver->metaData("Title").toString();
    if(title == "")
        title = QFileInfo(metaInformationResolver->currentMedia().canonicalUrl().fileName()).baseName();
    QTableWidgetItem* titleItem = new QTableWidgetItem(title);
    //设置数据项不可编辑
    titleItem->setFlags(titleItem->flags()^Qt::ItemIsEditable);
    //获取艺术家信息
    QString author = metaInformationResolver->metaData("Author").toString();
    QTableWidgetItem* artistItem = new QTableWidgetItem(author);
    artistItem->setFlags(artistItem->flags()^Qt::ItemIsEditable);
    //获取总时间信息
    qint64 totalTime = metaInformationResolver->duration();
    QTime time(0,(totalTime/60000)%60,(totalTime/1000)%60);
    QTableWidgetItem* timeItem = new QTableWidgetItem(time.toString("mm:ss"));

    //插入到播放列表
    int currentRow = playlist->rowCount();
    playlist->insertRow(currentRow);
    playlist->setItem(currentRow,0,titleItem);
    playlist->setItem(currentRow,1,artistItem);
    playlist->setItem(currentRow,2,timeItem);
    int index = sources.indexOf(metaInformationResolver->currentMedia()) + 1;
    if(sources.size() > index)        //如果添加的媒体源还没有解析完，那么继续解析下一个媒体源
        metaInformationResolver->setMedia(sources.at(index));
    else      //如果所有的媒体源都已经解析完成
    {
        if(playlist->selectedItems().isEmpty())
        {
            //如果现在没有播放歌曲则设置第一个媒体源为媒体对象的当前媒体源
            //（因为可能正在播放歌曲时清空了播放列表，然后又添加了新的列表）
            if(mediaObject->state()!= QMediaPlayer::PlayingState&&mediaObject->state()!= QMediaPlayer::PausedState)
                mediaObject->setMedia(sources.at(0));
            else           //如果正在播放歌曲，则选中播放列表的第一个曲目，并更改图标状态
            {
                playlist->selectRow(0);
                changeActionState();
            }
        }
        else       //如果播放列表有选中的行，则直接更新图标状态
        {
            changeActionState();
        }
    }
}

//根据媒体源列表内容和当前媒体源的位置来改变主界面图标的状态
void MyWidget::changeActionState()
{
  if(sources.count() == 0)          //如果媒体源列表为空
   {
    //如果没有在播放歌曲，则播放和停止按钮都不可用（因为可能歌曲正在播放时清除了播放列表）
     if(mediaObject->state() != QMediaPlayer::PlayingState && mediaObject->state() != QMediaPlayer::PausedState)
      {
      playAction->setEnabled(false);
      stopAction->setEnabled(false);
      }
     skipBackwardAction->setEnabled(false);
     skipForwardAction->setEnabled(false);
    }
  else                //如果媒体列表不为空
    {
    playAction->setEnabled(true);
    stopAction->setEnabled(true);

    if(sources.count() == 1)   //如果媒体源列表只有一行
    {
        skipBackwardAction->setEnabled(false);
        skipForwardAction->setEnabled(false);
    }
    else                    //如果媒体源列表有多行
    {
       skipBackwardAction->setEnabled(true);
        skipForwardAction->setEnabled(true);
        int index = playlist->currentRow();            //返回当前项的行下标
        if(index == 0)                            //如果播放列表当前选中的是第一行
        skipBackwardAction->setEnabled(false);
        if(index + 1 == sources.count())
        skipForwardAction->setEnabled(false);       //如果播放列表当前选中的是最后一行
    }
   }
}


//当媒体源改变时，在播放列表中选中相应的行并更新图标的状态
void MyWidget::sourceChanged(const QMediaContent &source)      //参数source指向的是改变后的QMediaContent
{
    int index = sources.indexOf(source);
    playlist->selectRow(index);
    changeActionState();
}


//处理播放列表每一首歌播放完毕时的问题
void MyWidget::aboutToFinish(QMediaPlayer::MediaStatus status)
{
    if(status == QMediaPlayer::EndOfMedia)
    {
   int index = sources.indexOf(mediaObject->currentMedia());
  if(index == sources.size()-1)
       index = 0;
      else
      index += 1;
  mediaObject->setMedia(sources.at(index));
  mediaObject->play();
  lrc->stopLrcMask();             //**************************
  resolveLrc(sources.at(index).canonicalUrl().fileName());       //**********************
    }
}


//单击播放列表
void MyWidget::tableClicked(int row)
{
//首先获取媒体对象当前的状态，然后停止播放
bool wasPlaying = (mediaObject->state() == QMediaPlayer::PlayingState);
mediaObject->stop();
//***********************************
//如果单击的播放列表中的行号大于媒体源列表的大小,则直接返回
if(row >= sources.size())
return;
    //设置单击的行对应的媒体源为媒体对象的当前媒体源
mediaObject->setMedia(sources.at(row));
    //如果以前媒体对象处于播放状态，那么开始播放选中的曲目
    if(wasPlaying)
        mediaObject->play();
}

//清空媒体源列表，它与播放列表的playListClean()信号相关联
void MyWidget::clearSources()
{
    sources.clear();
    changeActionState();                   //更改动作图标
}

//解析LRC歌词
void MyWidget::resolveLrc(const QString &sourceFileName)
{
    //先清空以前的内容
    lrcMap.clear();

    //获取LRC歌词的文件名
    if(sourceFileName.isEmpty())
      return;
    QString fileName = sourceFileName;
    QString lrcFileName = fileName.remove(fileName.right(3))+"lrc";
    //打开歌词文件
    QFile file("C:/Users/蒋健/Music/" + lrcFileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        lrc->setText(QFileInfo(mediaObject->currentMedia().canonicalUrl().fileName()).baseName()+tr(" - - - 未找到歌词文件!"));
    return;
    }
    //设置字符串编码

//    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());                //*********************
    //获取全部歌词信息
    QString allText = QString(file.readAll());
    file.close();             //关闭歌词文件
    //将歌词按行分解为歌词列表
    QStringList lines = allText.split("\n");
  //使用正则表达式将时间标签和歌词内容分离
    QRegExp rx("\\[\\d{2}:\\d{2}\\.\\d{2}\\]");             ///////////////中括号匹配需要转义??????????????
    foreach(QString line,lines)
    {
        QString temp=line;
        temp.replace(rx,"");               //用空字符串替换匹配正则的字符串，即清除时间标签
        //然后依次获取当前行中所有时间标签，并分别与歌词文本存入QMap中
        int pos = rx.indexIn(line,0);
        while(pos != -1)
        {
            QString cap=rx.cap(0);
           //将时间标签转换为时间数值，以毫秒为单位
            QRegExp regexp("\\d{2}(?=:)");
            regexp.indexIn(cap);
            int minute = regexp.cap(0).toInt();
            regexp.setPattern("\\d{2}(?=\\.)");
            regexp.indexIn(cap);
            int second = regexp.cap(0).toInt();
            regexp.setPattern("\\d{2}(?=\\])");
            regexp.indexIn(cap);
            int millisecond = regexp.cap(0).toInt();
            qint64 totalTime = minute*60000 + second*1000 + millisecond*10;
            //插入到lrcMap中
            lrcMap.insert(totalTime,temp);             //QMap会自动将时间排序
            pos += rx.matchedLength();
            pos = rx.indexIn(line,pos);               //一行中可能有多个时间标签
        }
    }
    if(lrcMap.isEmpty())                           //如果lrcMap为空
    {
        lrc->setText(QFileInfo(mediaObject->currentMedia().canonicalUrl().fileName()).baseName() + tr(" - - - 歌词文件内容错误!"));
        return;
    }
}


//系统托盘图标被激活
void MyWidget::trayIconActivated(QSystemTrayIcon::ActivationReason activationReason)
{
    //如果单击了系统托盘图标，则显示应用程序界面
  if(activationReason == QSystemTrayIcon::Trigger)
    this->show();
}

//关闭事件处理函数
void MyWidget::closeEvent(QCloseEvent *event)
{
    if(isVisible())
        this->hide();
    trayIcon->showMessage(tr("MyPlayer音乐播放器"),tr("点我重新显示主界面"));
    event->ignore();
}












