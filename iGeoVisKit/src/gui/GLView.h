#ifndef GL_VIEW_H
#define GL_VIEW_H

#include <QOpenGLWidget>
#include <functional>
#include <memory>
//#include <QOpenGLFunctions>
// #include <QGLWidget>  // Removed in Qt6, replaced with QOpenGLWidget

// 移除Windows的OpenGL头文件包含
// #include <GL/gl.h>
// #include <GL/glu.h>

class GLView : public QOpenGLWidget/*, protected QOpenGLFunctions*/
{
    Q_OBJECT

public:
    GLView(QWidget *parent = nullptr);
    virtual ~GLView();
    
    float aspect();
    int height();
    int width();
    void resize();
    
    // 兼容性方法，用于替代Windows API
    void make_current();
    void swap();
    void setRenderer(const std::function<void()>& fn);
    // 新增：设置 Renderer 实例与上下文感知的回调
    void setRendererInstance(const std::shared_ptr<class Renderer>& renderer);
    void setRenderCallback(const std::function<void(class Renderer&, class OpenGLContext&)>& cb);
    
    // 移除Windows特定的方法
    // HDC get_device_context();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void showEvent(QShowEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void resized(int w, int h);
    void wheelScrolled(int delta, int x, int y);
    void mousePressed(int x, int y);
    void mouseMoved(int buttons, int x, int y);
    void mouseReleased(int x, int y);

private:
    // 添加缺失的成员变量
    int status;
    const char* error_text;
    QWidget* widget;
    int window_height;
    int window_width;
    bool gladReady;
    std::function<void()> renderer;
    std::shared_ptr<class Renderer> rendererInstance;
    std::function<void(class Renderer&, class OpenGLContext&)> renderCallback;
};

#endif