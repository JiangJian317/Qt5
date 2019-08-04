#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSqlDatabase>
#include <QDomDocument>

class QStandardItemModel;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    bool createConnection();              //连接数据库
    enum DateTimeType{Time,Date,DateTime};
    QString getDateTime(DateTimeType type);
    QSqlDatabase retDbCon(){return db;}
private:
bool docRead();
bool docWrite();
void writeXml();
void createNodes(QDomElement& date);
void showDailyList();

void createChartModelView();
void showChart();
private slots:
    void on_sellTypeComboBox_currentIndexChanged(const QString &type);

    void on_sellBrandComboBox_currentIndexChanged(const QString &brand);

    void on_sellNumSpinBox_valueChanged(int value);

    void on_sellCancelBtn_clicked();

    void on_sellOkBtn_clicked();

    void on_typeComboBox_currentIndexChanged(const QString &type);

    void on_updateBtn_clicked();

    void on_manageBtn_clicked();

    void on_chartBtn_clicked();

private:
    Ui::Widget *ui;
    QSqlDatabase db;
    QDomDocument doc;
    QStandardItemModel* chartModel;
};

#endif // WIDGET_H
