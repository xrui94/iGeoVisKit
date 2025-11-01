#include "PchApp.h"

#include "OverviewWindow.h"

#include "ImageWindow.h"
#include "MainWindow.h"

// Qt includes
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QSizePolicy>

OverviewWindow::OverviewWindow(QWidget *parent)
    : QWidget(parent)
    , mainMenu_(nullptr)
{
    setWindowTitle(QStringLiteral("Overview Window"));
    resize(WIDTH, HEIGHT);
    // 作为 OpenGL 父容器，直接使用自身并建立零边距布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

OverviewWindow::~OverviewWindow()
{
}

int OverviewWindow::Create(QWidget* parent)
{
    Q_UNUSED(parent);

    /* Get Main Window Location for overview window alignment*/
    // GetWindowRect(mainWindow.GetHandle(),&rect);
    
    /* create overview window */
    // if (!CreateWin(0, "Parbat3D Overview Window", "Overview",
    //      WS_POPUP+WS_SYSMENU+WS_CAPTION,
    //      rect.left, rect.bottom, WIDTH, HEIGHT, parent, NULL))
    //     return false;
        
    /* Overview Window title */
    // HWND hTitle=CreateWindowEx(0, szStaticControl, "Overview",
    // WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE  | SS_OWNERDRAW, 8, 8, 100, 18,
    // GetHandle(), NULL, Window::GetAppInstance(), NULL);
    // SetStaticFont(hTitle,STATIC_FONT_HEADING);
    
    /* Create the overview display area */
    // Qt 环境中控件在构造时已建立，无需额外创建；布局已在构造函数完成。
    show();
    return 1;

}

int OverviewWindow::toggleMenuItemTick(QMenu* menu, int itemId)
{
    Q_UNUSED(menu);
    Q_UNUSED(itemId);
    // In Qt, we would use QAction::isChecked() and QAction::setChecked()
    // For now, we'll just return a default value
    return 1; // Checked
}

int OverviewWindow::getWidth() const
{
    return WIDTH;
}

int OverviewWindow::getHeight() const
{
    return HEIGHT;
}
