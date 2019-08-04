#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setFixedSize(400,300);
    setWindowTitle(tr("登录"));
    ui->pwdLineEdit->setFocus();
    ui->loginBtn->setDefault(true);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_loginBtn_clicked()
{
    if(ui->pwdLineEdit->text().isEmpty())
      {
     QMessageBox::information(this,"请输入密码","请先输入密码再登录!",QMessageBox::Ok);
     ui->pwdLineEdit->setFocus();
      }
    else
      {
 QSqlQuery query(db);
 query.exec("select pwd from password");
 qDebug()<<query.lastError();
 query.next();
 if(query.value(0).toString() == ui->pwdLineEdit->text())
 {
   QDialog::accept();
 }
 else
  {
     QMessageBox::warning(this,"密码错误","请输入正确的密码!",QMessageBox::Ok);
     ui->pwdLineEdit->clear();
     ui->pwdLineEdit->setFocus();
  }
      }
}

void LoginDialog::on_quitBtn_clicked()
{
QDialog::reject();
}








