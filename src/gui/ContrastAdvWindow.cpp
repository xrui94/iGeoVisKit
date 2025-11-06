#include "PchApp.h"

#include "ContrastAdvWindow.h"

#include "utils/Settings.h"

// Qt includes
#include <QApplication>
#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>

ContrastAdvWindow::ContrastAdvWindow(QWidget *parent)
    : QDialog(parent)
    , redBrightnessSlider(nullptr)
    , redContrastSlider(nullptr)
    , redBrightnessLabel(nullptr)
    , redContrastLabel(nullptr)
    , greenBrightnessSlider(nullptr)
    , greenContrastSlider(nullptr)
    , greenBrightnessLabel(nullptr)
    , greenContrastLabel(nullptr)
    , blueBrightnessSlider(nullptr)
    , blueContrastSlider(nullptr)
    , blueBrightnessLabel(nullptr)
    , blueContrastLabel(nullptr)
    , okButton(nullptr)
    , cancelButton(nullptr)
{
    setWindowTitle(QStringLiteral("Advanced Contrast Stretch"));
    resize(330, 570);
    
    setupUI();
}

ContrastAdvWindow::~ContrastAdvWindow()
{
}

int ContrastAdvWindow::Create(QWidget *parent)
{
	if (parent) {
		setParent(parent);
		setWindowFlags(windowFlags() | Qt::Dialog);
	}
	show();
	return true;
}

void ContrastAdvWindow::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 红色通道组
    QGroupBox *redGroup = new QGroupBox(QStringLiteral("Red Channel"), this);
    QVBoxLayout *redLayout = new QVBoxLayout(redGroup);
    
    redBrightnessLabel = new QLabel(QStringLiteral("Red Brightness"), this);
    redLayout->addWidget(redBrightnessLabel);
    
    redBrightnessSlider = new QSlider(Qt::Horizontal, this);
    redBrightnessSlider->setRange(1, 255);
    redBrightnessSlider->setTickPosition(QSlider::TicksBelow);
    redBrightnessSlider->setTickInterval(128);
    redBrightnessSlider->setPageStep(1);
    redBrightnessSlider->setValue(129);
    redLayout->addWidget(redBrightnessSlider);
    
    QLabel *redBrightnessValues = new QLabel(QStringLiteral("0                                             50                                         100"), this);
    redLayout->addWidget(redBrightnessValues);
    
    redLayout->addSpacing(10);
    
    redContrastLabel = new QLabel(QStringLiteral("Red Contrast"), this);
    redLayout->addWidget(redContrastLabel);
    
    redContrastSlider = new QSlider(Qt::Horizontal, this);
    redContrastSlider->setRange(1, 255);
    redContrastSlider->setTickPosition(QSlider::TicksBelow);
    redContrastSlider->setTickInterval(128);
    redContrastSlider->setPageStep(1);
    redContrastSlider->setValue(129);
    redLayout->addWidget(redContrastSlider);
    
    QLabel *redContrastValues = new QLabel(QStringLiteral("0                                             50                                         100"), this);
    redLayout->addWidget(redContrastValues);
    
    mainLayout->addWidget(redGroup);
    
    // 绿色通道组
    QGroupBox *greenGroup = new QGroupBox(QStringLiteral("Green Channel"), this);
    QVBoxLayout *greenLayout = new QVBoxLayout(greenGroup);
    
    greenBrightnessLabel = new QLabel(QStringLiteral("Green Brightness"), this);
    greenLayout->addWidget(greenBrightnessLabel);
    
    greenBrightnessSlider = new QSlider(Qt::Horizontal, this);
    greenBrightnessSlider->setRange(1, 255);
    greenBrightnessSlider->setTickPosition(QSlider::TicksBelow);
    greenBrightnessSlider->setTickInterval(128);
    greenBrightnessSlider->setPageStep(1);
    greenBrightnessSlider->setValue(129);
    greenLayout->addWidget(greenBrightnessSlider);
    
    QLabel *greenBrightnessValues = new QLabel(QStringLiteral("0                                             50                                         100"), this);
    greenLayout->addWidget(greenBrightnessValues);
    
    greenLayout->addSpacing(10);
    
    greenContrastLabel = new QLabel(QStringLiteral("Green Contrast"), this);
    greenLayout->addWidget(greenContrastLabel);
    
    greenContrastSlider = new QSlider(Qt::Horizontal, this);
    greenContrastSlider->setRange(1, 255);
    greenContrastSlider->setTickPosition(QSlider::TicksBelow);
    greenContrastSlider->setTickInterval(128);
    greenContrastSlider->setPageStep(1);
    greenContrastSlider->setValue(129);
    greenLayout->addWidget(greenContrastSlider);
    
    QLabel *greenContrastValues = new QLabel(QStringLiteral("0                                             50                                         100"), this);
    greenLayout->addWidget(greenContrastValues);
    
    mainLayout->addWidget(greenGroup);
    
    // 蓝色通道组
    QGroupBox *blueGroup = new QGroupBox(QStringLiteral("Blue Channel"), this);
    QVBoxLayout *blueLayout = new QVBoxLayout(blueGroup);
    
    blueBrightnessLabel = new QLabel(QStringLiteral("Blue Brightness"), this);
    blueLayout->addWidget(blueBrightnessLabel);
    
    blueBrightnessSlider = new QSlider(Qt::Horizontal, this);
    blueBrightnessSlider->setRange(1, 255);
    blueBrightnessSlider->setTickPosition(QSlider::TicksBelow);
    blueBrightnessSlider->setTickInterval(128);
    blueBrightnessSlider->setPageStep(1);
    blueBrightnessSlider->setValue(129);
    blueLayout->addWidget(blueBrightnessSlider);
    
    QLabel *blueBrightnessValues = new QLabel(QStringLiteral("0                                             50                                         100"), this);
    blueLayout->addWidget(blueBrightnessValues);
    
    blueLayout->addSpacing(10);
    
    blueContrastLabel = new QLabel(QStringLiteral("Blue Contrast"), this);
    blueLayout->addWidget(blueContrastLabel);
    
    blueContrastSlider = new QSlider(Qt::Horizontal, this);
    blueContrastSlider->setRange(1, 255);
    blueContrastSlider->setTickPosition(QSlider::TicksBelow);
    blueContrastSlider->setTickInterval(128);
    blueContrastSlider->setPageStep(1);
    blueContrastSlider->setValue(129);
    blueLayout->addWidget(blueContrastSlider);
    
    QLabel *blueContrastValues = new QLabel(QStringLiteral("0                                             50                                         100"), this);
    blueLayout->addWidget(blueContrastValues);
    
    mainLayout->addWidget(blueGroup);
    
    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QPushButton *previewButton = new QPushButton(QStringLiteral("Preview"), this);
    buttonLayout->addWidget(previewButton);
    
    buttonLayout->addStretch();
    
    okButton = new QPushButton(QStringLiteral("OK"), this);
    cancelButton = new QPushButton(QStringLiteral("Cancel"), this);
    
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(okButton, &QPushButton::clicked, this, &ContrastAdvWindow::onOKClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ContrastAdvWindow::onCancelClicked);
    connect(redBrightnessSlider, &QSlider::valueChanged, this, &ContrastAdvWindow::onRedBrightnessChanged);
    connect(redContrastSlider, &QSlider::valueChanged, this, &ContrastAdvWindow::onRedContrastChanged);
    connect(greenBrightnessSlider, &QSlider::valueChanged, this, &ContrastAdvWindow::onGreenBrightnessChanged);
    connect(greenContrastSlider, &QSlider::valueChanged, this, &ContrastAdvWindow::onGreenContrastChanged);
    connect(blueBrightnessSlider, &QSlider::valueChanged, this, &ContrastAdvWindow::onBlueBrightnessChanged);
    connect(blueContrastSlider, &QSlider::valueChanged, this, &ContrastAdvWindow::onBlueContrastChanged);
}

void ContrastAdvWindow::onOKClicked()
{
    // 处理OK按钮点击
    accept();
}

void ContrastAdvWindow::onCancelClicked()
{
    // 处理取消按钮点击
    reject();
}

void ContrastAdvWindow::onRedBrightnessChanged(int value)
{
    Q_UNUSED(value);
    // 处理红色亮度变化
}

void ContrastAdvWindow::onRedContrastChanged(int value)
{
    Q_UNUSED(value);
    // 处理红色对比度变化
}

void ContrastAdvWindow::onGreenBrightnessChanged(int value)
{
    Q_UNUSED(value);
    // 处理绿色亮度变化
}

void ContrastAdvWindow::onGreenContrastChanged(int value)
{
    Q_UNUSED(value);
    // 处理绿色对比度变化
}

void ContrastAdvWindow::onBlueBrightnessChanged(int value)
{
    Q_UNUSED(value);
    // 处理蓝色亮度变化
}

void ContrastAdvWindow::onBlueContrastChanged(int value)
{
    Q_UNUSED(value);
    // 处理蓝色对比度变化
}

/* ------------------------------------------------------------------------------------------------------------------------ */
/* Prefs Window Functions */


/* create main window */
