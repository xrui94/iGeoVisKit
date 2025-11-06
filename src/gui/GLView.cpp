#include "PchApp.h"

#include "GLView.h"

#include "opengl/Renderer.h"
#include "opengl/OpenGLContext.h"

#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QWheelEvent>



#include "config.h"
#include "utils/Console.h"

// 从PchApp.h获取Qt包含
// #include <QOpenGLWidget>
// #include <QSurfaceFormat>

GLView::GLView(QWidget *widget_arg)
    : QOpenGLWidget(widget_arg)
{
    status = 0; // No error
    error_text = "No error";
    gladReady = false;
    
    // 使用Qt widget替代Windows API
    //widget = widget_arg;
    
    // 初始化窗口尺寸
    //window_height = 0;
    //window_width = 0;
    
    // 设置 OpenGL 上下文为 3.3 Core Profile
    QSurfaceFormat format;
    format.setDepthBufferSize(16);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);

	// 确保可以接收滚轮与键盘事件
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
}

GLView::~GLView()
{
    // QOpenGLWidget会自动清理OpenGL上下文
}

/* Call on window re-size to adjust OpenGL context to fit */
void GLView::resize()
{
    // 使用Qt widget获取尺寸
    //if (widget) {
    //    window_height = widget->height();
    //    window_width = widget->width();
    //}
    
    // 调用QOpenGLWidget的resizeGL函数
    // 使用控件实际大小，避免内部缓存未初始化导致传入 0
    resizeGL(QOpenGLWidget::width(), QOpenGLWidget::height());
}

void GLView::initializeGL()
{
    // 初始化 Qt 的 OpenGL 函数指针（必须在上下文可用时调用）
    //initializeOpenGLFunctions();

    // 使用glad初始化OpenGL函数指针
    // if (!gladLoadGL()) {
    //     status = 1;
    //     error_text = "Failed to initialize GLAD";
    //     return;
    // }

    if (!gladLoadGLLoader([](const char* name) -> void* {
        QOpenGLContext* ctx = QOpenGLContext::currentContext();
        if (!ctx) return nullptr;
        return reinterpret_cast<void*>(ctx->getProcAddress(name));
        })) {
        qFatal("Failed to initialize GLAD");
        return;
    }

    m_initialized = true;
    
    //// 设置OpenGL状态（不透明白色，作为默认背景色）
    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);
    // 防止 Qt 绘制背景导致白屏
    //setAutoFillBackground(false);
}

void GLView::resizeGL(int w, int h)
{
    window_width = w;
    window_height = h;
    
    if (h == 0) h = 1; // 防止除零错误
    
    glViewport(0, 0, w, h);
    // 向外通知尺寸变化，便于首次绘制时更新视口
    emit resized(w, h);
}

void GLView::paintGL()
{
    if (m_initialized) {
        // 清除缓冲区，随后委托渲染器进行绘制
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glEnable(GL_DEPTH_TEST);//    目前都是2D渲染，暂时不开启深度测试，而且，最好不要在这里设置，而是在渲染每个 Mesh 时，再设置
        glDepthFunc(GL_LESS);
        // 上下文已由 QOpenGLWidget 设为当前
        if (rendererInstance) {
            for (const auto& renderCallback : m_renderCallbacks) {
                if (renderCallback) {
                    //ctx.bind(this);
                    renderCallback(*rendererInstance);
                    //swap();
                }
            }
        }
        else if (renderer) {
            renderer();
        }
    }
    else {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

float GLView::aspect()
{
    if (window_height == 0) return 1.0f;
    return (float)window_width / (float)window_height;
}

int GLView::height()
{
	// 返回实际控件高度，避免初始阶段 window_height 为 0 导致视口无效
	return QOpenGLWidget::height();
}

int GLView::width()
{
	// 返回实际控件宽度，避免初始阶段 window_width 为 0 导致视口无效
	return QOpenGLWidget::width();
}

// 实现兼容性方法
void GLView::make_current()
{
    // 在需要直接调用 OpenGL 函数前，确保当前上下文有效
    makeCurrent();

    //// 确保在首次使用前初始化 GLAD（如果 initializeGL 尚未运行）
    //if (!gladReady) {
    //    if (!gladLoadGLLoader([](const char* name) -> void* {
    //        QOpenGLContext* ctx = QOpenGLContext::currentContext();
    //        if (!ctx) return nullptr;
    //        return reinterpret_cast<void*>(ctx->getProcAddress(name));
    //    })) {
    //        qFatal("Failed to initialize GLAD (make_current)");
    //        return;
    //    }
    //    // 基础状态设置，避免后续调用使用未定义状态
    //    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //    glEnable(GL_DEPTH_TEST);
    //    glDepthFunc(GL_LESS);
    //    gladReady = true;
    //}
}

void GLView::swap()
{
    // QOpenGLWidget 使用 FBO 进行绘制，触发更新以呈现内容
    update();
}

void GLView::setRenderer(const std::function<void()>& fn)
{
    renderer = fn;
}

void GLView::setRendererInstance(const std::shared_ptr<Renderer>& r)
{
    rendererInstance = r;
}

void GLView::addRenderCallback(const std::function<void(Renderer&)>& cb)
{
    m_renderCallbacks.push_back(cb);
}

void GLView::showEvent(QShowEvent *event)
{
    QOpenGLWidget::showEvent(event);
    // 控件首次可见时，触发一次尺寸通知与绘制，避免首帧空白
    emit resized(width(), height());
    update();
}

void GLView::wheelEvent(QWheelEvent *event)
{
    // 使用 Qt6 的 angleDelta 取代旧版 delta
    int dy = event->angleDelta().y();
    // 触控板等设备可能只提供 pixelDelta；做回退以确保缩放响应
    if (dy == 0) {
        dy = event->pixelDelta().y();
    }
    const QPointF posf = event->position();
    emit wheelScrolled(dy, (int)posf.x(), (int)posf.y());
    event->accept();
}

void GLView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Console::write("GLView::mousePressEvent LeftButton at (%.1f, %.1f)\n", event->position().x(), event->position().y());
        emit mousePressed((int)event->position().x(), (int)event->position().y());
    }
    QOpenGLWidget::mousePressEvent(event);
}

void GLView::mouseMoveEvent(QMouseEvent *event)
{
    Console::write("GLView::mouseMoveEvent buttons=0x%X pos=(%.1f, %.1f)\n", (int)event->buttons(), event->position().x(), event->position().y());
    emit mouseMoved((int)event->buttons(), (int)event->position().x(), (int)event->position().y());
    QOpenGLWidget::mouseMoveEvent(event);
}

void GLView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Console::write("GLView::mouseReleaseEvent LeftButton at (%.1f, %.1f)\n", event->position().x(), event->position().y());
        emit mouseReleased((int)event->position().x(), (int)event->position().y());
        event->accept();
        return;
    }
    QOpenGLWidget::mouseReleaseEvent(event);
}