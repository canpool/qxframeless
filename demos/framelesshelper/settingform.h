#ifndef SETTINGFORM_H
#define SETTINGFORM_H

#include <QWidget>

namespace Ui {
class SettingForm;
}

class SettingForm : public QWidget
{
    Q_OBJECT

public:
    explicit SettingForm(QWidget *parent = nullptr);
    ~SettingForm();

private slots:
    void on_checkBoxWidgetResizable_clicked(bool checked);
    void on_checkBoxWidgetMovable_clicked(bool checked);
    void on_checkBoxRubberBandOnMove_clicked(bool checked);
    void on_checkBoxRubberBandOnResize_clicked(bool checked);
    void on_spinBoxTitleHeight_valueChanged(int arg1);
    void on_spinBoxBorderWidth_valueChanged(int arg1);

private:
    Ui::SettingForm *ui;
};

#endif // SETTINGFORM_H
