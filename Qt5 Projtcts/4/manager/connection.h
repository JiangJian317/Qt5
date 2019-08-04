#ifndef CONNECTION_H
#define CONNECTION_H

#include <QtSql>
#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QtXml>

/*
static bool createConnection()
{
    db = QSqlDatabase::addDatabase("QMYSQL","Manager");
    db.setHostName("localhost");
    db.setDatabaseName("m_data");
    db.setPort(3306);
    db.setUserName("root");
    db.setPassword("317727jiangjian");
    QSqlQuery query(db);
    if(!db.open())
    {
        QMessageBox::critical(0,"Cannot open database","Unable to establish a database connection.","取消");
        qDebug()<<query.lastError();
        return false;
    }
 //创建分类表
   query.exec("create table type(id varchar(5) primary key,name varchar(5))");
// query.exec("insert into type values('0','请选择类型'),('01','电视'),('02','空调')");
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
return true;
}
*/

static bool createXml()
{
 QFile file("data.xml");
 if(file.exists())  return true;           //判断文件是否存在，防止重复创建
 if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
 {
    QMessageBox::warning(0,"创建文件","文件创建失败","确认");
    return false;
 }
 QDomDocument doc;                   //它代表一个XML文档
 QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
 doc.appendChild(instruction);
 QDomElement root = doc.createElement(QString("日销售清单"));            //创建根节点
 doc.appendChild(root);
 QTextStream out(&file);
 doc.save(out,4);
 file.close();
 return true;
}

#endif // CONNECTION_H
