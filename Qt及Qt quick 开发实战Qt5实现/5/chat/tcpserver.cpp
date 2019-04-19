#include "tcpserver.h"
#include "ui_tcpserver.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

TcpServer::TcpServer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    setFixedSize(350,180);
    tcpPort = 6666;
    tcpServer = new QTcpServer(this);
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(sendMessage()));   //有客户端连接时调用sendMessage()槽
    initServer();
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::initServer()
{
    payloadSize =  64*1024;          //64KB
    TotalBytes = 0;
    bytesToWrite = 0;
    bytesWritten = 0;
    ui->serverStatusLabel->setText("请选择要发送的文件!");
    ui->progressBar->reset();
    ui->serverOpenBtn->setEnabled(true);
    ui->serverSendBtn->setEnabled(false);
    tcpServer->close();
}


//开始发送数据，实质只是发送了数据报的头结构
void TcpServer::sendMessage()
{
    ui->serverSendBtn->setEnabled(false);
    clientConnection = tcpServer->nextPendingConnection();                 //获取连接的套接字
    connect(clientConnection,SIGNAL(bytesWritten(qint64)),this,SLOT(updateClientProgress(qint64)));
    ui->serverStatusLabel->setText(tr("开始传送文件 %1 !").arg(theFileName));

    localFile = new QFile(fileName);
    if(!localFile->open(QFile::ReadOnly))            //************************************
    {
    QMessageBox::warning(this,"应用程序",tr("无法读取文件 %1:\n%2").arg(fileName).arg(localFile->errorString()));
      return;
    }
    TotalBytes = localFile->size();           //文件实际内容大小
    QDataStream sendOut(&outBlock,QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_6);
    time.start();                    //将此时间设置为当前时间,开始计时
    QString currentFile = fileName.right(fileName.size() - fileName.lastIndexOf('/')-1);

    sendOut<<qint64(0)<<qint64(0)<<currentFile;   //数据头信息
    TotalBytes += outBlock.size();
    sendOut.device()->seek(0);
    sendOut<<TotalBytes<<qint64((outBlock.size() - sizeof(qint64)*2));
    qDebug()<<outBlock.size();
    bytesToWrite = TotalBytes - clientConnection->write(outBlock);      //将outBlock中的数据写入到clientConnection中，
                                                          //outBlock的大小并不会改变,会调用updateClientProgress(qint64)槽函数
    qDebug()<<outBlock.size();
    outBlock.resize(0);
    qDebug()<<TotalBytes<<"      "<<bytesToWrite;
    qDebug()<<"服务端开始发送数据头结构";
}


void TcpServer::updateClientProgress(qint64 numBytes)     //正式开始发送文件内容
{
 qDebug()<<"服务端开始发送文件内容";
qApp->processEvents();                       //避免界面冻结
bytesWritten += (int)numBytes;
if(bytesToWrite > 0)
 {
 outBlock = localFile->read(qMin(bytesToWrite,payloadSize));           //开始读取实际文件内容
 bytesToWrite -= (int)clientConnection->write(outBlock);
 outBlock.resize(0);
 }
else
localFile->close();

ui->progressBar->setMaximum(TotalBytes);
ui->progressBar->setValue(bytesWritten);

float useTime = time.elapsed();
double speed = bytesWritten/useTime;
ui->serverStatusLabel->setText(tr("已发送 %1MB(%2MB/s)\n共%3MB已用时:%4秒\n估计剩余时间:%5秒")
                               .arg(bytesWritten/(1024*1024))
                               .arg(speed*1000/(1024*1024),0,'f',2)      //保留两位小数
                               .arg(TotalBytes/(1024*1024))
                               .arg(useTime/1000,0,'f',0)                    //保留0位小数
                               .arg(TotalBytes/speed/1000 - useTime/1000,0,'f',0)    //保留0位小数
                                    );
if(bytesWritten == TotalBytes)
 {
 localFile->close();
 tcpServer->close();
 ui->serverStatusLabel->setText(tr("传送文件 %1 成功").arg(theFileName));
 }
}


//打开按钮
void TcpServer::on_serverOpenBtn_clicked()
{
fileName = QFileDialog::getOpenFileName(this);
if(!fileName.isEmpty())
 {
 theFileName = fileName.right(fileName.size() - fileName.lastIndexOf('/') - 1);
 ui->serverStatusLabel->setText(tr("要传送的文件为:%1").arg(theFileName));
 ui->serverSendBtn->setEnabled(true);
 ui->serverOpenBtn->setEnabled(false);
 }
}


//发送按钮
void TcpServer::on_serverSendBtn_clicked()
{
if(!tcpServer->listen(QHostAddress::AnyIPv4,tcpPort))      //开始监听(包括所有IPV4地址)
 {
qDebug()<<tcpServer->errorString()<<"服务端发送失败";
this->close();
return;
 }
ui->serverStatusLabel->setText(tr("等待对方接收... ..."));
emit sendFileName(theFileName);              //发射一个发送文件信号
}


//关闭按钮
void TcpServer::on_serverCloseBtn_clicked()
{
if(tcpServer->isListening())
 {
 tcpServer->close();
   if(localFile->isOpen())
     localFile->close();
   clientConnection->abort();
 }
close();
}


//如果接收端拒绝接收该文件
void TcpServer::refused()
{
    tcpServer->close();
    ui->serverStatusLabel->setText(tr("对方拒绝接收!!!"));
}


void TcpServer::closeEvent(QCloseEvent *)
{
    on_serverCloseBtn_clicked();
}












