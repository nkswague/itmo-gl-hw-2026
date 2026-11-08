#pragma once

#include "../camera/camera2d.h"

#include <QEvent>
#include <QObject>
#include <QWheelEvent>
#include <QWidget>
#include <cmath>

class CameraZoomController : public QObject
{
public:
    CameraZoomController(Camera2D &camera, QWidget &viewport);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    bool handleWheel(QWheelEvent *event);
    static float wheelSteps(const QWheelEvent &event);
    void zoomAt(const QPointF &position, float steps);

    Camera2D &camera_;
    QWidget &viewport_;
};

inline CameraZoomController::CameraZoomController(Camera2D &camera, QWidget &viewport)
    : camera_(camera)
    , viewport_(viewport)
{
    viewport_.installEventFilter(this);
}

inline bool CameraZoomController::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Wheel) {
        return handleWheel(static_cast<QWheelEvent*>(event));
    }
    return QObject::eventFilter(watched, event);
}

inline bool CameraZoomController::handleWheel(QWheelEvent *event)
{
    if (viewport_.childAt(event->position().toPoint())) {
        return false;
    }

    const float steps = wheelSteps(*event);
    if (steps == 0.0f) {
        return false;
    }

    zoomAt(event->position(), steps);
    event->accept();
    return true;
}

inline float CameraZoomController::wheelSteps(const QWheelEvent &event)
{
    if (event.angleDelta().y() != 0) {
        return event.angleDelta().y() / 120.0f;
    }
    return event.pixelDelta().y() / 100.0f;
}

inline void CameraZoomController::zoomAt(const QPointF &position, float steps)
{
    camera_.resize(viewport_.size() * viewport_.devicePixelRatioF());
    camera_.zoomAt(position * viewport_.devicePixelRatioF(), std::pow(1.15f, steps));
    viewport_.update();
}
