#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolBar>
#include <QStyle>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QDebug>

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

#if 1           // 模态对话框数据获取
void MainWindow::open()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Hello, dialog!"));

    QDialogButtonBox *button = new QDialogButtonBox(&dialog);
    button->addButton( "OK", QDialogButtonBox::YesRole);
    button->addButton( "CANCEL", QDialogButtonBox::NoRole);
    connect(button, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(button, SIGNAL(rejected()), &dialog, SLOT(reject()));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(button);
    dialog.setLayout(layout);


    if (dialog.exec() ==  QDialog::Accepted)    // 确定按钮
    {
        qDebug() << "OK";
    }
    else                                        // 取消按钮
    {
        qDebug() << "CANCEL";
    }
    qDebug() << dialog.result();
}

#else           // 非模态对话框数据获取。这里仅是示意代码
// in dialog:
void UserAgeDialog::accept()
{
    emit userAgeChanged(newAge); // newAge is an int
    QDialog::accept();
}

// in main window:
void MainWindow::showUserAgeDialog()
{
    UserAgeDialog *dialog = new UserAgeDialog(this);
    connect(dialog, &UserAgeDialog::userAgeChanged, this, &MainWindow::setUserAge);
    dialog->show();
}

// ...

void MainWindow::setUserAge(int age)
{
    userAge = age;
}
#endif
