#include "widget.h"
#include "ui_widget.h"
#include <QUdpSocket>
#include <QHostInfo>
#include <QMessageBox>
#include <QDateTime>
#include <QNetworkInterface>
#include <QProcess>
#include <QScrollBar>
#include "tcpserver.h"
#include "tcpclient.h"
#include <QFileDialog>
#include <QDebug>
#include <QColorDialog>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/images/icon.png"));
    udpSocket = new QUdpSocket(this);
    port = 45454;
    udpSocket->bind(port,QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint);
    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(processPendingDatagrams()));
    sendMessage(NewParticipant);

    server = new TcpServer(this);
    connect(server,SIGNAL(sendFileName(QString)),this,SLOT(getFileName(QString)));
    connect(ui->messageTextEdit,SIGNAL(currentCharFormatChanged(QTextCharFormat)),this,SLOT(currentFormatChanged(QTextCharFormat)));
}

Widget::~Widget()
{
    delete ui;
}


void Widget::sendMessage(MessageType type, QString serverAddress)   //此函数最后会进行一个UDP广播
{
QByteArray data;
QDataStream out(&data,QIODevice::WriteOnly);
QString localHostName = QHostInfo::localHostName();               //返回主机名
QString address = getIP();
out<<type<<getUserName()<<localHostName;
switch(type)
 {
case Message:
 if(ui->messageTextEdit->toPlainText() == "")
  {
  QMessageBox::warning(0,"警告","发送内容不能为空",QMessageBox::Ok);
     return;
  }
 out<<address<<getMessage();
 ui->messageBrowser->verticalScrollBar()->setValue(ui->messageBrowser->verticalScrollBar()->maximum());   //总是显示最新的内容
    break;
case NewParticipant:
 out<<address;
    break;
case ParticipantLeft:
   break;
case FileName:
{
    int row = ui->userTableWidget->currentRow();
    QString clientAddress = ui->userTableWidget->item(row,2)->text();
    out<<address<<clientAddress<<fileName;
 break;
}
case Refuse:
    out<<serverAddress;
    break;
 }
 udpSocket->writeDatagram(data,data.length(),QHostAddress::Broadcast,port);    //将数据报发送到主机的指定端口
 qDebug()<<"sendMessage()函数被调用";
}


// 接收UDP信息
void Widget::processPendingDatagrams()             //该函数会接收UDP广播发送来的数据
{
    while(udpSocket->hasPendingDatagrams())
    {
    QByteArray datagram;
    datagram.resize(udpSocket->pendingDatagramSize());                  //将数据报的大小设置为接收的数据报的大小
    udpSocket->readDatagram(datagram.data(),datagram.size());           //将大小为datagram.size()的数据写入到datagram中
    QDataStream in(&datagram,QIODevice::ReadOnly);
    int messageType;
    in>>messageType;
    QString userName,localHostName,ipAddress,message;
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    switch(messageType)
     {
    case Message:
      in>>userName>>localHostName>>ipAddress>>message;
      ui->messageBrowser->setTextColor(Qt::blue);
      ui->messageBrowser->setCurrentFont(QFont("Times New Roman",12));
      ui->messageBrowser->append("[" + userName + "]" + time);
      ui->messageBrowser->append(message);          //由于message的格式是HTML，上面的字体设置对其无效
      break;
    case NewParticipant:
      in>>userName>>localHostName>>ipAddress;
      newParticipant(userName,localHostName,ipAddress);
        break;
    case ParticipantLeft:
      in>>userName>>localHostName;
      participantLeft(userName,localHostName,time);
        break;
    case FileName:
    {
      qDebug()<<"processPendingDatagrams()函数被调用  FileName";
      in>>userName>>localHostName>>ipAddress;
      QString clientAddress,fileName;
      in>>clientAddress>>fileName;
      hasPendingFile(userName,ipAddress,clientAddress,fileName);
      break;
    }
    case Refuse:
    {
      in>>userName>>localHostName;
      QString serverAddress;
      in>>serverAddress;
      QString ipAddress = getIP();
      if(ipAddress == serverAddress)
          server->refused();
      break;
    }
     }
    }
}


//新用户加入
void Widget::newParticipant(QString userName, QString localHostName, QString ipAddress)
{
    bool isEmpty = ui->userTableWidget->findItems(localHostName,Qt::MatchExactly).isEmpty();
  if(isEmpty)          //即是新加入的用户
   {
    QTableWidgetItem* user = new QTableWidgetItem(userName);
    QTableWidgetItem* host = new QTableWidgetItem(localHostName);
    QTableWidgetItem* ip = new QTableWidgetItem(ipAddress);

    ui->userTableWidget->insertRow(0);             //在ui文件中已经设置了3列
    ui->userTableWidget->setItem(0,0,user);
    ui->userTableWidget->setItem(0,1,host);
    ui->userTableWidget->setItem(0,2,ip);

    ui->messageBrowser->setTextColor(Qt::gray);
    ui->messageBrowser->setCurrentFont(QFont("Times New Roman",10));
    ui->messageBrowser->append(tr("%1 在线!").arg(userName));

    ui->userNumLabel->setText(tr("在线人数:%1").arg(ui->userTableWidget->rowCount()));
    sendMessage(NewParticipant);
   }
}


//用户离开
void Widget::participantLeft(QString userName, QString localHostName, QString time)
{
    int rowNum = ui->userTableWidget->findItems(localHostName,Qt::MatchExactly).first()->row();   //返回localHostName所在的行
    ui->userTableWidget->removeRow(rowNum);
    ui->messageBrowser->setTextColor(Qt::gray);
    ui->messageBrowser->setCurrentFont(QFont("Times New Roman",10));
    ui->messageBrowser->append(tr("%1 于 %2 离开!").arg(userName).arg(time));
    ui->userNumLabel->setText(tr("在线人数: %1").arg(ui->userTableWidget->rowCount()));
}


//获取ip地址
QString Widget::getIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();        //返回当前运行程序主机的网络接口所有IP地址
    foreach(QHostAddress address,list)
     {
     if(address.protocol() == QAbstractSocket::IPv4Protocol)
         return address.toString();
     }
 return 0;
}


//获取用户名
QString Widget::getUserName()
{
    QStringList envVariables;
    envVariables<<"USERNAME.*"<<"USER.*"<<"USERDOMAIN.*"<<"HOSTNAME.*"<<"DOMAINNAME.*";
    QStringList environment = QProcess::systemEnvironment();            //以key = value对的列表的形式返回调用进程的环境
    foreach (QString string,envVariables)
    {
        int index = environment.indexOf(QRegExp(string));
        if(index != -1)                       //匹配成功
         {
         QStringList stringList = environment.at(index).split('=');
         if(stringList.size() == 2)
           {
            return stringList.at(1);
            break;
           }
         }
    }
return "unknown";
}


//获得要发送的信息
QString Widget::getMessage()
{
    QString msg = ui->messageTextEdit->toHtml();
    ui->messageTextEdit->clear();
    ui->messageTextEdit->setFocus();
    return msg;
}


void Widget::on_sendButton_clicked()
{
   sendMessage(Message);
}


void Widget::getFileName(QString name)
{
  this->fileName = name;
  sendMessage(FileName);        //发送FileName类型的UDP广播
}


//传送文件
void Widget::on_sendToolBtn_clicked()
{
if(ui->userTableWidget->selectedItems().isEmpty())    //未选择用户
  {
    QMessageBox::warning(0,"选择用户",tr("请先从用户列表选择要传送的用户!"),QMessageBox::Ok);
    return;
  }
server->show();                                     //弹出发送端界面
server->initServer();
}


void Widget::hasPendingFile(QString userName, QString serverAddress, QString clientAddress, QString fileName)
{
    QString ipAddress = getIP();
    if(ipAddress == clientAddress)
    {
     int btn = QMessageBox::information(this,"接收文件",tr("来自%1(%2)的文件:%3,是否接收?")
                                        .arg(userName).arg(serverAddress).arg(fileName),
                                        QMessageBox::Yes,QMessageBox::No);
    if(btn == QMessageBox::Yes) 
       {
       QString name = QFileDialog::getSaveFileName(0,tr("保存文件"),fileName);
        if(!name.isEmpty())
          {
          TcpClient* client = new TcpClient(this);
          client->setFileName(name);
          client->setHostAddress(QHostAddress(serverAddress));
          client->show();
          }
       }
     else
        sendMessage(Refuse,serverAddress);
    }
}

//选择字体
void Widget::on_fontComboBox_currentFontChanged(const QFont &f)
{
ui->messageTextEdit->setCurrentFont(f);
ui->messageTextEdit->setFocus();
}


//字体大小
void Widget::on_sizeComboBox_currentIndexChanged(const QString &size)
{
   ui->messageTextEdit->setFontPointSize(size.toDouble());
   ui->messageTextEdit->setFocus();
}

//加粗
void Widget::on_boldToolBtn_clicked(bool checked)
{
if(checked)
  ui->messageTextEdit->setFontWeight(QFont::Bold);
else
  ui->messageTextEdit->setFontWeight(QFont::Normal);
ui->messageTextEdit->setFocus();
}

//倾斜
void Widget::on_italicToolBtn_clicked(bool checked)
{
ui->messageTextEdit->setFontItalic(checked);
ui->messageTextEdit->setFocus();
}

//下划线
void Widget::on_underlineToolBtn_clicked(bool checked)
{
ui->messageTextEdit->setFontUnderline(checked);
ui->messageTextEdit->setFocus();
}

//更改颜色
void Widget::on_colorToolBtn_clicked()
{
color = QColorDialog::getColor(color,this);
  if(color.isValid())
    {
      ui->messageTextEdit->setTextColor(color);
      ui->messageTextEdit->setFocus();
    }
}


void Widget::currentFormatChanged(const QTextCharFormat &format)
{
    ui->fontComboBox->setCurrentFont(format.font());
    //如果字体大小出错(因为设定最小的字体是9),使用12
    if(format.fontPointSize()<9)
       ui->sizeComboBox->setCurrentIndex(3);
    else
       ui->sizeComboBox->setCurrentIndex(ui->sizeComboBox         //根据文本内容改变ComboBox
                                         ->findText(QString::number(format.fontPointSize())));
    ui->boldToolBtn->setChecked(format.font().bold());
    ui->italicToolBtn->setChecked(format.font().italic());
    ui->underlineToolBtn->setChecked(format.font().underline());
    color = format.foreground().color();
}



void Widget::on_saveToolBtn_clicked()
{
if(ui->messageBrowser->document()->isEmpty())
    QMessageBox::warning(0,"警告","聊天记录为空，无法保存",QMessageBox::Ok);
else
{
 QString fileName = QFileDialog::getSaveFileName(this,"保存聊天记录","聊天记录",tr("文本(*.txt);;All File(*.*)"));
 if(!fileName.isEmpty())
    saveFile(fileName);
}
}


bool Widget::saveFile(const QString &fileName)
{
QFile file(fileName);
if(!file.open(QFile::WriteOnly|QFile::Text))
  {
  QMessageBox::warning(this,"保存文件",tr("无法保存文件 %1:\n%2").arg(fileName).arg(file.errorString()));
    return false;
  }
QTextStream out(&file);
out<<ui->messageBrowser->toPlainText();
return true;
}

//清空按钮
void Widget::on_clearToolBtn_clicked()
{
  ui->messageBrowser->clear();
}

//退出按钮
void Widget::on_exitButton_clicked()
{
this->close();
}


//关闭事件
void Widget::closeEvent(QCloseEvent *event)
{
    sendMessage(ParticipantLeft);
    QWidget::closeEvent(event);
}



