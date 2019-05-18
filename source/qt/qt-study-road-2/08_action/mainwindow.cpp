#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QStyle>

/*
  Qt 使用 QAction 类表示窗口的一个“动作”，这个动作可能显示为一个菜单项，当用户点击该菜单项，执行相应
  操作；也可能显示为一个工具栏按钮，用户点击该按钮时执行相应操作。
  
  无论 QAction 是出现在菜单栏还是工具栏，用户点击之后，执行的动作是一样的。Qt 并没有针对菜单项或工具
  栏分别设计“动作”类，而是使用 QAction 类抽象出公共的“动作”。当我们把 QAction 对象添加到菜单，就显示
  成一个菜单项，添加到工具栏，就显示成一个工具栏按钮。用户可以通过点击菜单项、点击工具栏按钮、使用快
  捷键来激活这个动作。

  QAction 包含了图标、菜单文字、快捷键、状态栏文字、浮动帮助等信息。在代码中添加一个 QAction 对象后，
  Qt 会选择使用哪个属性来显示，无需用户关心。同时，Qt 能够保证菜单、工具栏中 QAction 对象显示内容是
  同步的。也就是说，如果我们在菜单中修改了 QAction 的状态，那么在工具栏上面这个 QAction 状态也会改变。
*/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),        // 初始化父类 QMainWindow，向导生成
    ui(new Ui::MainWindow)      // 为成员变量 ui 分配空间并调用 Ui::MainWindow 构造函数，向导生成
{
    // this 指向 ::MainWindow 对象，表示处理资源的类
    // ui 指向 Ui::MainWindow 对象，表示窗体资源
    ui->setupUi(this);          // 将资源与处理资源的类关联了起来，向导生成

    // 设置窗口标题
    setWindowTitle(tr("Main Window"));

    QStyle* style = QApplication::style();
    QIcon icon = style->standardIcon(QStyle::SP_DialogOpenButton);  // 使用 Qt 内置图标
    // 创建 QAction 对象，为构造函数传入图标、标题文本和 this 指针
    // 标题文本前的 '&' 表示这个动作有一个快捷键
    openAction = new QAction(QIcon(icon), tr("&Open..."), this);
    // 设置快捷键，QKeySequence 中定义了很多内置快捷键
    openAction->setShortcuts(QKeySequence::Open);
    // 设置鼠标悬停提示
    openAction->setStatusTip(tr("Open an existing file"));

    // 将 QAction 对象的 triggered() 信号与 MainWindow 对象的 open() 函数连接起来
    // triggered() 信号发生时，将自动调用 open() 函数。是为信号槽机制
    connect(openAction, &QAction::triggered, this, &MainWindow::open);

    // 为 MainWindow 窗口中添加菜单 "File"
    QMenu *file = menuBar()->addMenu(tr("&File"));
    // 为 "File" 菜单添加动作
    file->addAction(openAction);

    // 为 MainWindow 窗口中添加工具栏按钮 "File"
    QToolBar *toolBar = addToolBar(tr("&File"));
    // 为工具栏按钮 "File" 添加动作
    toolBar->addAction(openAction);

    statusBar() ;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::open()
{
    QMessageBox::information(this, tr("Information"), tr("Open a file"));
}
