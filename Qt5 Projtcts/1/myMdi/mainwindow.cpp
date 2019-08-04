#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mdichild.h"
#include <QMdiSubWindow>
#include <QFileDialog>
#include <QSignalMapper>
#include <QSettings>
#include <QCloseEvent>
#include <QLabel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //创建间隔器动作并在其中设置间隔器
    actionSeparator = new QAction(this);
    actionSeparator->setSeparator(true);
    updateMenus();
    connect(ui->mdiArea,SIGNAL(subWindowActivated(QMdiSubWindow*)),this,SLOT(updateMenus()));    //当有活动窗口时更新菜单

    windowMapper = new QSignalMapper(this);            //创建信号映射器
    //映射器重新发射信号，根据信号设置活动窗口
    connect(windowMapper,SIGNAL(mapped(QWidget*)),this,SLOT(setActiveSubWindow(QWidget*)));
    updateWindowMenu();                      //更新窗口菜单，并且设置当窗口菜单将要显示的时候更新窗口菜单
    connect(ui->menuW,SIGNAL(aboutToShow()),this,SLOT(updateWindowMenu()));

    readSettings();                     //初始窗口时读取窗口设置信息
    initWindow();                      //初始化窗口
}

MainWindow::~MainWindow()
{
    delete ui;
}

//新建文件
void MainWindow::on_actionNew_triggered()
{
//    MdiChild* child = new MdiChild;         //创建MdiChild
//    this->ui->mdiArea->addSubWindow(child);      //多文档区域添加子窗口
    MdiChild* child = createMdiChild();
    child->newFile();              //新建文件
    child->show();                 //显示子窗口
}


void MainWindow::updateMenus()
{
    //根据是否有获得窗口来设置各个动作是否可用
    bool hasMdiChild = (this->activeMdiChild() != 0);
    ui->actionSave->setEnabled(hasMdiChild);
    ui->actionSaveAs->setEnabled(hasMdiChild);
    ui->actionPaste->setEnabled(hasMdiChild);
    ui->actionClose->setEnabled(hasMdiChild);
    ui->actionCloseAll->setEnabled(hasMdiChild);
    ui->actionTile->setEnabled(hasMdiChild);
    ui->actionCascade->setEnabled(hasMdiChild);
    ui->actionNext->setEnabled(hasMdiChild);
    ui->actionPrevious->setEnabled(hasMdiChild);


    this->actionSeparator->setVisible(hasMdiChild);             //设置间隔器是否显示

    //有活动窗口且有被选择的文本，剪切复制才可用
    bool hasSelection = (this->activeMdiChild()&&activeMdiChild()->textCursor().hasSelection());
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);

    //有活动窗口且文档的有撤销操作时撤销动作可用
    ui->actionUndo->setEnabled(this->activeMdiChild() && activeMdiChild()->document()->isUndoAvailable());
    //有活动窗口且文档的有恢复操作时恢复动作可用
    ui->actionRedo->setEnabled(this->activeMdiChild() && activeMdiChild()->document()->isRedoAvailable());

}

MdiChild* MainWindow::activeMdiChild()
{
    if(QMdiSubWindow* activeSubWindow = ui->mdiArea->activeSubWindow())
      return qobject_cast<MdiChild*>(activeSubWindow->widget());
    return 0;
}


MdiChild* MainWindow::createMdiChild()
{
    MdiChild* child = new MdiChild;        //创建MdiChild部件
    ui->mdiArea->addSubWindow(child);      //向多文档区域添加子窗口，child为中心部件
    //根据QTextEdit类的是否可以复制信号设置剪切复制动作是否可用
    connect(child,SIGNAL(copyAvailable(bool)),ui->actionCut,SLOT(setEnabled(bool)));
    connect(child,SIGNAL(copyAvailable(bool)),ui->actionCopy,SLOT(setEnabled(bool)));
    //根据QTextDocumentleide是否可以撤销恢复信号设置撤销恢复动作是否可用
    connect(child->document(),SIGNAL(undoAvailable(bool)),ui->actionUndo,SLOT(setEnabled(bool)));
    connect(child->document(),SIGNAL(redoAvailable(bool)),ui->actionRedo,SLOT(setEnabled(bool)));
    //每当编辑器中的光标位置改变，就重新显示行号和列号
    connect(child,SIGNAL(cursorPositionChanged()),this,SLOT(showTextRowAndCol()));
    return child;
}



void MainWindow::on_actionOpen_triggered()
{
  QString fileName = QFileDialog::getOpenFileName(this);
  if(!fileName.isEmpty())                      //如果路径不为空，则查看该文件是否已经打开
    {
      QMdiSubWindow* existing = findMdiChild(fileName);
      if(existing)                       //如果已经存在，则将对应的子窗口设置为活动窗口
      {
          ui->mdiArea->setActiveSubWindow(existing);
          return;
      }
      //如果没有打开，则创建子窗口
      MdiChild* child = createMdiChild();
      if(child->loadFile(fileName))            //加载文件成功
      {
          ui->statusBar->showMessage(tr("打开文件成功"),2000);
          child->show();
      }
      else
          child->close();
    }
}


QMdiSubWindow* MainWindow::findMdiChild(const QString &fileName)
{
QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
//利用foreach语句遍历子窗口列表，如果其文件路径和要查找的路径相同，则返回该窗口
foreach(QMdiSubWindow* window,ui->mdiArea->subWindowList())
{
    MdiChild* mdiChild = qobject_cast<MdiChild*>(window->widget());
    if(mdiChild->currentFile() == canonicalFilePath)
    return window;
}
return 0;
}

void MainWindow::setActiveSubWindow(QWidget *window)            //将传递过来的窗口部件设置为活动窗口
{
    if(!window)
      return;
    //如果传递了窗口部件，则将其设置为活动窗口
    ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(window));
}


void MainWindow::updateWindowMenu()
{
    //先清空菜单，然后添加各个菜单动作
    ui->menuW->clear();
    ui->menuW->addAction(ui->actionClose);
    ui->menuW->addAction(ui->actionCloseAll);
    ui->menuW->addSeparator();
    ui->menuW->addAction(ui->actionTile);
    ui->menuW->addAction(ui->actionCascade);
    ui->menuW->addSeparator();
    ui->menuW->addAction(ui->actionNext);
    ui->menuW->addAction(ui->actionPrevious);
    ui->menuW->addAction(actionSeparator);

    //如果有活动窗口，则显示间隔器
    QList<QMdiSubWindow*> windows = ui->mdiArea->subWindowList();
    actionSeparator->setVisible(!windows.isEmpty());
    for(int i=0;i<windows.size();++i)           //遍历各个子窗口
    {
        MdiChild* child = qobject_cast<MdiChild*>(windows.at(i)->widget());
        QString text;
        if(i<9)              //如果窗口数小于9，则设置编号为快捷键
            text = tr("&%1 %2").arg(i+1).arg(child->userFriendlyCurrentFile());
        else
            text = tr("%1 %2").arg(i+1).arg(child->userFriendlyCurrentFile());
        //添加动作到菜单，设置动作可以选择
        QAction* action = ui->menuW->addAction(text);
        action->setCheckable(true);
        action->setChecked(child == activeMdiChild());    //设置当前活动窗口动作为选中状态
        //关联动作的触发信号到信号映射器的map()槽，这个槽会发射mapped()信号
        connect(action,SIGNAL(triggered(bool)),windowMapper,SLOT(map()));
       //将动作与相应的窗口部件进行映射，在发射mapped()信号时就会以这个窗口部件为参数
        windowMapper->setMapping(action,windows.at(i));
    }
}

//保存
void MainWindow::on_actionSave_triggered()
{
    if(activeMdiChild()&&activeMdiChild()->save())
               ui->statusBar->showMessage(tr("文件保存成功"),2000);
}

//另存为
void MainWindow::on_actionSaveAs_triggered()
{
    if(activeMdiChild()&&activeMdiChild()->saveAs())
                ui->statusBar->showMessage(tr("文件保存成功"),2000);
}

//撤销
void MainWindow::on_actionUndo_triggered()
{
  if(activeMdiChild())
      activeMdiChild()->undo();
}

//恢复
void MainWindow::on_actionRedo_triggered()
{
    if(activeMdiChild())
       activeMdiChild()->redo();
}

//剪切
void MainWindow::on_actionCut_triggered()
{
    if(activeMdiChild())
       activeMdiChild()->cut();
}

//复制
void MainWindow::on_actionCopy_triggered()
{
    if(activeMdiChild())
       activeMdiChild()->copy();
}


//粘贴
void MainWindow::on_actionPaste_triggered()
{
    if(activeMdiChild())
       activeMdiChild()->paste();
}


//关闭菜单
void MainWindow::on_actionClose_triggered()
{
  ui->mdiArea->closeActiveSubWindow();
}

//关闭所有窗口菜单
void MainWindow::on_actionCloseAll_triggered()
{
 ui->mdiArea->closeAllSubWindows();
}

//平铺
void MainWindow::on_actionTile_triggered()
{
    ui->mdiArea->tileSubWindows();
}

//层叠
void MainWindow::on_actionCascade_triggered()
{
    ui->mdiArea->cascadeSubWindows();
}

//下一个菜单
void MainWindow::on_actionNext_triggered()
{
    ui->mdiArea->activateNextSubWindow();
}

//上一个菜单
void MainWindow::on_actionPrevious_triggered()
{
    ui->mdiArea->activatePreviousSubWindow();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  ui->mdiArea->closeAllSubWindows();           //先执行多文档区域的关闭操作
    if(ui->mdiArea->currentSubWindow())         //如果还有窗口没关闭，则忽略该事件
       event->ignore();
    else
    {                                          //在关闭前写入窗口设置
        writeSettings();
        event->accept();
    }
}

void MainWindow::on_actionExit_triggered()
{
 //qApp是QApplication对象的全局指针
    qApp->closeAllWindows();           //等价于调用QApplication::closeAllWindows()
}

//关于菜单
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,tr("关于本软件"),tr("欢迎访问我们的网站：www.yafeilinux.com"));
}

//关于Qt菜单
void MainWindow::on_actionAboutQt_triggered()
{
  qApp->aboutQt();          // 这里的qApp是QApplication对象的全局指针，这行代码相当于QApplication::aboutQt();
}


//写入窗口设置
void MainWindow::writeSettings()
{
    QSettings settings("yafeilinux","myMdi");
//写入位置信息和大小信息
    settings.setValue("pos",this->pos());
    settings.setValue("size",this->size());
}


//读取窗口设置
void MainWindow::readSettings()
{
 QSettings settings("yafeilinux","myMdi");
 QPoint pos = settings.value("pos",QPoint(200,200)).toPoint();
 QSize size = settings.value("size",QSize(400,400)).toSize();
 this->move(pos);
 this->resize(size);
}


void MainWindow::showTextRowAndCol()
{
if(this->activeMdiChild())      //如果有活动窗口，则显示其中光标所在的位置
  {//因为获取的行号和列号都是从0开始的，所以我们这里进行了加1
    int rowNumber = this->activeMdiChild()->textCursor().blockNumber() + 1;
    int colNumber = this->activeMdiChild()->textCursor().columnNumber() + 1;
    ui->statusBar->showMessage(tr("%1行 %2列").arg(rowNumber).arg(colNumber));
  }
}


void MainWindow::initWindow()
{
this->setWindowTitle(tr("多文档编辑器"));
ui->mainToolBar->setWindowTitle(tr("工具栏"));            //在工具栏上右击时，可以关闭工具栏
//多文档区域的内容超出可视区域后，出现滚动条
ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

ui->statusBar->showMessage(tr("欢迎使用多文档编辑器"));
QLabel* label = new QLabel(this);
label->setFrameStyle(QFrame::Box|QFrame::Sunken);
label->setText(tr("<a href = \"http://www.yafeilinux.com\">yafeilinux.com</a>"));
label->setTextFormat(Qt::RichText);           //标签文本为富文本
label->setOpenExternalLinks(true);               //可以打开外部链接
ui->statusBar->addPermanentWidget(label);

ui->actionNew->setStatusTip(tr("创建一个文件"));
ui->actionOpen->setStatusTip(tr("打开一个已经存在的文件"));
ui->actionSave->setStatusTip(tr("保存文档到硬盘"));
ui->actionSaveAs->setStatusTip(tr("以新的名称保存文档"));
ui->actionExit->setStatusTip(tr("退出应用程序"));
ui->actionUndo->setStatusTip(tr("撤销先前的操作"));
ui->actionRedo->setStatusTip(tr("恢复先前的操作"));
ui->actionCut->setStatusTip(tr("剪切选中的内容到剪贴板"));
ui->actionCopy->setStatusTip(tr("复制选中的内容到剪贴板"));
ui->actionPaste->setStatusTip(tr("粘贴剪贴板的内容到当前位置"));
ui->actionClose->setStatusTip(tr("关闭活动窗口"));
ui->actionCloseAll->setStatusTip(tr("关闭所有窗口"));
ui->actionTile->setStatusTip(tr("平铺所有窗口"));
ui->actionCascade->setStatusTip(tr("层叠所有窗口"));
ui->actionNext->setStatusTip(tr("将焦点移动到下一个窗口"));
ui->actionPrevious->setStatusTip(tr("将焦点移动到前一个窗口"));
ui->actionAbout->setStatusTip(tr("显示本软件的介绍"));
ui->actionAboutQt->setStatusTip(tr("显示Qt的介绍"));

}










