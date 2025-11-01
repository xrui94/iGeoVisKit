#include "PchApp.h"
#include "ROIWindow.h"

#include "ImageWindow.h"
#include "OverviewWindow.h"
#include "utils/Settings.h"
#include "imagery/ROISet.h"
#include "imagery/ROI.h"

// Qt includes
#include <QApplication>
#include <QDialog>
#include <QWidget>
#include <QListWidget>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QColor>
#include <QVector>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QColorDialog>

extern ROISet *regionsSet;

QString* ROIWindow::editingROIName = nullptr;

ROIWindow::ROIWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("ROI Window"));
    resize(WIDTH, HEIGHT);
    
    setupUI();
    setupActions();
    setupToolBars();
}

ROIWindow::~ROIWindow()
{
}

int ROIWindow::Create(QWidget *parent)
{
	if (parent) {
		setParent(parent);
		// 作为工具型窗口，以保持与原始弹出工具窗一致的行为
		setWindowFlags(windowFlags() | Qt::Tool);
	}
	show();
	return true;
}

void ROIWindow::setupUI()
{
    // 顶层布局，包含工具栏与主体
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 工具栏（作为普通控件加入布局）
    roiToolBar = new QToolBar(QStringLiteral("ROI Tools"), this);
    layout->addWidget(roiToolBar);
    
    // 创建中央容器
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *centerLayout = new QVBoxLayout(centralWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    
    // 创建ROI列表
    roiListWidget = new QListWidget(this);
    centerLayout->addWidget(roiListWidget);
    layout->addWidget(centralWidget);
    
    // 连接列表项变化信号
    connect(roiListWidget, &QListWidget::itemSelectionChanged, this, &ROIWindow::onListItemChanged);
}

void ROIWindow::setupActions()
{
    // 创建动作
    openAction = new QAction(QStringLiteral("Open ROI"), this);
    connect(openAction, &QAction::triggered, this, &ROIWindow::onOpenROI);
    
    saveAction = new QAction(QStringLiteral("Save ROI"), this);
    connect(saveAction, &QAction::triggered, this, &ROIWindow::onSaveROI);
    
    newAction = new QAction(QStringLiteral("New ROI"), this);
    connect(newAction, &QAction::triggered, this, &ROIWindow::onNewROI);
    
    deleteAction = new QAction(QStringLiteral("Delete ROI"), this);
    connect(deleteAction, &QAction::triggered, this, &ROIWindow::onDeleteROI);
    
    polyAction = new QAction(QStringLiteral("Polygon Selection"), this);
    connect(polyAction, &QAction::triggered, this, &ROIWindow::onPolySelection);
    
    rectAction = new QAction(QStringLiteral("Rectangle Selection"), this);
    connect(rectAction, &QAction::triggered, this, &ROIWindow::onRectSelection);
    
    singlePointAction = new QAction(QStringLiteral("Single Point Selection"), this);
    connect(singlePointAction, &QAction::triggered, this, &ROIWindow::onSinglePointSelection);
}

void ROIWindow::setupToolBars()
{
    // 将动作添加到已创建的工具栏
    roiToolBar->addAction(openAction);
    roiToolBar->addAction(saveAction);
    roiToolBar->addAction(newAction);
    roiToolBar->addAction(deleteAction);
    roiToolBar->addSeparator();
    roiToolBar->addAction(polyAction);
    roiToolBar->addAction(rectAction);
    roiToolBar->addAction(singlePointAction);
}

void ROIWindow::deleteAllROI()
{
    // 删除所有ROI
    if (regionsSet) {
        regionsSet->delete_all_regions();
    }
    
    // 清空列表
    if (roiListWidget) {
        roiListWidget->clear();
    }
    
    // 清空颜色按钮列表
    roiColourButtonList.clear();
}

void ROIWindow::newEntity(ROIWindow* window, const char* type)
{
    Q_UNUSED(window);
    Q_UNUSED(type);
    // 创建新实体的实现
}

void ROIWindow::updateButtons(ROIWindow* window)
{
    Q_UNUSED(window);
    // 更新按钮状态的实现
}

int ROIWindow::getWidth() const
{
    return WIDTH;
}

int ROIWindow::getHeight() const
{
    return HEIGHT;
}

void ROIWindow::onOpenROI()
{
    // 打开ROI文件
    QString fileName = QFileDialog::getOpenFileName(this,
        QStringLiteral("Open ROI File"), QString(),
        QStringLiteral("ROI Files (*.roi);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        loadROI(this);
    }
}

void ROIWindow::onSaveROI()
{
    // 保存ROI文件
    QString fileName = QFileDialog::getSaveFileName(this,
        QStringLiteral("Save ROI File"), QString(),
        QStringLiteral("ROI Files (*.roi);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        saveROI(this);
    }
}

void ROIWindow::onNewROI()
{
    // 创建新ROI
    newROI(this, nullptr);
}

void ROIWindow::onDeleteROI()
{
    // 删除选中的ROI
    deleteROI(this);
}

void ROIWindow::onPolySelection()
{
    // 多边形选择工具
}

void ROIWindow::onRectSelection()
{
    // 矩形选择工具
}

void ROIWindow::onSinglePointSelection()
{
    // 单点选择工具
}

void ROIWindow::onListItemChanged()
{
    // 列表项变化处理
}

void ROIWindow::onChangeROIColour()
{
    // 改变ROI颜色
}

int ROIWindow::getROICheckedCount()
{
    // 返回选中的ROI复选框数量
    return 0;
}

int ROIWindow::getSelectedItemIndex()
{
    // 返回选中项的索引
    if (roiListWidget && roiListWidget->currentRow() >= 0) {
        return roiListWidget->currentRow();
    }
    return -1;
}

QString ROIWindow::getItemText(int i)
{
    // 返回指定项的文本
    if (roiListWidget && i >= 0 && i < roiListWidget->count()) {
        return roiListWidget->item(i)->text();
    }
    return QString();
}

QString ROIWindow::getSelectedItemText()
{
    // 返回选中项的文本
    if (roiListWidget && roiListWidget->currentItem()) {
        return roiListWidget->currentItem()->text();
    }
    return QString();
}

void ROIWindow::newROI(ROIWindow* window, const char* type)
{
    Q_UNUSED(window);
    Q_UNUSED(type);
    // 创建新ROI的实现
}

void ROIWindow::loadROI(ROIWindow* window)
{
    Q_UNUSED(window);
    // 加载ROI的实现
}

void ROIWindow::saveROI(ROIWindow* window)
{
    Q_UNUSED(window);
    // 保存ROI的实现
}

void ROIWindow::deleteROI(ROIWindow* window)
{
    Q_UNUSED(window);
    // 删除ROI的实现
}

void ROIWindow::updateROIList(ROIWindow* window)
{
    Q_UNUSED(window);
    // 更新ROI列表的实现
}

void ROIWindow::addNewRoiToList(ROI *rCur, int newId)
{
    Q_UNUSED(rCur);
    Q_UNUSED(newId);
    // 添加新ROI到列表的实现
}