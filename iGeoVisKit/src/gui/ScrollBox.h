#ifndef _PARBAT_SCROLLABLEWINDOW_H
#define _PARBAT_SCROLLABLEWINDOW_H

// #include "Window.h"
#include <QWidget>
#include <QScrollArea>

class ScrollBox:public QScrollArea
{
    Q_OBJECT

private:
    // WNDPROC prevProc;  // 移除Windows特定成员

protected:
    int maxScrollHeight;
    int pixelPosition;

    // static BOOL CALLBACK GetMaxScrollHeight(HWND hwnd, LPARAM lparam);  // 移除Windows特定函数
    // static LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);  // 移除Windows特定函数
    void Scroll(int msg);  // 保留但需要修改实现

public:
    // int Create(HWND parentHandle,RECT *rect);  // 修改函数签名
    int Create(QWidget *parentWidget, QRect *rect);  // 修改函数签名
    void UpdateScrollBar();  // 保留但需要修改实现
};

#endif