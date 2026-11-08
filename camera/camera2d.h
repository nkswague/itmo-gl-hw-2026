#pragma once

#include <QMatrix4x4>
#include <QPointF>
#include <QSize>
#include <QVector2D>
#include <algorithm>

class Camera2D
{
public:
    void resize(const QSize &viewport);
    QVector2D screenToWorld(const QPointF &position) const;
    void keepPointAt(const QVector2D &worldPoint, const QPointF &screenPosition);
    void zoomAt(const QPointF &screenPosition, float factor);
    QMatrix4x4 projectionMatrix() const;
    QMatrix4x4 viewMatrix() const;
    float worldUnitsPerPixel() const;

private:
    QVector2D center_{0.0f, 0.0f};
    static constexpr float minZoom_ = 0.1f;
    static constexpr float maxZoom_ = 100.0f;
    float zoom_ = 1.0f;
    float halfExtent_ = 1.2f;
    QSize viewport_{1, 1};
};

inline void Camera2D::resize(const QSize &viewport)
{
    viewport_ = viewport.expandedTo(QSize(1, 1));
}

inline QVector2D Camera2D::screenToWorld(const QPointF &position) const
{
    const QVector2D offset(
        static_cast<float>(position.x() - viewport_.width() / 2.0),
        static_cast<float>(viewport_.height() / 2.0 - position.y())
    );
    return center_ + worldUnitsPerPixel() * offset;
}

inline void Camera2D::keepPointAt(const QVector2D &worldPoint, const QPointF &screenPosition)
{
    center_ += worldPoint - screenToWorld(screenPosition);
}

inline void Camera2D::zoomAt(const QPointF &screenPosition, float factor)
{
    const QVector2D anchor = screenToWorld(screenPosition);
    zoom_ = std::clamp(zoom_ * factor, minZoom_, maxZoom_);
    keepPointAt(anchor, screenPosition);
}

inline QMatrix4x4 Camera2D::projectionMatrix() const
{
    const float scale = worldUnitsPerPixel();
    const float halfWidth = viewport_.width() * scale / 2.0f;
    const float halfHeight = viewport_.height() * scale / 2.0f;
    QMatrix4x4 matrix;
    matrix.ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, 0.1f, 100.0f);
    return matrix;
}

inline QMatrix4x4 Camera2D::viewMatrix() const
{
    QMatrix4x4 matrix;
    matrix.translate(-center_.x(), -center_.y(), 0.0f);
    return matrix;
}

inline float Camera2D::worldUnitsPerPixel() const
{
    const int shortSide = std::min(viewport_.width(), viewport_.height());
    return 2.0f * halfExtent_ / (zoom_ * shortSide);
}
