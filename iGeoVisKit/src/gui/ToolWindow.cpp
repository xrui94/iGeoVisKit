#include "PchApp.h"
#include "ToolWindow.h"

#include "config.h"
#include "MainWindow.h"
extern MainWindow* mainWindow;
#include "OverviewWindow.h"
#include "ImageWindow.h"
#include "utils/Console.h"
#include "ToolTab.h"
#include "DisplayTab.h"
#include "QueryTab.h"
#include "ImageTab.h"
#include "FeatureTab.h"

// Qt includes
#include <QApplication>
#include <QDialog>
#include <QTabWidget>
#include <QFont>
#include <QBrush>
#include <QPen>
#include <QVector>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QString>
#include <QSizePolicy>

ToolWindow::ToolWindow(QWidget *parent)
    : QDialog(parent)
    , tabWidget(nullptr)
    , toolWindowImageTabHeading(nullptr)
    , toolWindowQueryTabHeading(nullptr)
    , toolWindowScrollBar(nullptr)
    , toolWindowCurrentTabContainer(nullptr)
    , bands(0)
{
    setWindowTitle(QStringLiteral("Tools Window"));
    resize(WIDTH, HEIGHT);
    
    setupUI();
    setupTabs();
}

ToolWindow::~ToolWindow()
{
}

// create tool window
int ToolWindow::Create(QWidget* parent) {
Q_UNUSED(parent);
// 作为工具型窗口，不在任务栏显示
setWindowFlags(windowFlags() | Qt::Tool);
show();
return true;
}

void ToolWindow::setupUI()
{
    // 创建布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 创建标签页控件
    tabWidget = new QTabWidget(this);
    layout->addWidget(tabWidget);
    
    // 连接标签页切换信号
    connect(tabWidget, &QTabWidget::currentChanged, this, &ToolWindow::onTabChanged);
    
    // 设置字体
    boldFont = QFont("Tahoma", 8, QFont::Bold);
    normalFont = QFont("Tahoma", 8);
    headingFont = QFont("Tahoma", 9, QFont::Bold);
    
    // 设置画笔和画刷
    tabPen = QPen(QColor(Qt::lightGray));
    tabBrush = QBrush(QColor(Qt::lightGray));
}

void ToolWindow::setupTabs()
{
    // 添加标签页到列表
    tabs.append(&displayTab);
    tabs.append(&queryTab);
    tabs.append(&imageTab);  
    tabs.append(&featureTab);      
  
    // 从 MainWindow 管理的 ImageHandler 获取波段数（+1含 NONE）
    if (mainWindow && mainWindow->imageHandler()) {
        bands = mainWindow->imageHandler()->get_image_properties()->getNumBands() + 1;
    }

    // 创建并渲染各标签页
    for (int i = 0; i < tabs.size(); i++) {
        ToolTab *tab = tabs.at(i);
        const char* tabNamePtr = tab->GetTabName();
        QString tabName = tabNamePtr ? QString::fromUtf8(tabNamePtr) : QStringLiteral("Tab");

        // 将具体标签页加入 QTabWidget，并让其自行构建内容
        tab->setParent(tabWidget);
        tab->setToolWindow(this);
        tab->setupUI();
        tabWidget->addTab(tab, tabName);
    }
}

void ToolWindow::refreshBands()
{
    // 从 MainWindow 管理的 ImageHandler 获取波段数（+1含 NONE）
    if (mainWindow && mainWindow->imageHandler() && mainWindow->imageHandler()->get_image_properties()) {
        bands = mainWindow->imageHandler()->get_image_properties()->getNumBands() + 1;
    } else {
        bands = 1; // 仅 NONE
    }
    // 通知 DisplayTab 刷新下拉框
    displayTab.refreshBands(bands);
}

void ToolWindow::onTabChanged(int index)
{
    Q_UNUSED(index);
    // 处理标签页切换事件
    if (index >= 0 && index < tabs.size()) {
        toolWindowCurrentTabContainer = tabs.at(index);
    }
}

// update cursor position on query tab
void ToolWindow::SetCursorPosition(int x, int y)
{
    // 在Qt中更新查询标签页的光标位置
    // 这里需要具体实现与查询标签页的交互
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void ToolWindow::SetImageBandValue(int band, int value)
{
    // 在Qt中更新图像波段值
    // 这里需要具体实现与图像标签页的交互
    Q_UNUSED(band);
    Q_UNUSED(value);
}

int ToolWindow::getWidth() const
{
    return WIDTH;
}

int ToolWindow::getHeight() const
{
    return HEIGHT;
}

