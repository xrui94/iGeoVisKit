#ifndef _PARBAT_MAINWINDOW_H
#define _PARBAT_MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QString>

#include <memory>
#include <string>

class OverviewWindow;
class ToolWindow;
class ROIWindow;
class ImageWindow;
class PrefsWindow;
class ContrastWindow;
class ContrastAdvWindow;
class ProgressStatusWidget;
class Renderer;
class ImageHandler;

class QWidget;
class QMenuBar;
class QMenu;
class QAction;
class QToolBar;
class QStatusBar;
class QTabWidget;
class QDockWidget;
class QTreeWidget;
class QTreeWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();
    
    void RestoreAll();      /* restore all windows back to their original state */
    void MinimizeAll();     /* hide all windows owned by the current thread */
    void DestroyAll();		/* destroy all windows owned by current thread */
    void DisableAll();		/* disable all windows owned by main thread */
    void DisableAll(QWidget*);	/* disable all windows except for a particular window */
    void EnableAll();		/* enable all windows owned by main thread */
    
    // Qt菜单和工具栏访问
    QMenuBar* mainMenuBar() const { return menuBar(); }
    QMenu* fileMenu() const { return fileMenu_; }
    QMenu* windowMenu() const { return windowMenu_; }
    QMenu* helpMenu() const { return helpMenu_; }
    QMenu* viewMenu() const { return viewMenu_; }
    
    // 动作访问
    QAction* openAction() const { return openAction_; }
    QAction* saveAction() const { return saveAction_; }
    QAction* saveAsAction() const { return saveAsAction_; }
    QAction* closeAction() const { return closeAction_; }
    QAction* exitAction() const { return exitAction_; }
    
    QAction* imageWindowAction() const { return imageWindowAction_; }
    QAction* toolsWindowAction() const { return toolsWindowAction_; }
    QAction* roiWindowAction() const { return roiWindowAction_; }
    QAction* prefsWindowAction() const { return prefsWindowAction_; }
    QAction* contrastWindowAction() const { return contrastWindowAction_; }
    QAction* contrastAdvWindowAction() const { return contrastAdvWindowAction_; }
    
    QAction* helpContentsAction() const { return helpContentsAction_; }
    QAction* aboutAction() const { return aboutAction_; }
    QAction* zoomInAction() const { return zoomInAction_; }
    QAction* zoomOutAction() const { return zoomOutAction_; }
    QAction* fitToWindowAction() const { return fitToWindowAction_; }

    // 窗口访问（统一从 MainWindow 获取）
    ImageWindow* imageWindow() const { return imageWindow_; }
    OverviewWindow* overviewWindow() const { return overviewWidget_; }
    ToolWindow* toolWindow() const { return toolWindow_; }
    ROIWindow* roiWindow() const { return roiWindow_; }
    PrefsWindow* prefsWindow();
    ContrastWindow* contrastWindow();
    ContrastAdvWindow* contrastAdvWindow();
    ProgressStatusWidget* progressWidget();
    
    // 统一的 Renderer 实例访问
    std::shared_ptr<Renderer> getRenderer() const { return renderer_; }
    // 当前图像会话访问（替代全局 image_handler）
    ImageHandler* imageHandler() const { return m_imageHandler.get(); }
    // 当前文件名访问
    const std::string& filename() const { return m_filename; }

private slots:
    void onFileItemActivated(QTreeWidgetItem* item, int column);

private:
    void setupMenus();
    void setupActions();
    void setupToolBar();
    void setupStatusBar();

    // 添加openFile函数声明
    void openFile();        /* open file dialog */

    void loadFile();

    void loadFile(const QString& fileName);

    void closeFile();

    // 确保主窗口在文件对话框关闭后处于可见与激活状态
    void ensureVisibleAndActive();
    // 强制刷新关键UI区域，避免首次绘制出现空白
    void forceRefreshUI();

private:
    // 菜单
    QMenu *fileMenu_;
    QMenu *windowMenu_;
    QMenu *helpMenu_;
    QMenu *viewMenu_;
    
    // 菜单动作
    QAction *openAction_;
    QAction *saveAction_;
    QAction *saveAsAction_;
    QAction *closeAction_;
    QAction *exitAction_;
    
    QAction *imageWindowAction_;
    QAction *toolsWindowAction_;
    QAction *roiWindowAction_;
    QAction *prefsWindowAction_;
    QAction *contrastWindowAction_;
    QAction *contrastAdvWindowAction_;
    
    QAction *helpContentsAction_;
    QAction *aboutAction_;
    QAction *zoomInAction_;
    QAction *zoomOutAction_;
    QAction *fitToWindowAction_;
    
    // 保存的窗口状态
    QVector<QWidget*> savedWindows;
    QVector<int> restoreStates;

    // 中央 Tab 管理
    QTabWidget* centralTabs_ = nullptr;
    QWidget* imageTabPage_ = nullptr;

    // Dock：文件树、概览、工具
    QDockWidget* filesDock_ = nullptr;
    QTreeWidget* filesTree_ = nullptr;
    QDockWidget* overviewDock_ = nullptr;
    OverviewWindow* overviewWidget_ = nullptr;
    QDockWidget* toolsDock_ = nullptr;
    ToolWindow* toolWindow_ = nullptr;
    ROIWindow* roiWindow_ = nullptr; // 非模态对话框（默认隐藏）
    ImageWindow* imageWindow_ = nullptr; // 中央 Image 页中的嵌入式视图

    // 其余窗口（按需惰性创建）
    PrefsWindow* prefsWindow_ = nullptr;
    ContrastWindow* contrastWindow_ = nullptr;
    ContrastAdvWindow* contrastAdvWindow_ = nullptr;
    ProgressStatusWidget* progressWidget_ = nullptr;

    // 工具栏（用于统一入口）
    QToolBar* m_toolsBar = nullptr;

    QStatusBar* m_statusBar = nullptr;
    
    // 统一的 Renderer 实例
    std::shared_ptr<Renderer> renderer_;
    // 当前图像会话（避免使用全局变量）
    std::unique_ptr<ImageHandler> m_imageHandler;
    // 当前打开的文件名（UTF-8）
    std::string m_filename;
};

#endif