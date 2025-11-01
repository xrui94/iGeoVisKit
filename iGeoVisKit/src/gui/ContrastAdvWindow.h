#ifndef _PARBAT_CONTSADVWINDOW_H
#define _PARBAT_CONTSADVWINDOW_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>

class ContrastAdvWindow : public QDialog
{
    Q_OBJECT

public:
    ContrastAdvWindow(QWidget *parent = nullptr);
    virtual ~ContrastAdvWindow();

    int Create(QWidget *parent = nullptr);

private slots:
    void onOKClicked();
    void onCancelClicked();
    void onRedBrightnessChanged(int value);
    void onRedContrastChanged(int value);
    void onGreenBrightnessChanged(int value);
    void onGreenContrastChanged(int value);
    void onBlueBrightnessChanged(int value);
    void onBlueContrastChanged(int value);

private:
    void setupUI();
    
    // 红色通道控件
    QSlider *redBrightnessSlider;
    QSlider *redContrastSlider;
    QLabel *redBrightnessLabel;
    QLabel *redContrastLabel;
    
    // 绿色通道控件
    QSlider *greenBrightnessSlider;
    QSlider *greenContrastSlider;
    QLabel *greenBrightnessLabel;
    QLabel *greenContrastLabel;
    
    // 蓝色通道控件
    QSlider *blueBrightnessSlider;
    QSlider *blueContrastSlider;
    QLabel *blueBrightnessLabel;
    QLabel *blueContrastLabel;
    
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif