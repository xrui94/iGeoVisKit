#ifndef _PARBAT_OVERVIEWWINDOW_H
#define _PARBAT_OVERVIEWWINDOW_H

#include <QWidget>
#include <QMenu>

class GLView;
class Renderer;
class OverviewGL;


class OverviewWindow : public QWidget
{
    Q_OBJECT

public:
    OverviewWindow(QWidget *parent = nullptr);
    virtual ~OverviewWindow();

    int toggleMenuItemTick(QMenu* menu, int itemId);
    
    int getWidth() const;
    int getHeight() const;
    
    // Qt特定的公共方法
    QMenu* mainMenu() const { return mainMenu_; }

    // 直接使用自身作为 OpenGL 父控件
    QWidget* displayWidget() { return this; }

    // 将 Renderer 与 OverviewGL 目标接线到内部 GLView
    void attachRendererAndOverview(const std::shared_ptr<Renderer>& renderer, OverviewGL* ov);

    // 公开访问嵌入的 GLView（用于主窗口接线）
    GLView* getGLView() const { return m_glView; }

private:
    QMenu *mainMenu_;
    GLView* m_glView = nullptr;
    
    static const int WIDTH = 250;  // width of the overview window in pixels
    static const int HEIGHT = 400;
};

#endif