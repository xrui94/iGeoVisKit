#include "PchApp.h"

#include "ContrastWindow.h"

#include "utils/Settings.h"
#include "OverviewWindow.h"

// Qt includes
#include <QApplication>
#include <QDialog>
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>

ContrastWindow::ContrastWindow(QWidget *parent)
    : QDialog(parent)
    , brightnessSlider(nullptr)
    , contrastSlider(nullptr)
    , brightnessLabel(nullptr)
    , contrastLabel(nullptr)
    , okButton(nullptr)
    , cancelButton(nullptr)
    , previewCheckBox(nullptr)
    , brightnessCancel(250)
    , contrastCancel(250)
{
    setWindowTitle(QStringLiteral("Contrast Stretch"));
    resize(330, 250);
    
    setupUI();
}

ContrastWindow::~ContrastWindow()
{
}

/* create main window */
int ContrastWindow::Create(QWidget *parent)
{
    Q_UNUSED(parent);
    
    // Use ImageHandler to get current values;
    int previous_brightness = 250;
    int previous_contrast = 250;
    // assert(image_handler != NULL);
    // if(image_handler != NULL) {
    //     image_handler->get_brightness_contrast(&previous_brightness, &previous_contrast);
    // }
   
    show();
    return true;
}

void ContrastWindow::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 亮度控制组
    QGroupBox *brightnessGroup = new QGroupBox(QStringLiteral("Brightness"), this);
    QVBoxLayout *brightnessLayout = new QVBoxLayout(brightnessGroup);
    
    brightnessSlider = new QSlider(Qt::Horizontal, this);
    brightnessSlider->setRange(1, 500);
    brightnessSlider->setTickPosition(QSlider::TicksBelow);
    brightnessSlider->setTickInterval(25);
    brightnessSlider->setPageStep(1);
    brightnessSlider->setValue(brightnessCancel);
    
    brightnessLabel = new QLabel(QStringLiteral("0                                             50                                         100"), this);
    
    brightnessLayout->addWidget(brightnessSlider);
    brightnessLayout->addWidget(brightnessLabel);
    
    // 对比度控制组
    QGroupBox *contrastGroup = new QGroupBox(QStringLiteral("Contrast"), this);
    QVBoxLayout *contrastLayout = new QVBoxLayout(contrastGroup);
    
    contrastSlider = new QSlider(Qt::Horizontal, this);
    contrastSlider->setRange(1, 500);
    contrastSlider->setTickPosition(QSlider::TicksBelow);
    contrastSlider->setTickInterval(25);
    contrastSlider->setPageStep(1);
    contrastSlider->setValue(contrastCancel);
    
    contrastLabel = new QLabel(QStringLiteral("0                                             50                                         100"), this);
    
    contrastLayout->addWidget(contrastSlider);
    contrastLayout->addWidget(contrastLabel);
    
    // 预览复选框
    previewCheckBox = new QCheckBox(QStringLiteral("Preview"), this);
    
    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    buttonLayout->addWidget(previewCheckBox);
    buttonLayout->addStretch();
    
    okButton = new QPushButton(QStringLiteral("OK"), this);
    cancelButton = new QPushButton(QStringLiteral("Cancel"), this);
    
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    // 添加到主布局
    mainLayout->addWidget(brightnessGroup);
    mainLayout->addWidget(contrastGroup);
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(okButton, &QPushButton::clicked, this, &ContrastWindow::onOKClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ContrastWindow::onCancelClicked);
    connect(brightnessSlider, &QSlider::valueChanged, this, &ContrastWindow::onBrightnessChanged);
    connect(contrastSlider, &QSlider::valueChanged, this, &ContrastWindow::onContrastChanged);
    connect(previewCheckBox, &QCheckBox::stateChanged, this, &ContrastWindow::onPreviewChanged);
}

void ContrastWindow::resetSliders()
{
    //reset slider positions
    if (brightnessSlider) {
        brightnessSlider->setValue(brightnessCancel);
    }
    if (contrastSlider) {
        contrastSlider->setValue(contrastCancel);
    }
}

void ContrastWindow::onOKClicked()
{
    // 处理OK按钮点击
    accept();
}

void ContrastWindow::onCancelClicked()
{
    // 处理取消按钮点击
    resetSliders();
    reject();
}

void ContrastWindow::onBrightnessChanged(int value)
{
    Q_UNUSED(value);
    // 处理亮度变化
    // 如果预览复选框被选中，则实时更新图像
    if (previewCheckBox && previewCheckBox->isChecked()) {
        // assert(image_handler != NULL);
        // if (image_handler != NULL) {
        //     image_handler->set_brightness_contrast(value, contrastSlider->value());
        // }
    }
}

void ContrastWindow::onContrastChanged(int value)
{
    Q_UNUSED(value);
    // 处理对比度变化
    // 如果预览复选框被选中，则实时更新图像
    if (previewCheckBox && previewCheckBox->isChecked()) {
        // assert(image_handler != NULL);
        // if (image_handler != NULL) {
        //     image_handler->set_brightness_contrast(brightnessSlider->value(), value);
        // }
    }
}

void ContrastWindow::onPreviewChanged(int state)
{
    Q_UNUSED(state);
    // 处理预览复选框状态变化
    // if (state == Qt::Checked) {
    //     // 预览被选中，更新图像
    //     // assert(image_handler != NULL);
    //     // if (image_handler != NULL) {
    //     //     image_handler->set_brightness_contrast(brightnessSlider->value(), contrastSlider->value());
    //     // }
    // } else {
    //     // 预览被取消，恢复原始值
    //     // assert(image_handler != NULL);
    //     // if (image_handler != NULL) {
    //     //     image_handler->set_brightness_contrast(brightnessCancel, contrastCancel);
    //     // }
    // }
}

// 移除Windows特定的WindowProcedure函数
// LRESULT CALLBACK ContrastWindow::WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// {   
//     // ... 原来的Windows特定代码 ...
// }