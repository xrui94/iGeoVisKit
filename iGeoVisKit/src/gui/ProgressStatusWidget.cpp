#include "PchApp.h"
#include "ProgressStatusWidget.h"


// Qt includes
#include <QApplication>
#include <QProgressBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>

ProgressStatusWidget::ProgressStatusWidget(QWidget *parent)
    : QWidget(parent)
    , progressBar(nullptr)
    , statusLabel(nullptr)
    , autoTimer(nullptr)
    , current_steps(0)
    , total_steps(0)
    , start_count(0)
    , loop_at_end(false)
    , autoIncrement(false)
{
    // 状态栏常驻控件：默认隐藏，右侧显示
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setVisible(false);
    
    setupUI();
    init();
}

ProgressStatusWidget::~ProgressStatusWidget()
{
}

void ProgressStatusWidget::setupUI()
{
    // 水平布局：标签 + 进度条（紧凑显示在状态栏右侧）
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 0, 4, 0);
    layout->setSpacing(6);

    statusLabel = new QLabel(QStringLiteral("Processing"), this);
    progressBar = new QProgressBar(this);
    progressBar->setFixedWidth(160);

    layout->addWidget(statusLabel);
    layout->addWidget(progressBar);
}

void ProgressStatusWidget::init()
{
    // 初始化
}

void ProgressStatusWidget::start(int steps, bool auto_increment)
{
    total_steps = steps;
    current_steps = 0;
    autoIncrement = auto_increment;
    
    if (progressBar) {
        progressBar->setRange(0, total_steps);
        progressBar->setValue(0);
    }
    
    if (statusLabel) {
        statusLabel->setText(QStringLiteral("Processing..."));
    }

    setVisible(true);

    if (auto_increment) {
        if (!autoTimer) autoTimer = new QTimer(this);
        connect(autoTimer, &QTimer::timeout, this, &ProgressStatusWidget::autoIncrementProgress);
        autoTimer->start(100); // 每100毫秒更新一次
    }
}

void ProgressStatusWidget::end()
{
    current_steps = total_steps;
    
    if (progressBar) {
        progressBar->setValue(total_steps);
    }
    
    if (statusLabel) {
        statusLabel->setText(QStringLiteral("Completed"));
    }

    if (autoTimer) {
        autoTimer->stop();
    }
    setVisible(false);
}

void ProgressStatusWidget::reset()
{
    current_steps = 0;
    
    if (progressBar) {
        progressBar->setValue(0);
    }
    
    if (statusLabel) {
        statusLabel->setText(QStringLiteral("Processing..."));
    }
}

void ProgressStatusWidget::reset(int steps)
{
    total_steps = steps;
    reset();
    
    if (progressBar) {
        progressBar->setRange(0, total_steps);
    }
}

void ProgressStatusWidget::increment()
{
    increment(1);
}

void ProgressStatusWidget::increment(int steps)
{
    current_steps += steps;
    
    if (current_steps >= total_steps && loop_at_end) {
        current_steps = 0;
    }
    
    if (progressBar) {
        progressBar->setValue(current_steps);
    }
}

void ProgressStatusWidget::autoIncrementProgress()
{
    if (autoIncrement && current_steps < total_steps) {
        increment();
    }
}