#include "PchApp.h"
#include "ToolTab.h"

#include "config.h"
#include "ScrollBox.h"
#include "ToolWindow.h"

// Qt includes
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

// create tool tab container
int ToolTab::Create(QWidget *parent, QRect *parentRect)
{
    Q_UNUSED(parent);
    Q_UNUSED(parentRect);
    
    // 在Qt中，我们不需要手动创建窗口和控件，而是在setupUI中完成
    
    return 1;
}

void ToolTab::setupUI()
{
    // 创建主布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // 创建标题标签
    headingWidget = new QLabel(GetTabHeading(), this);
    headingWidget->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(headingWidget);
    
    // 设置背景色
    //setStyleSheet("background-color: lightgray;");
}