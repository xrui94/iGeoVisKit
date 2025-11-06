#include "PchApp.h"

#include "QueryTab.h"

// #include "Window.h"  // 移除对Window.h的依赖
#include "ScrollBox.h"
#include "ToolTab.h"
#include "config.h"
#include "ToolWindow.h"

// Qt includes
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QScrollArea>


int QueryTab::GetContainerHeight()
{
    //return 80 + (20 * (toolWindow->bands+1));
    return 0;
}

// int QueryTab::Create(HWND parent,RECT *parentRect)
int QueryTab::Create(QWidget *parent, QRect *parentRect)
{
    Q_UNUSED(parent);
    Q_UNUSED(parentRect);
    
    // HWND hstatic;  // 移除Windows特定变量
    
    // ToolTab::Create(parent,parentRect);  // 修改为Qt方式
    
    // RECT rect;  // 移除Windows特定变量
    // rect.top=5;
    // rect.left=25;
    // rect.right=218;
    // rect.bottom=200;                 
    // hQScrollBox.Create(GetHandle(),&rect);  // 修改为Qt方式

    /* Create container for band values */
    // HWND queryValueContainer = CreateWindowEx(0, "BUTTON", "Values",
    // 	WS_CHILD | BS_GROUPBOX | WS_VISIBLE, 118, 5, 66, 20 + (20 * (toolWindow.bands-1)),
    // 	hQScrollBox.GetHandle(), NULL, Window::GetAppInstance(), NULL);
    // SetFont(queryValueContainer,FONT_BOLD);
    
    /* Dynamically add image band values */
    int bands = 0;
    ToolWindow* tw = qobject_cast<ToolWindow*>(parent);
    if (tw) bands = tw->bands;
    imageBandValues = new QLabel*[bands];
    
    // 在Qt中创建UI组件
    setupUI();
    
    return 1;
}

void QueryTab::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建滚动区域
    QScrollArea *scrollArea = new QScrollArea(this);
    QWidget *scrollWidget = new QWidget(scrollArea);
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
    
    // 创建值容器组框
    QGroupBox *queryValueContainer = new QGroupBox(QStringLiteral("Values"), this);
    QVBoxLayout *valuesLayout = new QVBoxLayout(queryValueContainer);
    
    // 动态添加图像波段值
    int bands = 0;
    ToolWindow* tw = qobject_cast<ToolWindow*>(parentWidget());
    if (tw) bands = tw->bands;
    for (int i=1; i<bands; i++)  
    {
        // 添加波段名称
        QString name = QStringLiteral("Band %1").arg(i);
        QLabel *bandLabel = new QLabel(name, this);
        scrollLayout->addWidget(bandLabel);
        
        // 添加波段值到值容器
        QString tempBandValue = "0"; // 临时存储波段值
        imageBandValues[i] = new QLabel(tempBandValue, this);
        valuesLayout->addWidget(imageBandValues[i]);
    }
    
    scrollLayout->addWidget(queryValueContainer);
    
    // 显示光标位置
    QLabel *xLabel = new QLabel("X", this);
    QLabel *yLabel = new QLabel("Y", this);
    cursorXPos = new QLabel("-", this);
    cursorYPos = new QLabel("-", this);
    
    scrollLayout->addWidget(xLabel);
    scrollLayout->addWidget(cursorXPos);
    scrollLayout->addWidget(yLabel);
    scrollLayout->addWidget(cursorYPos);
    
    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea);
}

void QueryTab::SetImageBandValue(int i,char *text)
{
    // SetWindowText(imageBandValues[i],text);  // 移除Windows特定函数
    if (imageBandValues[i]) {
        imageBandValues[i]->setText(text);
    }
}

void QueryTab::SetCursorX(char *text)
{
    // SetWindowText(cursorXPos,text);  // 移除Windows特定函数
    if (cursorXPos) {
        cursorXPos->setText(text);
    }
}

void QueryTab::SetCursorY(char *text)
{
    // SetWindowText(cursorYPos,text);  // 移除Windows特定函数
    if (cursorYPos) {
        cursorYPos->setText(text);
    }
}