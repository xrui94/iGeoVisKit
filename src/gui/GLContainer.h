#ifndef _PARBAT_GLContainer_H
#define _PARBAT_GLContainer_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

class GLContainerHandler
{
    public:
    virtual void PaintGLContainer() { };
    virtual void OnGLContainerLeftMouseDown(int x,int y) { };    
    virtual void OnGLContainerMouseMove(int vkeys,int x,int y) { }; 
};

class GLContainer : public QWidget
{
    Q_OBJECT

public:
    GLContainer(QWidget *parent, GLContainerHandler *eventHandler, int x, int y, int width, int height);
    
    int GetContainerHeight();
    
    // 将setupUI函数移到public部分
    void setupUI();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    GLContainerHandler *handler;
};

#endif