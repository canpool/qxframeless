#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QApplication>
#include <QMenuBar>
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

    ui->menubar->installEventFilter(this);
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
    ui->checkBoxRubberBandOnMove->setChecked(helper->rubberBandOnMove());
    ui->checkBoxRubberBandOnResize->setChecked(helper->rubberBandOnResize());

    connect(ui->checkBoxWidgetMovable, &QCheckBox::clicked, this, [helper](bool checked) {
        helper->setWidgetMovable(checked);
    });
    connect(ui->checkBoxWidgetResizable, &QCheckBox::clicked, this, [this, helper](bool checked) {
        helper->setWidgetResizable(checked);
        ui->statusbar->setSizeGripEnabled(checked);
    });
    connect(ui->checkBoxRubberBandOnMove, &QCheckBox::clicked, this, [helper](bool checked) {
        helper->setRubberBandOnMove(checked);
    });
    connect(ui->checkBoxRubberBandOnResize, &QCheckBox::clicked, this, [helper](bool checked) {
        helper->setRubberBandOnResize(checked);
    });

    ui->spinBoxTitleHeight->setValue(helper->titleHeight());
    ui->spinBoxBorderWidth->setValue(helper->borderWidth());

    connect(ui->spinBoxTitleHeight, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, [helper](int value) {
        helper->setTitleHeight(value);
    });
    connect(ui->spinBoxBorderWidth, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, [helper](bool value) {
        helper->setBorderWidth(value);
    });


    setWindowTitle(tr("FramelessHelper Demo"));
    resize(800, 400);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
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

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == ui->menubar) {
        switch (e->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove:
        case QEvent::Leave:
        case QEvent::HoverMove:
        case QEvent::MouseButtonDblClick: {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QPoint pos = mouseEvent->globalPos();
#else
            QPoint pos = mouseEvent->position().toPoint();
#endif
            QWidget *child = QApplication::widgetAt(pos);
            if (child) {
            qDebug() << child->metaObject()->className();
            if (child == ui->menubar) {
                QApplication::sendEvent(this, e); // post to framelessHelper
            }
            }
        }
        default:
            break;
        }
    }
    return QMainWindow::eventFilter(obj, e);
}
