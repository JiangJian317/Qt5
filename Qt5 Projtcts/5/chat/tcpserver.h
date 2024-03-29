#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QDialog>
#include <QTime>

class QFile;
class QTcpServer;
class QTcpSocket;

namespace Ui {
class TcpServer;
}

class TcpServer : public QDialog
{
    Q_OBJECT

public:
    explicit TcpServer(QWidget *parent = 0);
    ~TcpServer();
    void initServer();
    void refused();
protected:
    void closeEvent(QCloseEvent *);
private slots:
    void sendMessage();
    void updateClientProgress(qint64 numBytes);
    void on_serverOpenBtn_clicked();

    void on_serverSendBtn_clicked();

    void on_serverCloseBtn_clicked();

signals:
void sendFileName(QString fileName);
private:
    Ui::TcpServer *ui;
    qint16 tcpPort;       //端口号
    QTcpServer* tcpServer;
    QString fileName;
    QString theFileName;
    QFile* localFile;

    qint64 TotalBytes;     //发送数据的总大小
    qint64 bytesWritten;     //已经发送的数据大小
    qint64 bytesToWrite;     //剩余数据大小
    qint64 payloadSize;      //每次发送数据的大小
    QByteArray outBlock;

    QTcpSocket* clientConnection;
    QTime time;
};

#endif // TCPSERVER_H
