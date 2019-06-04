#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(tr("Main Window"));

    QStyle* style = QApplication::style();
    QIcon icon = style->standardIcon(QStyle::SP_DialogOpenButton);
    openAction = new QAction(QIcon(icon), tr("&Open..."), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing file"));

    connect(openAction, &QAction::triggered, this, &MainWindow::open);

    QMenu *file = menuBar()->addMenu(tr("&File"));
    file->addAction(openAction);

    QToolBar *toolBar = addToolBar(tr("&File"));
    toolBar->addAction(openAction);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::open()
{
    // 对话框是用于短期任务和简单交互的顶层窗口
    // 对话框分为模态对话框和非模态对话框
    // 1. 模态对话框会阻塞其它窗口的输入。如记事本程序的“打开”对话框。Qt 有应用程序和窗口两种级别的模态：
    // 1.1 应用程序级别模态：用户只能与当前对话框交互，应用程序中其他窗口被阻塞。默认是应用程序级别模态。
    //     应用程序级别模态使用 QDialog::exec() 显示对话框。
    // 1.2 窗口级别的模态：仅阻塞父窗口，但是允许用户与应用程序中其它窗口交互。
    //     窗口级别模态使用 QDialog::open() 显示对话框。
    // 2. 非模块对话框不会阻塞其他窗口的输入。如记事本程序的“查找”对话框。
    //    非模态对话框使用 QDialog::show() 显示。

#if 0                   // 栈上创建，本函数返回里 dialog 对象被析构
    QDialog dialog;
    dialog.setWindowTitle(tr("Hello, dialog!"));
    dialog.exec();      // 应用程序级别模态
    // dialog.open();   // 窗口级别模态，单这一句不行，还需要更多代码支持，此处暂不深究
    // dialog.show();   // 非模态。这一句不会阻塞程序，本函数返回时堆上对象 dialog 立即被析构，因此一闪即逝
#else                   // 堆上创建，本函数返回里 dialog 对象不会被析构
    QDialog *dialog = new QDialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Hello, dialog!"));
    // dialog->exec();  // 应用程序级别模态
    // dialog->open();  // 窗口级别模态，单这一句不行，还需要更多代码支持，此处暂不深究
    dialog->show();     // 非模态。这一句不会阻塞程序。本 函数返回时堆上对象 dialog 不会被析构
#endif
}
