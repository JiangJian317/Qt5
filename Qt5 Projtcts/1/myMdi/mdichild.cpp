#include "mdichild.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QFileInfo>
#include <QFileDialog>
#include <QCloseEvent>
#include <QPushButton>
#include <QMenu>

MdiChild::MdiChild(QWidget* parent):QTextEdit(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);           //设置在子窗口关闭时销毁这个类的对象
    isUntitled = true;

}

void MdiChild::newFile()
{
    static int sequenceNumber = 1;              //设置窗口编号，因为编号一直被保存，所以需要使用静态变量
    isUntitled = true;                                  //新建的文档没有被保存过
    curFile = tr("未命名文档%1.txt").arg(sequenceNumber ++);
    setWindowTitle(curFile + "[*]" + tr(" - 多文档编辑器"));  //设置窗口标题，使用[*]可以在文档被更改后在文件名称后显示*号
    //文档被更改时发射contentChanged()信号，实习documentWasModified()槽
    connect(document(),SIGNAL(contentsChanged()),this,SLOT(documentWasModified()));
}


void MdiChild::documentWasModified()
{
//根据文档的isModified()函数的返回值，判断编辑器的内容是否被更改了
//如果被更改了，就要在设置了[*]的地方显示*号，这里会在窗口标题中显示
    this->setWindowModified(this->document()->isModified());
}

bool MdiChild::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this,tr("多文档编辑器"),tr("无法读取文件 %1:\n%2.").arg(fileName).arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);                 //新建文本流对象
    QApplication::setOverrideCursor(Qt::WaitCursor);       //设置鼠标状态为等待状态
    this->setPlainText(in.readAll());           //读取文件的全部文本内容，并添加到编辑器中
    QApplication::restoreOverrideCursor();       //恢复鼠标状态
    this->setCurrentFile(fileName);              //设置当前文件
    connect(this->document(),SIGNAL(contentsChanged()),this,SLOT(documentWasModified()));
    return true;
}


void MdiChild::setCurrentFile(const QString &fileName)
{
    //canoncialFilePath()可以除去路径中的符号链接,"."和".."等符号
    this->curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;       //文件已经被保存过了
    this->document()->setModified(false);         //文档没有被更改过
    setWindowModified(false);                 //窗口不显示被更改标志
    setWindowTitle(userFriendlyCurrentFile() + "[*]");

}


QString MdiChild::userFriendlyCurrentFile()
{
    return QFileInfo(curFile).fileName();           //从文件路径中提取文件名
}

bool MdiChild::save()
{
    //如果文件未被保存过，则执行另存为操作，否则直接保存文件
    if(isUntitled)
      return saveAs();
    else
        return saveFile(curFile);
}


bool MdiChild::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("另存为"),curFile);
    if(fileName.isEmpty())      //如果文件路径为空，则返回false,否则保存文件
       return false;
return saveFile(fileName);
}


bool MdiChild::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this,tr("多文档编辑器"),tr("无法写入文件 %1:\n%2").arg(fileName).arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out<<this->toPlainText();                  //以纯文本写入
    QApplication::restoreOverrideCursor();
    setCurrentFile(fileName);
    return true;
}


void MdiChild::closeEvent(QCloseEvent *event)
{
    //如果maybeSave()函数返回true,则关闭窗口，否则忽略该事件
    if(maybeSave())
       event->accept();
    else
       event->ignore();
}


bool MdiChild::maybeSave()
{
    if(this->document()->isModified())             //如果文档被更改过
    {
        QMessageBox box;
        box.setWindowTitle(tr("多文档编辑器"));
        box.setText(tr("是否保存对“%1”的更改?").arg(userFriendlyCurrentFile()));     //为什么%1要用中文双引号括起来？
        box.setIcon(QMessageBox::Warning);
        //添加按钮，QMessageBox::YesRole可以表明这个按钮的行为
        QPushButton* yesBtn = box.addButton(tr("是(&Y)"),QMessageBox::YesRole);
        box.addButton(tr("否(&N)"),QMessageBox::NoRole);
        QPushButton* cancelBtn = box.addButton(tr("取消"),QMessageBox::RejectRole);
        box.exec();             //弹出对话框，让用户选择是否保存修改，或者选择取消，则返回false
        //如果用户选择是，则返回保存操作的结果；如果选择取消，则返回false
        if(box.clickedButton() == yesBtn)
            return save();
        else if(box.clickedButton() == cancelBtn)
            return false;
    }
    return true;                   //如果文档没有更改过
}


void MdiChild::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu* menu = new QMenu;              //创建菜单，并向其中添加动作
    QAction* undo = menu->addAction(tr("撤销(&U)"),this,SLOT(undo()),QKeySequence::Undo);
    undo->setEnabled(this->document()->isUndoAvailable());
    QAction* redo = menu->addAction(tr("恢复(&R)"),this,SLOT(redo()),QKeySequence::Redo);
    redo->setEnabled(this->document()->isRedoAvailable());
    menu->addSeparator();
    QAction* cut = menu->addAction(tr("剪切(&T)"),this,SLOT(cut()),QKeySequence::Cut);
    cut->setEnabled(this->textCursor().hasSelection());
    QAction* copy = menu->addAction(tr("复制(&C)"),this,SLOT(copy()),QKeySequence::Copy);
    copy->setEnabled(this->textCursor().hasSelection());
    menu->addAction(tr("粘贴(&P)"),this,SLOT(paste()),QKeySequence::Paste);
    QAction* clear = menu->addAction(tr("清空"),this,SLOT(clear()));
    clear->setEnabled(!document()->isEmpty());
    menu->addSeparator();
    QAction* select = menu->addAction(tr("全选"),this,SLOT(selectAll()),QKeySequence::SelectAll);
    select->setEnabled(!document()->isEmpty());

    menu->exec(e->globalPos());           //获取鼠标的位置，然后在这个位置显示菜单
    delete menu;                          //最后销毁这个菜单
}












