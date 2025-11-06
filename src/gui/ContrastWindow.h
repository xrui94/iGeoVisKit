#ifndef _PARBAT_CONTSWINDOW_H
#define _PARBAT_CONTSWINDOW_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QCheckBox>

class ContrastWindow : public QDialog
{
    Q_OBJECT

public:
    ContrastWindow(QWidget *parent = nullptr);
    virtual ~ContrastWindow();

    int Create(QWidget *parent = nullptr);
    void resetSliders();
    
    // 添加缺少的成员函数
    // void Show();  // 移除Windows特定函数
    // void Hide();  // 移除Windows特定函数

private slots:
    void onOKClicked();
    void onCancelClicked();
    void onBrightnessChanged(int value);
    void onContrastChanged(int value);
    // 添加onPreviewChanged函数声明
    void onPreviewChanged(int state);

private:
    void setupUI();
    
    QSlider *brightnessSlider;
    QSlider *contrastSlider;
    QLabel *brightnessLabel;
    QLabel *contrastLabel;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QCheckBox *previewCheckBox;  // 添加预览复选框
    
    // 添加缺少的成员变量
    // HWND hBrightnessTrackbar;  // 移除Windows特定成员
    // HWND hContrastTrackbar;  // 移除Windows特定成员
    // HWND hPreview;  // 移除Windows特定成员
    
    int brightnessCancel;
    int contrastCancel;
    
    // 移除Windows特定的函数
    // static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
};

#endif