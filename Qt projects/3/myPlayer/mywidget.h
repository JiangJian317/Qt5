#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include <QSlider>
#include <QSystemTrayIcon>

class QLabel;
class MyPlaylist;
class MyLrc;

namespace Ui {
class MyWidget;
}

class MyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MyWidget(QWidget *parent = 0);
    ~MyWidget();
    void metaDataResolver();
protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void updateTime(qint64 time);      //更新时间
    void setPaused();                  //暂停
    void skipBackward();               //上一曲
    void skipforward();                //下一曲
    void openFile();                   //打开文件
    void setPlaylistShown();           //展示歌曲列表
    void setLrcShown();                //显示歌词
    void updatePosition(qint64 position);    //更新滑块位置
    void mediaStatusChange(QMediaPlayer::MediaStatus state);  //媒体状态改变
    void showError(QMediaPlayer::Error error);    //播放器错误
    void stateChanged(QMediaPlayer::State state);   //播放器状态
    void sourceChanged(const QMediaContent& source);
    void aboutToFinish(QMediaPlayer::MediaStatus status);       //处理播放列表最后一首歌
    void metaStatusChanged(QMediaPlayer::MediaStatus status);
    void metaStateChanged(QMediaPlayer::State state);
    void metaShowError(QMediaPlayer::Error error);
    void tableClicked(int row);
    void clearSources();
    void trayIconActivated(QSystemTrayIcon::ActivationReason activationReason);     //参数表示系统托盘被激活的原因
private:
    Ui::MyWidget *ui;
    void initPlayer();                //初始化播放器
    QMediaPlayer* mediaObject;
    QAction* playAction;               //播放按钮
    QAction* stopAction;              //暂停按钮
    QAction* skipBackwardAction;       //上一首
    QAction* skipForwardAction;       //下一首
    QLabel* topLabel;                 //显示播放的歌曲
    QLabel* timeLabel;                 //时间标签
    QSlider* seekSlider;               //歌曲滑动条
    MyPlaylist* playlist;
    QMediaPlayer* metaInformationResolver;
    QList<QMediaContent> sources;
    void changeActionState();
    MyLrc* lrc;
    QMap<qint64,QString> lrcMap;
    void resolveLrc(const QString& sourceFileName);
    QSystemTrayIcon* trayIcon;
};

#endif // MYWIDGET_H
