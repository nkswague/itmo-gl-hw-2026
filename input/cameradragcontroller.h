#pragma once

#include "../camera/camera2d.h"

#include <QEvent>
#include <QMouseEvent>
#include <QObject>
#include <QWidget>
#include <QtGlobal>

class CameraDragController : public QObject
{
public:
    CameraDragController(Camera2D &camera, QWidget &viewport);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    bool handleMousePress(QMouseEvent *event);
    bool handleMouseMove(QMouseEvent *event);
    bool handleMouseRelease(QMouseEvent *event);
    static QPointF mousePosition(const QMouseEvent &event);
    void beginDrag(const QPointF &position);
    void dragTo(const QPointF &position);
    void endDrag();

    Camera2D &camera_;
    QWidget &viewport_;
    bool dragging_ = false;
    QVector2D anchor_;
};

inline CameraDragController::CameraDragController(Camera2D &camera, QWidget &viewport)
    : camera_(camera), viewport_(viewport)
{
    viewport_.installEventFilter(this);
}

inline bool CameraDragController::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        return handleMousePress(static_cast<QMouseEvent*>(event));
    case QEvent::MouseMove:
        return handleMouseMove(static_cast<QMouseEvent*>(event));
    case QEvent::MouseButtonRelease:
        return handleMouseRelease(static_cast<QMouseEvent*>(event));
    default:
        return QObject::eventFilter(watched, event);
    }
}

inline bool CameraDragController::handleMousePress(QMouseEvent *event)
{
    const QPointF position = mousePosition(*event);
    if (event->button() != Qt::LeftButton || viewport_.childAt(position.toPoint())) {
        return false;
    }

    beginDrag(position);
    event->accept();
    return true;
}

inline bool CameraDragController::handleMouseMove(QMouseEvent *event)
{
    if (!dragging_) {
        return false;
    }

    if (!(event->buttons() & Qt::LeftButton)) {
        endDrag();
        return false;
    }

    dragTo(mousePosition(*event));
    event->accept();
    return true;
}

inline bool CameraDragController::handleMouseRelease(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || !dragging_) {
        return false;
    }

    dragTo(mousePosition(*event));
    endDrag();
    event->accept();
    return true;
}

inline QPointF CameraDragController::mousePosition(const QMouseEvent &event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event.position();
#else
    return event.localPos();
#endif
}

inline void CameraDragController::beginDrag(const QPointF &position)
{
    camera_.resize(viewport_.size() * viewport_.devicePixelRatioF());
    anchor_ = camera_.screenToWorld(position * viewport_.devicePixelRatioF());
    dragging_ = true;
    viewport_.setCursor(Qt::ClosedHandCursor);
}

inline void CameraDragController::dragTo(const QPointF &position)
{
    camera_.resize(viewport_.size() * viewport_.devicePixelRatioF());
    camera_.keepPointAt(anchor_, position * viewport_.devicePixelRatioF());
    viewport_.update();
}

inline void CameraDragController::endDrag()
{
    dragging_ = false;
    viewport_.unsetCursor();
}
