#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),        // 初始化父类 QMainWindow，向导生成
    ui(new Ui::MainWindow)      // 为成员变量 ui 分配空间并调用 Ui::MainWindow 构造函数，向导生成
{
    // this 指向 ::MainWindow 对象，表示处理资源的类
    // ui 指向 Ui::MainWindow 对象，表示窗体资源
    ui->setupUi(this);          // 将资源与处理资源的类关联了起来，向导生成
}

MainWindow::~MainWindow()
{
    delete ui;
}
