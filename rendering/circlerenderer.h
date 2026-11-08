#pragma once

#include "../fractal/circle.h"
#include "../camera/camera2d.h"

#include <QColor>
#include <QElapsedTimer>
#include <QImage>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QtGlobal>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <memory>
#include <vector>

class CircleRenderer : protected QOpenGLFunctions
{
public:
    void initialize();
    void render(const std::vector<Circle> &circles, const Camera2D &camera, bool animated);
    void resize(size_t width, size_t height);
    void release();
    void updatePaletteRange(const std::vector<Circle> &circles);
    void setPalettePower(float power);

private:
    void initializeShaderProgram();
    void initializeTexture();
    void initializeUniforms();
    void initializeGeometry();
    void initializeRenderState();

    void beginFrame(const Camera2D &camera, bool animated);
    void drawCircles(const std::vector<Circle> &circles);
    void drawCircle(
        const Circle &circle,
        float worldUnitsPerPixel
    );
    void endFrame();

    inline static constexpr std::array<GLfloat, 28u> vertices_ = {
        -1.0f, -1.0f,   1.f, 1.f, 1.f,  0.f, 0.f,
         1.0f, -1.0f,   1.f, 1.f, 1.f,  1.f, 0.f,
         1.0f,  1.0f,   1.f, 1.f, 1.f,  1.f, 1.f,
        -1.0f,  1.0f,   1.f, 1.f, 1.f,  0.f, 1.f,
    };

    inline static constexpr std::array<GLuint, 6u> indices_ = {0, 1, 2, 2, 3, 0};

    QElapsedTimer timer_;
    GLint colorUniform_ = -1;
    float hue_ = 0;
    GLint mvpUniform_ = -1;

    GLint radiusUniform_ = -1;
    GLint curvatureUniform_ = -1;
    GLint paletteRangeUniform_ = -1;
    float paletteRange_ = 1.0f;

    GLint palettePowerUniform_ = -1;
    float palettePower_ = 1.0f;

    QMatrix4x4 model_;
    QMatrix4x4 view_;
    QMatrix4x4 projection_;

    QOpenGLBuffer vbo_{QOpenGLBuffer::Type::VertexBuffer};
    QOpenGLBuffer ibo_{QOpenGLBuffer::Type::IndexBuffer};
    QOpenGLVertexArrayObject vao_;

    std::unique_ptr<QOpenGLTexture> texture_;
    std::unique_ptr<QOpenGLShaderProgram> program_;

    bool colorAnimated_ = true;
};

inline void CircleRenderer::initialize()
{
    initializeOpenGLFunctions();
    initializeShaderProgram();

    program_->bind();
    initializeTexture();
    initializeUniforms();
    initializeGeometry();
    program_->release();

    timer_.start();
    initializeRenderState();
}

inline void CircleRenderer::render(const std::vector<Circle> &circles, const Camera2D &camera, bool animated)
{
    beginFrame(camera, animated);
    drawCircles(circles);
    endFrame();
}

inline void CircleRenderer::resize(const size_t width, const size_t height)
{
    glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height));
}

inline void CircleRenderer::release()
{
    texture_.reset();
    program_.reset();
    vao_.destroy();
    ibo_.destroy();
    vbo_.destroy();
}

inline void CircleRenderer::updatePaletteRange(const std::vector<Circle> &circles)
{
    paletteRange_ = 1.0f;
    for (const auto &circle : circles) {
        paletteRange_ = std::max(paletteRange_, std::log2(std::abs(circle.curvature)));
    }
}

inline void CircleRenderer::setPalettePower(float power)
{
    palettePower_ = power;
}

inline void CircleRenderer::initializeShaderProgram()
{
    program_ = std::make_unique<QOpenGLShaderProgram>();

    if (!program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/template.vs")) {
        qFatal("Вершинный шейдер: ошибка загрузки или компиляции: %s", qPrintable(program_->log()));
    }

    if (!program_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/template.fs")) {
        qFatal("Фрагментный шейдер: ошибка загрузки или компиляции: %s", qPrintable(program_->log()));
    }

    if (!program_->link()) {
        qFatal("Ошибка линковки шейдерной программы: %s", qPrintable(program_->log()));
    }
}

inline void CircleRenderer::initializeTexture()
{
    texture_ = std::make_unique<QOpenGLTexture>(QImage(":/textures/cat.jpg"));
    texture_->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    texture_->setWrapMode(QOpenGLTexture::WrapMode::Repeat);
    program_->setUniformValue("tex_2d", 0);
}

inline void CircleRenderer::initializeUniforms()
{
    mvpUniform_ = program_->uniformLocation("mvp");
    radiusUniform_ = program_->uniformLocation("circleRadius");
    curvatureUniform_ = program_->uniformLocation("circleCurvature");

    paletteRangeUniform_ = program_->uniformLocation("paletteRange");
    palettePowerUniform_ = program_->uniformLocation("palettePower");

    colorUniform_ = program_->uniformLocation("color");
    program_->setUniformValue(colorUniform_, QColor::fromHsv(0, 0, 255));
}

inline void CircleRenderer::initializeGeometry()
{
    vao_.create();
    vao_.bind();

    vbo_.create();
    vbo_.bind();
    vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vbo_.allocate(vertices_.data(), static_cast<int>(vertices_.size() * sizeof(GLfloat)));

    ibo_.create();
    ibo_.bind();
    ibo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
    ibo_.allocate(indices_.data(), static_cast<int>(indices_.size() * sizeof(GLuint)));

    program_->enableAttributeArray(0);
    program_->setAttributeBuffer(0,
                                 GL_FLOAT,
                                 0,
                                 2,
                                 static_cast<int>(7 * sizeof(GLfloat)));

    program_->enableAttributeArray(1);
    program_->setAttributeBuffer(1,
                                 GL_FLOAT,
                                 static_cast<int>(2 * sizeof(GLfloat)),
                                 3,
                                 static_cast<int>(7 * sizeof(GLfloat)));

    program_->enableAttributeArray(2);
    program_->setAttributeBuffer(2,
                                 GL_FLOAT,
                                 static_cast<int>(5 * sizeof(GLfloat)),
                                 2,
                                 static_cast<int>(7 * sizeof(GLfloat)));

    vao_.release();
    vbo_.release();
    ibo_.release();
}

inline void CircleRenderer::initializeRenderState()
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA,
        GL_ONE,
        GL_ONE_MINUS_SRC_ALPHA
    );
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

inline void CircleRenderer::beginFrame(const Camera2D &camera, bool animated)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    view_ = camera.viewMatrix();
    projection_ = camera.projectionMatrix();

    program_->bind();

    program_->setUniformValue(paletteRangeUniform_, paletteRange_);
    program_->setUniformValue(palettePowerUniform_, palettePower_);

    if (animated && colorAnimated_) {
        const float dt = timer_.restart() / 1000.0f;
        hue_ = std::fmod(hue_ + dt * (60.0f / 360.0f), 1.0f);
        program_->setUniformValue(colorUniform_, QColor::fromHsvF(hue_, 1.0, 1.0));
    }

    glActiveTexture(GL_TEXTURE0);
    texture_->bind();

    vao_.bind();
}

inline void CircleRenderer::drawCircles(const std::vector<Circle> &circles)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    const float pixelHeight = static_cast<float>(viewport[3] > 0 ? viewport[3] : 1);
    const float worldUnitsPerPixel = 2.0f / (projection_(1, 1) * pixelHeight);

    for (const auto &circle : circles) {
        drawCircle(circle, worldUnitsPerPixel);
    }
}

inline void CircleRenderer::drawCircle(const Circle &circle, float worldUnitsPerPixel)
{
    const float radius = 1.0f / std::abs(circle.curvature);
    program_->setUniformValue(radiusUniform_, radius);
    program_->setUniformValue(curvatureUniform_, circle.curvature);
    const float lineWidthPixels = 1.0f;
    const float halfWidth = 0.5f * lineWidthPixels * worldUnitsPerPixel / radius;
    program_->setUniformValue("halfWidth", halfWidth);

    model_.setToIdentity();
    model_.translate(circle.center.x(), circle.center.y(), -1.5f);
    model_.scale(radius, radius, 1.0f);

    const auto mvp = projection_ * view_ * model_;
    program_->setUniformValue(mvpUniform_, mvp);

    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(indices_.size()),
        GL_UNSIGNED_INT,
        nullptr
    );
}

inline void CircleRenderer::endFrame()
{
    vao_.release();
    program_->release();
    texture_->release();
}

