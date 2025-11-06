#ifndef _PARBAT_TOOLWINDOW_H
#define _PARBAT_TOOLWINDOW_H

#include <QDialog>
#include <QTabWidget>
#include <QFont>
#include <QBrush>
#include <QPen>
#include <QVector>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "ScrollBox.h"
#include "ToolTab.h"
#include "DisplayTab.h"
#include "QueryTab.h"
#include "ImageTab.h"
#include "FeatureTab.h"

class ToolWindow : public QDialog
{
    Q_OBJECT

public:
    ToolWindow(QWidget *parent = nullptr);
    virtual ~ToolWindow();

    virtual int Create(QWidget* parent = nullptr);
    
    void SetCursorPosition(int x, int y); // update cursor position on query tab
    void SetImageBandValue(int band, int value); // update image band value on query tab
    
    int getWidth() const;
    int getHeight() const;
    // 根据当前图像属性刷新波段数并通知各标签页
    void refreshBands();
    
    // Public members (for compatibility with existing code)
    QVector<ToolTab*> tabs;
    DisplayTab displayTab;                
    QueryTab queryTab;     
    ImageTab imageTab;   
    FeatureTab featureTab;                
    QFont boldFont, normalFont, headingFont;
    QBrush tabBrush;
    QWidget* toolWindowCurrentTabContainer;
    
    // 添加public访问权限的bands成员
    int bands;

private slots:
    void onTabChanged(int index);

private:
    void setupUI();
    void setupTabs();
    
    QTabWidget *tabWidget;
    QPen tabPen;
    QWidget *toolWindowImageTabHeading;
    QWidget *toolWindowQueryTabHeading;
    QWidget *toolWindowScrollBar;
    
    static const int WIDTH = 250;
    static const int HEIGHT = 340;
};

#endif