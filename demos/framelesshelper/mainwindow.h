#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonMinimize_clicked();
    void on_pushButtonClose_clicked();
    void on_pushButtonMaximize_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
