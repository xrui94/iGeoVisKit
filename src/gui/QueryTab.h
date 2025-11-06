#ifndef _PARBAT_QUERYTAB_H
#define _PARBAT_QUERYTAB_H

#include "ToolTab.h"

// Qt includes
#include <QWidget>
#include <QLabel>

class QueryTab:public ToolTab
{
    Q_OBJECT

    private:
        // WNDPROC prevProc;  // 移除Windows特定成员
    public:
       // HWND *imageBandValues;  // 移除Windows特定成员
       QLabel **imageBandValues;  // 使用Qt组件替代
       
       // HWND cursorXPos, cursorYPos;  // 移除Windows特定成员
       QLabel *cursorXPos, *cursorYPos;  // 使用Qt组件替代
        
        const char* GetTabName() {return "Query";};
        const char* GetTabHeading() {return "Band Values";};
        int GetContainerHeight();   
        // int Create(HWND parent,RECT *parentRect);  // 修改函数签名
        int Create(QWidget *parent, QRect *parentRect);
        // static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);  // 移除Windows特定函数
        
        void SetCursorX(char *text);  // set text on cursorX control
        void SetCursorY(char *text);  // set text on cursorY control
        void SetImageBandValue(int i,char *text);   // set text on imageBandValue[i] control
        
        // 添加setupUI函数声明
        void setupUI();
};

#endif