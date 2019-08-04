#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTextCharFormat>

class QUdpSocket;
class TcpServer;

namespace Ui {
class Widget;
}

//枚举变量标志信息类型，分别为消息、新用户加入、用户退出、文件名、拒绝接收文件，表示不同的广播类型
enum MessageType{Message,NewParticipant,ParticipantLeft,FileName,Refuse};

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
protected:
    void newParticipant(QString userName,QString localHostName,QString ipAddress);
    void participantLeft(QString userName,QString localHostName,QString time);
    void sendMessage(MessageType type,QString serverAddress = "");
    QString getIP();
    QString getUserName();
    QString getMessage();

    void hasPendingFile(QString userName,QString serverAddress,QString clientAddress,QString fileName);

    bool saveFile(const QString& fileName);
private slots:
    void processPendingDatagrams();
    void on_sendButton_clicked();

    void getFileName(QString);       //获取服务器类sendFileName()信号发送过来的文件名
    void on_sendToolBtn_clicked();

    void on_fontComboBox_currentFontChanged(const QFont &f);

    void on_sizeComboBox_currentIndexChanged(const QString &size);

    void on_boldToolBtn_clicked(bool checked);

    void on_italicToolBtn_clicked(bool checked);

    void on_underlineToolBtn_clicked(bool checked);

    void on_colorToolBtn_clicked();

    void currentFormatChanged(const QTextCharFormat& format);
    void on_saveToolBtn_clicked();

    void on_clearToolBtn_clicked();

    void on_exitButton_clicked();

    void closeEvent(QCloseEvent *event);
private:
    Ui::Widget *ui;
    QUdpSocket* udpSocket;
    qint16 port;

    QString fileName;
    TcpServer* server;

    QColor color;
};

#endif // WIDGET_H
