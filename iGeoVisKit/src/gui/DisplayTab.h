#ifndef _PARBAT_DISPLAYTAB_H
#define _PARBAT_DISPLAYTAB_H

#include "PchApp.h"
#include "ToolTab.h"

// Qt includes
#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QRect>

class DisplayTab:public ToolTab
{
    Q_OBJECT

    private:
        // Qt 组件成员
        QComboBox *comboR;
        QComboBox *comboG;
        QComboBox *comboB;
        QPushButton *updateButton;
    public:
        const char* GetTabName() {return "Display";};
        const char* GetTabHeading() {return "Channel Selection";};
        int GetContainerHeight();   
        // int Create(HWND parent,RECT *parentRect);  // 淇敼鍑芥暟绛惧悕
        int Create(QWidget *parent, QRect *parentRect);  // 淇敼鍑芥暟绛惧悕
        // static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);  // 绉婚櫎Windows鐗瑰畾鍑芥暟
        
        // 娣诲姞setupUI鍑芥暟澹版槑
        void setupUI();
        // 根据波段数刷新下拉框与可用性；参数为“包含NONE”的总数
        void refreshBands(int totalBandsIncludingNone);
        
    private slots:
        void onUpdateClicked();
};

#endif
