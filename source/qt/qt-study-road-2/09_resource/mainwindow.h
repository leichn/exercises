#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

/* 如下注释 1. 和 2. 两步是 PIMPL 技法，用于解开类的接口与实现的耦合，减少编译依赖

   如果不使用 PIMPL，则代码类似如下：
   #include "ui_mainwindow.h"
   class MainWindow : public QMainWindow
   {
   private:
       Ui::MainWindow ui;
   };
   本文件 mainwindow.h 依赖于 ui_mainwindow.h，ui_mainwindow.h 中关于 Ui::MainWindow 类定义的变化将
   导致所有包含 mainwindow.h 的源文件都被重新编译。

   实际上这是不必要的。公有成员的声明是类的接口部分，而私有成员实际属于类的实现部分。类的实现部分应
   当向类的使用者隐藏，其改变不应引起使用者重编译。类的接口部分的改变才应要求使用者重编译。按照这种
   思路，类的接口应定义在头文件中，而类的实现应定义在源文件中。私有成员包含在头文件中实际是 C++ 中一
   个不完善的地方，向外部暴露了本该隐藏的信息。

   PIMPL 技法在编译解除了类的接口与实现的耦合，减少了编译依赖。在本例中，如果类 Ui::MainWindow 的定
   义有变化，不会引起包含了 mainwindow.h 的源文件的重新编译。
*/

// 1. 前导声明 Ui::MainWindow，这个定义于构建项目时自动生成的 ui_mainwindow.h 中
namespace Ui {
class MainWindow;
}

// 此处 MainWindow 类定义在全局域，即 ::MainWindow，是处理资源的类
// 成员变量 ui 定义在 Ui 域，其类型 Ui::MainWindow 定义于 ui_mainwindow.h 中，表示资源
// 二者都叫 MainWindow，但位于不同的命名空间(域)，所以是不同的两个类
// Qt 中与资源相关的定义全部位于 Ui 命名空间中
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // 2. 私有成员变量是一个指针，指向 Ui::MainWindow
    // 一个硬件平台指针变量的的长度是确定的 (32 位或 64 位) ，因此类类型的长度也是确定的
    // Ui::MainWindow 定义的改变只会引起使用了 Ui::MainWindow 类的源文件重新编译，而不会引起其他包含
    // 本头文件的源文件重新编译。即：本头文件 (MainWindow 类) 与 Ui::MainWindow 类解除了耦合。
    Ui::MainWindow *ui;

private:
    void open();
    QAction *openAction;
};

#endif // MAINWINDOW_H
