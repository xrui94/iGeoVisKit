#include "PchApp.h"
#include "ImageWindow.h"
#include "MainWindow.h"
#include "opengl/ImageViewport.h"
#include "opengl/ImageGL.h"
#include "opengl/Renderer.h"
#include "opengl/OpenGLContext.h"

#include "config.h"
#include "utils/Console.h"
#include "OverviewWindow.h"
#include "MainWindow.h"
#include "ToolWindow.h"
#include "utils/Settings.h"
#include "imagery/ImageHandler.h"
//#include "opengl/ImageViewport.h"

char* imageWindowTitle;

ImageWindow::ImageWindow(MainWindow* mainWidnow, QWidget *parent)
    : QWidget(parent), m_mainWindow(mainWidnow)
    , scrollArea(nullptr)
    , imageLabel(nullptr)
    , horizontalScrollBar(nullptr)
    , verticalScrollBar(nullptr)
    , scaleFactor(1.0)
{
    setupUI();
    
    // 设置窗口属性
    //setWindowTitle(QStringLiteral("Image Window"));
    //resize(800, 600);
    //resize(mainWidnow->width(), mainWidnow->height());
}

ImageWindow::~ImageWindow()
{
}

void ImageWindow::setupUI()
{
    // 直接在自身上建立布局（作为 QWidget）
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    //// 直接使用自身作为 GLView 父容器（移除 DisplayWindow）
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //// 保留滚动区域但默认不显示（用于非 OpenGL 路径或未来用）
    //scrollArea = new QScrollArea(this);
    //scrollArea->setWidgetResizable(true);
    //scrollArea->setAlignment(Qt::AlignCenter);
    //imageLabel = new QLabel(this);
    //imageLabel->setAlignment(Qt::AlignCenter);
    //imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    //scrollArea->setWidget(imageLabel);
    //scrollArea->hide();
    //
    //// 获取滚动条
    //horizontalScrollBar = scrollArea->horizontalScrollBar();
    //verticalScrollBar = scrollArea->verticalScrollBar();
    //
    //// 连接滚动条信号
    //connect(horizontalScrollBar, &QScrollBar::valueChanged, 
    //        this, &ImageWindow::onHorizontalScroll);
    //connect(verticalScrollBar, &QScrollBar::valueChanged, 
    //        this, &ImageWindow::onVerticalScroll);
    
    // QWidget 不使用状态栏，由 MainWindow 管理菜单/状态


    m_glImageView = new GLView(this);
    layout->addWidget(m_glImageView);
    // 确保滚轮与键盘事件优先到达 GLView
    m_glImageView->setFocus();

    const auto& renderer = m_mainWindow->getRenderer();

    m_glImageView->setRendererInstance(renderer);
    m_glImageView->addRenderCallback([this](Renderer& renderer) {
        ImageHandler* imageHandler = m_mainWindow->imageHandler();
        if (imageHandler) {
            // 直接使用控件当前大小进行视口同步，避免在渲染阶段重复调用 resizeGL
            imageHandler->getImageGL()->resize(m_glImageView->width(), m_glImageView->height());
            renderer.renderImageScene(imageHandler->getImageGL());
        }
    });

    // 在首次显示或控件尺寸变化时，同步 ImageGL 视口尺寸，确保首帧即可加载纹理
    QObject::connect(m_glImageView, &GLView::resized, [this](int w, int h) {
        ImageHandler* ih = m_mainWindow->imageHandler();
        if (ih) {
            ih->getImageGL()->resize(w, h);
            // 同步视口窗口尺寸，确保缩放/滚动计算有效
            if (ih->get_image_viewport()) {
                ih->get_image_viewport()->set_window_size(w, h);
            }
        }
    });

	// 左键拖拽平移：记录按下位置，并在移动时根据位移更新视口
	QObject::connect(m_glImageView, &GLView::mousePressed, this, [this](int wx, int wy) {
		m_dragLastX = wx;
		m_dragLastY = wy;
		Console::write("Mouse pressed at (%d, %d)\n", wx, wy);
	});

	QObject::connect(m_glImageView, &GLView::mouseMoved, this, [this](int buttons, int wx, int wy) {
		ImageHandler* ih = m_mainWindow->imageHandler();
		ImageViewport* vp = ih ? ih->get_image_viewport() : nullptr;
		if (vp && (buttons & Qt::LeftButton) && m_panEnabled) {
			int dx = wx - m_dragLastX;
			int dy = wy - m_dragLastY;
			Console::write("Mouse drag: dx=%d, dy=%d, from (%d,%d) to (%d,%d)\n",
				dx, dy, m_dragLastX, m_dragLastY, wx, wy);

            float zoom_level = vp->get_zoom_level();
            if (zoom_level <= 0.0f) zoom_level = vp->get_zoom_minimum();
            int image_dx = int(round(dx / zoom_level));
            int image_dy = int(round(dy / zoom_level));

            int old_image_x = vp->get_image_x();
            int old_image_y = vp->get_image_y();
            vp->set_image_x(old_image_x - image_dx);
            vp->set_image_y(old_image_y - image_dy);

            Console::write("Image coords updated: image_x %d->%d, image_y %d->%d (dx=%d, dy=%d, zoom=%.2f)\n",
                old_image_x, vp->get_image_x(), old_image_y, vp->get_image_y(),
                image_dx, image_dy, zoom_level);

            // 视口参数更新后请求重绘，确保平移立即生效
            if (m_glImageView) {
                m_glImageView->update();
            }

            m_dragLastX = wx;
            m_dragLastY = wy;
        }
    });

	// 鼠标滚轮缩放（以鼠标位置为锚点）
	QObject::connect(m_glImageView, &GLView::wheelScrolled, this, [this](int delta, int wx, int wy) {
		ImageHandler* ih = m_mainWindow->imageHandler();
		ImageViewport* vp = ih ? ih->get_image_viewport() : nullptr;
		if (vp) {
			float cur = vp->get_zoom_level();
			float step = (delta >= 0) ? 1.25f : 0.8f; // 每格约±120
			int ix = 0, iy = 0;
			vp->translate_window_to_image(wx, wy, &ix, &iy);
			float newZoom = cur * step;
			vp->set_zoom_level(newZoom);
			int zx = int(round(ix * vp->get_zoom_level() - wx));
			int zy = int(round(iy * vp->get_zoom_level() - wy));
			vp->set_zoom_x(zx);
			vp->set_zoom_y(zy);
			// 视口参数更新后请求重绘，确保缩放立即生效
			if (m_glImageView) {
				m_glImageView->update();
			}
		}
	});

	// 鼠标左键释放：结束拖拽并记录位置
	QObject::connect(m_glImageView, &GLView::mouseReleased, this, [this](int wx, int wy) {
		Console::write("Mouse released at (%d, %d)\n", wx, wy);
		m_dragLastX = wx;
		m_dragLastY = wy;
	});
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
    // 旧版基于 QScrollArea 的滚动条更新逻辑
    // 当采用 OpenGL 视口渲染路径时，scrollArea/imageLabel 为空，需直接跳过
    if (!scrollArea || !imageLabel) {
        return;
    }

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
    // 优先走 OpenGL 视口缩放路径（避免旧 QScrollArea 逻辑的空指针崩溃）
    ImageHandler* ih = m_mainWindow ? m_mainWindow->imageHandler() : nullptr;
    ImageViewport* vp = ih ? ih->get_image_viewport() : nullptr;
    if (vp) {
        float curZoom = vp->get_zoom_level();
        float newZoom = curZoom * nlevels;
        float minZoom = vp->get_zoom_minimum();
        if (newZoom < minZoom) newZoom = minZoom;
        if (newZoom > 10.0f) newZoom = 10.0f;
        vp->set_zoom_level(newZoom);

        if (m_glImageView) {
            m_glImageView->update();
        }
        return;
    }

    // 回退：仅当仍在旧滚动区域路径时，才执行以下逻辑
    scaleFactor *= nlevels;
    if (scaleFactor < 0.1) scaleFactor = 0.1;
    if (scaleFactor > 10.0) scaleFactor = 10.0;
    if (imageLabel) {
        QSize newSize = currentPixmap.size() * scaleFactor;
        imageLabel->resize(newSize);
    }
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

void ImageWindow::setPanEnabled(bool enabled)
{
	m_panEnabled = enabled;
	if (m_panEnabled) {
		setCursor(Qt::OpenHandCursor);
	} else {
		unsetCursor();
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

void ImageWindow::updateImageProperties(ImageProperties* imageProps)
{
    if (!m_viewport) {
        m_viewport = new ImageViewport(imageProps);
    }
    
}

// 设置影像拉伸模式并请求刷新
void ImageWindow::setStretchMode(ImageGL::StretchMode m)
{
    // 通过 ImageHandler 转发到 ImageGL，并请求刷新
    ImageHandler* ih = m_mainWindow ? m_mainWindow->imageHandler() : nullptr;
    if (ih) {
        ih->set_stretch_mode(m);
        if (m_glImageView) m_glImageView->update();
    }
}

