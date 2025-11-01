#include "PchApp.h"
#include "GLContainer.h"
#include "config.h"
#include "utils/Console.h"

// Qt includes
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

GLContainer::GLContainer(QWidget *parent, GLContainerHandler *eventHandler, int x, int y, int width, int height)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(width);
    Q_UNUSED(height);
    
    // setup variables
    handler = eventHandler;
    
    // 在Qt中，我们不需要手动创建窗口和控件，而是在setupUI中完成
    
    setParent(parent);
}

void GLContainer::setupUI()
{
    // 设置窗口属性
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    
    // 设置尺寸策略
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void GLContainer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Console::write("GLContainer mousePressEvent\n");
        if (handler != NULL)
            handler->OnGLContainerLeftMouseDown(event->x(), event->y());
    }
    
    QWidget::mousePressEvent(event);
}

void GLContainer::mouseMoveEvent(QMouseEvent *event)
{
    if (handler != NULL)
        handler->OnGLContainerMouseMove(event->buttons(), event->x(), event->y());
    
    QWidget::mouseMoveEvent(event);
}

void GLContainer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    if (handler != NULL)
        handler->PaintGLContainer();
}

int GLContainer::GetContainerHeight()
{
    // 返回容器高度
    return height();
}