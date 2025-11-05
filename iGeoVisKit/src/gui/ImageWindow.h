#ifndef _PARBAT_IMAGEWINDOW_H
#define _PARBAT_IMAGEWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QImage>
#include <QPixmap>
#include <QKeyEvent>

#include "imagery/ImageHandler.h"

class MainWindow;
class ImageViewport;
class GLView;

class ImageWindow : public QWidget
{
    Q_OBJECT

public:
    ImageWindow(MainWindow* mainWindow, QWidget *parent = nullptr);
    virtual ~ImageWindow();

    virtual int Create(QWidget* parent = nullptr);
    
    void scrollImage(bool vertical, int amount);
    void updateImageScrollbar();
    void updateImageWindowTitle();

    // 作为 GLView 的父容器，直接使用自身（移除 DisplayWindow）
    
    int getWidth() const;
    int getHeight() const;

    void updateImageProperties(ImageProperties* imageProps);

    // 拉伸模式设置：转发到 ImageGL
    void setStretchMode(ImageGL::StretchMode m);

    // 公开访问嵌入的 GLView（用于主窗口接线）
    GLView* getGLView() const { return m_glImageView; }
    // 绑定视口到外部 ImageHandler 的视口，确保鼠标事件驱动渲染
    void setViewport(ImageViewport* vp) { m_viewport = vp; }

public slots:
    void zoomIn();
    void zoomOut();
    void setFitToWindow(bool enabled);
    void onHorizontalScroll(int value);
    void onVerticalScroll(int value);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupUI();
    void loadImage(const QString &fileName);
    void scrollImageX(int scrollMsg);
    void scrollImageY(int scrollMsg);
    void zoomImage(float nlevels);
    void onKeyDown(int virtualKey);

private:
    // UI组件
    MainWindow* m_mainWindow;
    QScrollArea *scrollArea;
    QLabel *imageLabel;
    QScrollBar *horizontalScrollBar;
    QScrollBar *verticalScrollBar;
    qreal scaleFactor;
    QImage currentImage;
    QPixmap currentPixmap;

    //
    int viewport_width, viewport_height; // Image pixels displayed in window
    int viewport_x, viewport_y; // Top-left corner (image pixels)
    int mouse_x, mouse_y;		// Current mouse position in image co-ords (for drawing new ROI)
    int m_dragLastX, m_dragLastY; // Last mouse pos for left-button drag panning
    ImageViewport* m_viewport = nullptr;
    GLView* m_glImageView = nullptr;
    GLView* m_glTextiew = nullptr;

    bool m_fitToWindow = false;
};

#endif