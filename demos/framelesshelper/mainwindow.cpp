#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QMouseEvent>

#include "framelesshelper.h"

QX_FRAMELESS_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QMenuBar *menubar = new QMenuBar(this);
    menubar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QMenu *menu = menubar->addMenu(tr("File"));
    menu->addAction(tr("action1"));
    menu->addAction(tr("action2"));
    menu->addAction(tr("action3"));
    menu = menubar->addMenu(tr("Edit"));
    menu->addAction(tr("action1"));

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(1, 1, 1, 0);
    titleLayout->addWidget(menubar);
    titleLayout->addStretch();
    titleLayout->addWidget(new QPushButton(tr("button1")));
    titleLayout->addWidget(new QPushButton(tr("button2")));
    titleLayout->addWidget(new QPushButton(tr("button3")));

    QWidget *titleWidget = new QWidget(this);
    titleWidget->setLayout(titleLayout);
    setMenuWidget(titleWidget);

    qDebug() << titleWidget->height();

    ui->spinBoxTitleHeight->setRange(10, 40);
    ui->spinBoxBorderWidth->setRange(2, 10);

    FramelessHelper *helper = new FramelessHelper(this);
    qDebug() << helper->titleHeight();

    connect(helper, &FramelessHelper::windowTitleChanged, this, [=](const QString &title) {
       qDebug() << title;
    });
    connect(helper, &FramelessHelper::windowStateChanged, this, [=](Qt::WindowStates state) {
       qDebug() << state;
    });

    ui->checkBoxWidgetMovable->setChecked(helper->widgetMovable());
    ui->checkBoxWidgetResizable->setChecked(helper->widgetResizable());

    connect(ui->checkBoxWidgetMovable, &QCheckBox::clicked, this, [helper](bool checked) {
        helper->setWidgetMovable(checked);
    });
    connect(ui->checkBoxWidgetResizable, &QCheckBox::clicked, this, [this, helper](bool checked) {
        helper->setWidgetResizable(checked);
        ui->statusbar->setSizeGripEnabled(checked);
    });

    ui->spinBoxTitleHeight->setValue(helper->titleHeight());
    ui->spinBoxBorderWidth->setValue(helper->borderWidth());

    connect(ui->spinBoxTitleHeight, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, [helper](int value) {
        helper->setTitleHeight(value);
    });
    connect(ui->spinBoxBorderWidth, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, [helper](int value) {
        helper->setBorderWidth(value);
    });

    // test cases
    {
        // test addWidget and removeWidget

        // FIXME: on NativeWin, the parent parameter of QWidget constructor function cannot specify this,
        // otherwise the helper's WM_NCHITTEST message will fail. And debug report
        // 'C:\Program Files (x86)\SogouInput\12.4.0.6503\Resource.dll':
        //      Shared library architecture i386 is not compatible with target architecture i386:x86-64.
        //
        // QWidget *w1 = new QWidget(this);
        //
        QWidget *w1 = new QWidget();
        helper->addWidget(w1);
        delete w1;

        QWidget *w2 = new QWidget();
        helper->addWidget(w2);
        helper->removeWidget(w2);
        delete w2;

        QWidget w3;
        helper->addWidget(&w3);
    }

    setWindowTitle(tr("FramelessHelper Demo"));
    resize(800, 400);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonMinimize_clicked()
{
    showMinimized();
}

void MainWindow::on_pushButtonClose_clicked()
{
    qApp->quit();
}

void MainWindow::on_pushButtonMaximize_clicked()
{
    if (isMaximized()) {
        ui->pushButtonMaximize->setText(tr("maximize"));
        showNormal();
    } else {
        ui->pushButtonMaximize->setText(tr("restore"));
        showMaximized();
    }
}

void MainWindow::on_pushButtonFull_clicked()
{
    bool isfull = isFullScreen();
    if (isfull) {
        ui->pushButtonFull->setText(tr("fullscreen"));
        showNormal();
    } else {
        ui->pushButtonFull->setText(tr("unfullscreen"));
        showFullScreen();
    }
    ui->pushButtonMinimize->setEnabled(isfull);
    ui->pushButtonMaximize->setEnabled(isfull);
    ui->pushButtonClose->setEnabled(isfull);
}

