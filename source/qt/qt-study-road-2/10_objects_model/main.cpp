#include "widget.h"
#include <QApplication>
#include <QPushButton>
#include <QTimer>

int main(int argc, char *argv[])
{
#if 0
    QApplication a(argc, argv);
    Widget w;
    w.show();

    return a.exec();
#else
    QApplication app(argc, argv);

    // 注意两点：
    // 1. Qt 保证，析构 QObject 对象时，如果这个对象有子对象，则自动析构每一个子对象；如果有父对象，则从父对象的子对象
    //    列表中删除当前对象。Qt 会使用合适的析构顺序。
    // 2. C++ 中，局部对象的析构顺序与创建顺序相反。
#if 0
    // main() 返回时会先析构 btn，再析构 win。btn 析构后，win 的子对象中不再有 btn。因此再析构 win 没有问题。
    QWidget win;
    QPushButton btn("BUTTON", &win);
#else
    // main() 返回时会先析构 win，再析构 btn。但 win 析构时，其子对象 btn 会被析构。因此 btn 被析构两次，出现段错误。
    QPushButton btn("BUTTON");
    QWidget win;
    btn.setParent(&win);
#endif
    // 结论：注意对象的创建顺序，应先创建父对象，后创建子对象

    win.show();                                   // 显示窗体
    QTimer::singleShot(4000, &app, SLOT(quit())); // 8 秒之后，调用 app 对象的 quit() 函数

    return app.exec();
#endif
}
