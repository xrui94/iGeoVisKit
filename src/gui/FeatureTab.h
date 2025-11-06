#ifndef _PARBAT_FEATURETAB_H
#define _PARBAT_FEATURETAB_H

#include "ToolTab.h"

// Qt includes
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>

class FeatureTab: public ToolTab
{
    private:
        // WNDPROC prevProc;  // 移除Windows特定成员
        // HWND hTrackbar;  // 移除Windows特定成员
        // HWND hX, hY, hZ;  // 移除Windows特定成员
        // HWND hgenerate;  // 移除Windows特定成员
        // HWND *xRadiobuttons;  // 移除Windows特定成员
        // HWND *yRadiobuttons;  // 移除Windows特定成员
        // HWND *zRadiobuttons;  // 移除Windows特定成员
        // HWND hROIOnly;  // 移除Windows特定成员
        // HWND htitle;  // 移除Windows特定成员
        
        // 添加Qt组件成员
        QSlider *hTrackbar;
        QRadioButton **xRadiobuttons;
        QRadioButton **yRadiobuttons;
        QRadioButton **zRadiobuttons;
        QCheckBox *hROIOnly;
        QLabel *htitle;
        QPushButton *hGenerate;
        
        void OnGenerateClicked(int lod, bool rois_only, int x, int y, int z);
        void OnUserMessage();
        static int guessPoints(int slider_pos);
        
        // 添加Qt信号槽
        void onSliderValueChanged(int value);
        void onXRadioToggled(bool checked);
        void onYRadioToggled(bool checked);
        void onZRadioToggled(bool checked);
        void onGenerateClicked();
        
    public:
        const char* GetTabName() {return "Feature";};
        const char* GetTabHeading() {return "Feature Space";};
        int GetContainerHeight();   
        // int Create(HWND parent,RECT *parentRect);  // 修改函数签名
        int Create(QWidget *parent, QRect *parentRect);
        // static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);  // 移除Windows特定函数
        
        // 添加setupUI函数声明
        void setupUI();
};

#endif