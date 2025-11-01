#ifndef _PARBAT_IMAGETAB_H
#define _PARBAT_IMAGETAB_H

#include "ToolTab.h"

// class ImageTab:public ToolTab  // 确保继承关系正确
class ImageTab: public ToolTab
{
    Q_OBJECT

    private:
        // WNDPROC prevProc;  // 移除Windows特定的成员
    public:
        const char* GetTabName() {return "Image";};
        const char* GetTabHeading() {return "Image Properties";};
        int GetContainerHeight();   
        // int Create(HWND parent,RECT *parentRect);  // 修改函数签名
        void Create(QWidget *parent);  // 修改函数签名
        // static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);  // 移除Windows特定的函数
};

#endif