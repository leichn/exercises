#include "widget.h"
#include <QSpinBox>
#include <QSlider>
#include <QHBoxLayout>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Enter your age");

    QSpinBox *spinBox = new QSpinBox(this);
    QSlider *slider = new QSlider(Qt::Horizontal, this);
    spinBox->setRange(0, 130);
    slider->setRange(0, 130);

    QObject::connect(slider, &QSlider::valueChanged, spinBox, &QSpinBox::setValue);
    // QSpinBox::valueChanged()有两个重载函数，因此此处使用函数指针显式指向其中一个函数
    // 如果不这样处理，下一步 connect() 会编译出错
    void (QSpinBox:: *spinBoxSignal)(int) = &QSpinBox::valueChanged;
    QObject::connect(spinBox, spinBoxSignal, slider, &QSlider::setValue);
    spinBox->setValue(35);

    // Qt 提供了两种组件定位机制：绝对定位和布局定位。
    // 绝对定位使用坐标和长宽值定位。布局定位由布局管理器管理定位。

    // 布局器，按照水平方向从左到右布局
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(spinBox);
    layout->addWidget(slider);
    setLayout(layout);
}

Widget::~Widget()
{

}
