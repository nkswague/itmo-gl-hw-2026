#pragma once

#include "fractal/apollonianpacking.h"
#include "camera/camera2d.h"
#include "input/cameradragcontroller.h"
#include "input/camerazoomcontroller.h"
#include "rendering/circlerenderer.h"
#include "openglwidget.h"

class Window : public OpenGLWidget
{
    Q_OBJECT

public:
    Window();
    ~Window();

    void onInit() override;
    void onRender() override;
    void onResize(size_t width, size_t height) override;

private:
    void createControlPanel();
    void setGenerationCount(int generationCount);
    void rebuildCircles();
    void setPaletteBalance(int value);
    void releaseRenderResources();

    ApollonianPacking packing_;
    Camera2D camera_;
    CameraDragController dragController_;
    CameraZoomController zoomController_;
    CircleRenderer renderer_;
    int generationCount_ = 20;
    bool animated_ = true;
};
