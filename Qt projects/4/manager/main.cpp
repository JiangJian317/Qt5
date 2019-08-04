#include "widget.h"
#include <QApplication>
#include "connection.h"
#include "logindialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QString strLibPath(QDir::toNativeSeparators(QApplication::applicationDirPath())+QDir::separator()+"plugins");
//    a.addLibraryPath(strLibPath);
    a.addLibraryPath("../dataManagerSystem/sqldrivers");
    Widget w;
    if(!w.createConnection()||!createXml())
        return 0;
    LoginDialog dlg;
    dlg.setDb(w);
    if(dlg.exec() == QDialog::Accepted)
    {
        w.show();
    return a.exec();
    }
    else
        return 0;
}
