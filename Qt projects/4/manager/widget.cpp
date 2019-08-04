#include "widget.h"
#include "ui_widget.h"
#include <QtSql>
#include <QSplitter>
#include <QMessageBox>
#include <QDebug>
#include "pieview.h"
#include <QStandardItem>
#include <QTableView>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    db = QSqlDatabase::addDatabase("QMYSQL","Manager");
    db.setHostName("localhost");
    db.setDatabaseName("m_data");
    db.setPort(3306);
    db.setUserName("root");
    db.setPassword("317727jiangjian");
    db.open();

    setFixedSize(750,500);
    ui->stackedWidget->setCurrentIndex(0);

    QSqlQueryModel* typeModel = new QSqlQueryModel(this);

    typeModel->setQuery("select name from type",db);

    ui->sellTypeComboBox->setModel(typeModel);          //为Combox设置Model


    QSplitter* splitter = new QSplitter(ui->managePage);
    splitter->resize(700,360);
    splitter->move(0,50);
    splitter->addWidget(ui->toolBox);
    splitter->addWidget(ui->dailyList);
    splitter->setStretchFactor(0,1);
    splitter->setStretchFactor(1,1);

    on_sellCancelBtn_clicked();                 //初始化部件状态
    showDailyList();

    ui->typeComboBox->setModel(typeModel);
    createChartModelView();

}


Widget::~Widget()
{
    delete ui;
}

bool Widget::createConnection()
{
    QSqlQuery query(db);
    if(!db.open())
    {
        QMessageBox::critical(0,"Cannot open database","Unable to establish a database connection.","取消");
        qDebug()<<query.lastError().databaseText();
        return false;
    }
 //创建分类表
   query.exec("create table type(id varchar(5) primary key,name varchar(5))");
   query.exec("insert into type values('0','请选择类型'),"
                                      "('01','电视'),"
                                      "('02','空调')");
 //创建品牌表
 query.exec("create table brand(id varchar(5) primary key,name varchar(5),"
            "type varchar(5),price int,sum int,sell int,last int)");
 query.exec("insert into brand values('01','海信','电视',3699,50,10,40),"
                                    "('02','创维','电视',3499,20,5,15),"
                                    "('03','海尔','电视',4199,80,40,40),"
                                    "('04','王牌','电视',3999,40,10,30),"
                                    "('05','海尔','空调',2899,60,10,50),"
                                    "('06','格力','空调',2799,70,20,50)");

 //创建密码表
 query.exec("create table password(pwd varchar(20) primary key)");
 query.exec("insert into password values('123456')");


return true;
}

// 出售商品的商品类型改变时
void Widget::on_sellTypeComboBox_currentIndexChanged(const QString &type)
{
  if(type == "请选择类型")
    {
    //进行其他部件的状态设置
    on_sellCancelBtn_clicked();
    }
  else
    {
      ui->sellBrandComboBox->setEnabled(true);
      QSqlQueryModel* model = new QSqlQueryModel(this);
      model->setQuery(QString("select name from brand where type = '%1'").arg(type),db);
      ui->sellBrandComboBox->setModel(model);
      ui->sellCancelBtn->setEnabled(true);
    }
  qDebug()<<"出售商品的商品类型改变";
}


// 出售商品的品牌改变时
void Widget::on_sellBrandComboBox_currentIndexChanged(const QString &brand)
{
ui->sellNumSpinBox->setValue(0);             //卖出的数量
ui->sellNumSpinBox->setEnabled(false);
ui->sellSumLineEdit->clear();
ui->sellSumLineEdit->setEnabled(false);
ui->sellOkBtn->setEnabled(false);


QSqlQuery query(db);
query.exec(QString("select price from brand where name = '%1' and type = '%2'").arg(brand).arg(ui->sellTypeComboBox->currentText()));
query.next();
ui->sellPriceLineEdit->setEnabled(true);
ui->sellPriceLineEdit->setReadOnly(true);         //设置价格只能读取
ui->sellPriceLineEdit->setText(query.value(0).toString());

query.exec(QString("select last from brand where name = '%1' and type = '%2'").arg(brand).arg(ui->sellTypeComboBox->currentText()));
query.next();
int num = query.value(0).toInt();
if(num == 0)
QMessageBox::information(this,tr("提示"),tr("该商品已售完!"),QMessageBox::Ok);
else
{
    ui->sellNumSpinBox->setEnabled(true);
    ui->sellNumSpinBox->setMaximum(num);
    ui->sellLastNumLabel->setText(tr("剩余数量:%1").arg(num));
    ui->sellLastNumLabel->setVisible(true);
}
qDebug()<<"出售商品的品牌改变";
}


// 出售商品数量改变时
void Widget::on_sellNumSpinBox_valueChanged(int value)
{
if(value == 0)
{
    ui->sellSumLineEdit->clear();
    ui->sellSumLineEdit->setEnabled(false);
    ui->sellOkBtn->setEnabled(false);
}
else
{
    ui->sellSumLineEdit->setEnabled(true);
    ui->sellSumLineEdit->setReadOnly(true);
    qreal sum = value*(ui->sellPriceLineEdit->text().toInt());
    ui->sellSumLineEdit->setText(QString::number(sum));
    ui->sellOkBtn->setEnabled(true);
}
qDebug()<<"出售商品数量改变";
}


// 出售商品的取消按钮
void Widget::on_sellCancelBtn_clicked()
{
 ui->sellTypeComboBox->setCurrentIndex(0);             //会触发currentIndexChanged(QString&)信号
 ui->sellBrandComboBox->clear();
 ui->sellBrandComboBox->setEnabled(false);
 ui->sellPriceLineEdit->clear();
 ui->sellPriceLineEdit->setEnabled(false);
 ui->sellNumSpinBox->setValue(0);
 ui->sellNumSpinBox->setEnabled(false);
 ui->sellSumLineEdit->clear();
 ui->sellSumLineEdit->setEnabled(false);
 ui->sellOkBtn->setEnabled(false);
 ui->sellCancelBtn->setEnabled(false);
 ui->sellLastNumLabel->setVisible(false);
 qDebug()<<"取消按钮";
}


void Widget::on_sellOkBtn_clicked()
{
QString type = ui->sellTypeComboBox->currentText();
QString name = ui->sellBrandComboBox->currentText();
int value = ui->sellNumSpinBox->value();
//sellNumSpinBox的最大值就是以前的剩余量
int last = ui->sellNumSpinBox->maximum()-value;
QSqlQuery query(db);
//获取以前的销售量
query.exec(QString("select sell from brand where name = '%1' and type = '%2'").arg(name).arg(type));
query.next();
int sell = query.value(0).toInt() + value;         //sell表示已经售出的数量,其中第一个加数表示默认已经售出的数量，第二个加数表示后来售出的数量

//事务操作
QSqlDatabase::database().transaction();
bool rtn = query.exec(QString("update brand set sell = %1,last = %2 where name = '%3' and type = '%4'").arg(sell).arg(last).arg(name).arg(type));

if(rtn)
{
    QSqlDatabase::database().commit();                       //提交事务
    QMessageBox::information(this,"提示","购买成功",QMessageBox::Ok);
    writeXml();
    showDailyList();
    on_sellCancelBtn_clicked();
}
else
QSqlDatabase::database().rollback();              //回滚事务
}


//获取当前日期和时间
QString Widget::getDateTime(Widget::DateTimeType type)
{
    QDateTime datetime = QDateTime::currentDateTime();            //获取当前时间日期
    QString date = datetime.toString("yyyy-MM-dd");
    QString time = datetime.toString("hh:mm");
    QString dateAndTime = datetime.toString("yyyy-MM-dd dddd hh:mm");            //dddd会显示星期几
    if(type == Date)  return date;
    else if(type == Time)  return time;
    else  return dateAndTime;
}


//读取XML文档
bool Widget::docRead()
{
    QFile file("data.xml");
    if(!file.open(QIODevice::ReadOnly))
      {
      QMessageBox::warning(this,"读取文件","文件读取失败","确认");
      return false;
      }
    if(!doc.setContent(&file))
    {
        file.close();
        return false;
    }
    file.close();
    return true;
}


//写入XML文档
bool Widget::docWrite()
{
    QFile file("data.xml");
    if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        QMessageBox::warning(this,"写入文件","文件写入失败","确认");
        return false;
    }
 QTextStream out(&file);
 doc.save(out,4);                     //将doc的内容保存到out中，而out和file绑定
 file.close();
 return true;
}


// 将销售记录写入文档
void Widget::writeXml()
{
//先从文件读取
 if(docRead())
 {
 QString currentDate = getDateTime(Date);
 QDomElement root = doc.documentElement();           //返回根节点
 //根据是否有日期节点进行处理
 if(!root.hasChildNodes())                          //如果没有日期节点
   {
   QDomElement date = doc.createElement(QString("日期"));
   QDomAttr curDate = doc.createAttribute("date");
   curDate.setValue(currentDate);                  //为属性设置值
   date.setAttributeNode(curDate);
   root.appendChild(date);
   createNodes(date);
   }
 else
   {
   QDomElement date = root.lastChild().toElement();
   //根据是否已经有今天的日期节点进行处理
   if(date.attribute("date") == currentDate)     //如果有今天的日期节点
    createNodes(date);
   else
   {
   QDomElement date = doc.createElement(QString("日期"));
   QDomAttr curDate = doc.createAttribute("date");
   curDate.setValue(currentDate);
   date.setAttributeNode(curDate);
   root.appendChild(date);
   createNodes(date);
   }
   }
 //写入到文件
 docWrite();
 }
}


//创建节点
void Widget::createNodes(QDomElement &date)
{
    QDomElement time = doc.createElement(QString("时间"));
    QDomAttr curTime = doc.createAttribute("time");
    curTime.setValue(getDateTime(Time));
    time.setAttributeNode(curTime);
    date.appendChild(time);              //将time设置为date的子节点
    QDomElement type = doc.createElement(QString("类型"));
    QDomElement brand = doc.createElement(QString("品牌"));
    QDomElement price = doc.createElement(QString("单价"));
    QDomElement num = doc.createElement(QString("数量"));
    QDomElement sum = doc.createElement(QString("金额"));
    QDomText text;
    text = doc.createTextNode(QString("%1").arg(ui->sellTypeComboBox->currentText()));
    type.appendChild(text);
    text = doc.createTextNode(QString("%1").arg(ui->sellBrandComboBox->currentText()));
    brand.appendChild(text);
    text = doc.createTextNode(QString("%1").arg(ui->sellPriceLineEdit->text()));
    price.appendChild(text);
    text = doc.createTextNode(QString("%1").arg(ui->sellNumSpinBox->value()));
    num.appendChild(text);
    text = doc.createTextNode(QString("%1").arg(ui->sellSumLineEdit->text()));
    sum.appendChild(text);

    time.appendChild(type);
    time.appendChild(brand);
    time.appendChild(price);
    time.appendChild(num);
    time.appendChild(sum);
}


//显示日销售清单
void Widget::showDailyList()
{
    ui->dailyList->clear();
    if(docRead())
    {
    QDomElement root = doc.documentElement();
    QString title = root.tagName();
    QListWidgetItem* titleItem = new QListWidgetItem;
    titleItem->setText(QString("- - - - - %1 - - - - -").arg(title));
    titleItem->setTextAlignment(Qt::AlignCenter);
    ui->dailyList->addItem(titleItem);
    if(root.hasChildNodes())
      {
      QString currentDate = getDateTime(Date);
      QDomElement dateElement = root.lastChild().toElement();
      QString date = dateElement.attribute("date");
      if(date == currentDate)
       {
       ui->dailyList->addItem("");        //占一个空行
       ui->dailyList->addItem(QString("日期:%1").arg(date));
       ui->dailyList->addItem("");
       QDomNodeList children = dateElement.childNodes();         //返回dateElement所有子节点的列表
       //遍历当日销售的所有商品
       for(int i=0;i<children.count();++i)
         {
         QDomNode node = children.at(i);
         QString time = node.toElement().attribute("time");
         QDomNodeList list = node.childNodes();
         QString type = list.at(0).toElement().text();
         QString brand = list.at(1).toElement().text();
         QString price = list.at(2).toElement().text();
         QString num = list.at(3).toElement().text();
         QString sum = list.at(4).toElement().text();
         QString str = time + " 出售 " + brand + type + " "+num + "台, " + "单价:" + price + "元,共" + sum + "元";
         QListWidgetItem* tempItem = new QListWidgetItem;
         tempItem->setText("********************************");
         tempItem->setTextAlignment(Qt::AlignCenter);
         ui->dailyList->addItem(tempItem);
         ui->dailyList->addItem(str);
         }
       }
      }
    }
}


//创建销售数据模型和视图
void Widget::createChartModelView()
{
chartModel = new QStandardItemModel(this);
chartModel->setColumnCount(2);
chartModel->setHeaderData(0,Qt::Horizontal,QString("品牌"));
chartModel->setHeaderData(1,Qt::Horizontal,QString("销售数量"));

QSplitter* splitter = new QSplitter(ui->chartPage);
splitter->resize(700,320);
splitter->move(0,80);
QTableView* table = new QTableView;
PieView* pieChart = new PieView;
splitter->addWidget(table);
splitter->addWidget(pieChart);
splitter->setStretchFactor(0,1);
splitter->setStretchFactor(1,2);

table->setModel(chartModel);
pieChart->setModel(chartModel);

QItemSelectionModel* selectionModel = new QItemSelectionModel(chartModel);
table->setSelectionModel(selectionModel);
pieChart->setSelectionModel(selectionModel);
}


//给数据模型添加数据
void Widget::showChart()
{
    QSqlQuery query(db);

    query.exec(QString("select name,sell from brand where type = '%1'").arg(ui->typeComboBox->currentText()));

    chartModel->removeRows(0,chartModel->rowCount(QModelIndex()),QModelIndex());
    int row = 0;

    while (query.next()) {
       int r = qrand() % 256;
       int g = qrand() % 256;
       int b = qrand() % 256;

       chartModel->insertRows(row,1,QModelIndex());
       chartModel->setData(chartModel->index(row,0,QModelIndex()),query.value(0).toString());
       chartModel->setData(chartModel->index(row,1,QModelIndex()),query.value(1).toInt());
       chartModel->setData(chartModel->index(row,0,QModelIndex()),QColor(r,g,b),Qt::DecorationRole);
       ++row;
    }
}


void Widget::on_typeComboBox_currentIndexChanged(const QString &type)
{
if(type != "请选择类型")
  showChart();
}


void Widget::on_updateBtn_clicked()
{
if(ui->typeComboBox->currentText() != "请选择类型")
  showChart();
}


//商品管理按钮
void Widget::on_manageBtn_clicked()
{
ui->stackedWidget->setCurrentIndex(0);
}


//销售统计按钮
void Widget::on_chartBtn_clicked()
{
ui->stackedWidget->setCurrentIndex(1);
}

















