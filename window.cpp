#include "window.h"
#include "ui/controlpanel.h"

#include <QOpenGLContext>
#include <QVBoxLayout>

Window::Window()
    : dragController_(camera_, *this)
    , zoomController_(camera_, *this)
{
    createControlPanel();
}

Window::~Window()
{
    releaseRenderResources();
}

void Window::onInit()
{
    connect(
        context(),
        &QOpenGLContext::aboutToBeDestroyed,
        this,
        &Window::releaseRenderResources,
        Qt::UniqueConnection
    );

    rebuildCircles();
    renderer_.initialize();
}

void Window::onRender()
{
    camera_.resize(size() * devicePixelRatioF());
    renderer_.render(packing_.circles(), camera_, animated_);
    if (animated_) {
        update();
    }
}

void Window::onResize(const size_t width, const size_t height)
{
    const QSize framebufferSize = QSize(static_cast<int>(width), static_cast<int>(height)) * devicePixelRatioF();
    camera_.resize(framebufferSize);
    renderer_.resize(framebufferSize.width(), framebufferSize.height());
}

void Window::createControlPanel()
{
    auto* controls = new ControlPanel(generationCount_, this);
    auto* overlayLayout = new QVBoxLayout(this);
    overlayLayout->setContentsMargins(8, 8, 8, 8);
    overlayLayout->addWidget(controls);
    overlayLayout->addStretch();

    connect(
        this,
        &Window::fpsUpdated,
        controls,
        &ControlPanel::setFps
    );
    connect(
        controls->iterationSlider(),
        &QSlider::valueChanged,
        this,
        &Window::setGenerationCount
    );
    connect(
        controls->paletteSlider(),
        &QSlider::valueChanged,
        this,
        &Window::setPaletteBalance
    );
}

void Window::setGenerationCount(int generationCount)
{
    if (generationCount_ == generationCount) {
        return;
    }

    generationCount_ = generationCount;
    rebuildCircles();
    update();
}

void Window::rebuildCircles()
{
    packing_.generateCircles(generationCount_);
    renderer_.updatePaletteRange(packing_.circles());
}

void Window::setPaletteBalance(int value)
{
    renderer_.setPalettePower(value / 100.0f);
    update();
}

void Window::releaseRenderResources()
{
    if (!context()) {
        return;
    }

    makeCurrent();
    renderer_.release();
    doneCurrent();
    disconnect(context(), &QOpenGLContext::aboutToBeDestroyed,
               this, &Window::releaseRenderResources);
}
