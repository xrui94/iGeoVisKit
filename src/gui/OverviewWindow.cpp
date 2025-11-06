#include "PchApp.h"

#include "OverviewWindow.h"

#include "gui/GLView.h"
#include "opengl/Renderer.h"
#include "opengl/OverviewGL.h"

#include "ImageWindow.h"
#include "MainWindow.h"


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

    // 创建内部 GLView 并加入布局
    m_glView = new GLView(this);
    m_glView->setMinimumSize(4, 4);
    m_glView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(m_glView);
}

OverviewWindow::~OverviewWindow()
{
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

void OverviewWindow::attachRendererAndOverview(const std::shared_ptr<Renderer>& renderer, OverviewGL* ov)
{
    if (!m_glView) return;
    m_glView->setRendererInstance(renderer);
    m_glView->addRenderCallback([ov](Renderer& r){
        if (ov) r.renderOverview(ov);
    });
}
