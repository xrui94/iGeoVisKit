#include "PchApp.h"
#include "ImageWindow.h"

#include "config.h"
#include "utils/Console.h"
#include "OverviewWindow.h"
#include "MainWindow.h"
#include "ToolWindow.h"
#include "utils/Settings.h"
#include "imagery/ImageHandler.h"
#include "opengl/ImageViewport.h"

// Qt头文件
#include <QApplication>
#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QKeyEvent>
#include <QImage>
#include <QPixmap>
#include <QDir>
#include <QAbstractSlider>
#include <QSizePolicy>

char* imageWindowTitle;

ImageWindow::ImageWindow(QWidget *parent)
    : QWidget(parent)
    , scrollArea(nullptr)
    , imageLabel(nullptr)
    , horizontalScrollBar(nullptr)
    , verticalScrollBar(nullptr)
    , scaleFactor(1.0)
{
    setupUI();
    
    // 设置窗口属性
    setWindowTitle(QStringLiteral("Image Window"));
    resize(800, 600);
}

ImageWindow::~ImageWindow()
{
}

int ImageWindow::Create(QWidget* parent) {
    //Q_UNUSED(parent);
    // 嵌入式视图：不以顶层窗口方式显示，由父布局控制
    //setWindowFlags(Qt::Widget);
    return true;
}

void ImageWindow::setupUI()
{
    // 直接在自身上建立布局（作为 QWidget）
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 直接使用自身作为 GLView 父容器（移除 DisplayWindow）
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 保留滚动区域但默认不显示（用于非 OpenGL 路径或未来用）
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignCenter);
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    scrollArea->setWidget(imageLabel);
    scrollArea->hide();
    
    // 获取滚动条
    horizontalScrollBar = scrollArea->horizontalScrollBar();
    verticalScrollBar = scrollArea->verticalScrollBar();
    
    // 连接滚动条信号
    connect(horizontalScrollBar, &QScrollBar::valueChanged, 
            this, &ImageWindow::onHorizontalScroll);
    connect(verticalScrollBar, &QScrollBar::valueChanged, 
            this, &ImageWindow::onVerticalScroll);
    
    // QWidget 不使用状态栏，由 MainWindow 管理菜单/状态
}

// 菜单/动作由 MainWindow 管理

void ImageWindow::loadImage(const QString &fileName)
{
    QImage newImage(fileName);
    if (newImage.isNull()) {
        QMessageBox::information(this, QStringLiteral("Image Viewer"),
                                 QStringLiteral("Cannot load %1.").arg(fileName));
        return;
    }
    
    currentImage = newImage;
    currentPixmap = QPixmap::fromImage(currentImage);
    imageLabel->setPixmap(currentPixmap);
    imageLabel->resize(currentPixmap.size());
    
    scaleFactor = 1.0;
    
    updateImageWindowTitle();
    updateImageScrollbar();
}

void ImageWindow::updateImageWindowTitle()
{
    setWindowTitle(QStringLiteral("Image Window[*]"));
}

void ImageWindow::updateImageScrollbar()
{
    // 更新滚动条范围
    if (!currentImage.isNull()) {
        QSize scaledSize = currentPixmap.size() * scaleFactor;
        QPoint pos = scrollArea->widget()->pos();
        
        horizontalScrollBar->setRange(0, scaledSize.width() - scrollArea->viewport()->width());
        verticalScrollBar->setRange(0, scaledSize.height() - scrollArea->viewport()->height());
    }
}

void ImageWindow::scrollImageX(int scrollMsg)
{
    // 处理水平滚动
    horizontalScrollBar->triggerAction(static_cast<QAbstractSlider::SliderAction>(scrollMsg));
}

void ImageWindow::scrollImageY(int scrollMsg)
{
    // 处理垂直滚动
    verticalScrollBar->triggerAction(static_cast<QAbstractSlider::SliderAction>(scrollMsg));
}

void ImageWindow::scrollImage(bool vert, int amount)
{
    // 处理滚动
    if (vert) {
        verticalScrollBar->setValue(verticalScrollBar->value() + amount);
    } else {
        horizontalScrollBar->setValue(horizontalScrollBar->value() + amount);
    }
}

void ImageWindow::zoomImage(float nlevels)
{
    // 处理缩放
    scaleFactor *= nlevels;
    
    if (scaleFactor < 0.1) scaleFactor = 0.1;
    if (scaleFactor > 10.0) scaleFactor = 10.0;
    
    QSize newSize = currentPixmap.size() * scaleFactor;
    imageLabel->resize(newSize);
    
    updateImageScrollbar();
}

// 打开/保存由 MainWindow 管理

void ImageWindow::zoomIn()
{
    zoomImage(1.25);
}

void ImageWindow::zoomOut()
{
    zoomImage(0.8);
}

void ImageWindow::setFitToWindow(bool enabled)
{
    m_fitToWindow = enabled;
    if (m_fitToWindow) {
        scrollArea->setWidgetResizable(true);
        imageLabel->resize(scrollArea->viewport()->size());
    } else {
        scrollArea->setWidgetResizable(false);
        imageLabel->resize(currentPixmap.size());
    }
}

void ImageWindow::onHorizontalScroll(int value)
{
    // 处理水平滚动
    Q_UNUSED(value);
}

void ImageWindow::onVerticalScroll(int value)
{
    // 处理垂直滚动
    Q_UNUSED(value);
}

void ImageWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void ImageWindow::onKeyDown(int virtualKey)
{
    // Qt中的键盘事件处理
    switch (virtualKey) {
    case VK_UP:
        scrollImageY(SB_LINEUP);
        break;
    case VK_DOWN:
        scrollImageY(SB_LINEDOWN);
        break;
    case VK_LEFT:
        scrollImageX(SB_LINEUP);
        break;
    case VK_RIGHT:
        scrollImageX(SB_LINEDOWN);
        break;
    case VK_PRIOR:  // Page Up
        zoomImage(1);
        break;
    case VK_NEXT:   // Page Down
        zoomImage(-1);
        break;
    }
}

int ImageWindow::getWidth() const
{
    return width();
}

int ImageWindow::getHeight() const
{
    return height();
}
