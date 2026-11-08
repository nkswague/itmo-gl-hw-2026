#pragma once

#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QWidget>

class ControlPanel : public QWidget
{
public:
    explicit ControlPanel(int generationCount, QWidget *parent = nullptr);
    void setFps(double fps);
    QSlider* iterationSlider() const;
    QSlider* paletteSlider() const;

private:
    QLabel* createFpsLabel();
    QWidget* createIterationControls(int generationCount);
    QWidget* createPaletteControls();
    QWidget* createSliderControls(QLabel *label, QSlider *slider);

    QLabel* fpsLabel_ = nullptr;
    QSlider* iterationSlider_ = nullptr;
    QSlider* paletteSlider_ = nullptr;
};

inline ControlPanel::ControlPanel(int generationCount, QWidget *parent)
    : QWidget(parent)
{
    setObjectName("controlPanel");
    setStyleSheet(
        "QWidget#controlPanel { background-color: rgba(0, 0, 0, 160); }"
        "QLabel { color: white; }"
    );

    auto* controlsLayout = new QGridLayout(this);
    controlsLayout->setContentsMargins(8, 4, 8, 4);
    controlsLayout->addWidget(createFpsLabel(), 0, 0);
    controlsLayout->addWidget(createIterationControls(generationCount), 0, 1);
    controlsLayout->addWidget(createPaletteControls(), 1, 1);
    controlsLayout->setColumnStretch(1, 1);
}

inline void ControlPanel::setFps(double fps)
{
    fpsLabel_->setText(QString("FPS: %1").arg(fps, 0, 'f', 0));
}

inline QSlider* ControlPanel::iterationSlider() const
{
    return iterationSlider_;
}

inline QSlider* ControlPanel::paletteSlider() const
{
    return paletteSlider_;
}

inline QLabel* ControlPanel::createFpsLabel()
{
    fpsLabel_ = new QLabel("FPS: 0", this);
    fpsLabel_->setMinimumWidth(80);
    fpsLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    fpsLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
    return fpsLabel_;
}

inline QWidget* ControlPanel::createIterationControls(int generationCount)
{
    auto* label = new QLabel(QString("iterations: %1").arg(generationCount), this);
    iterationSlider_ = new QSlider(Qt::Horizontal, this);
    iterationSlider_->setObjectName("iterations");
    iterationSlider_->setRange(1, 1000);
    iterationSlider_->setSingleStep(1);
    iterationSlider_->setPageStep(1);
    iterationSlider_->setValue(generationCount);

    connect(iterationSlider_, &QSlider::valueChanged, this, [label](int value) {
        label->setText(QString("iterations: %1").arg(value));
    });
    return createSliderControls(label, iterationSlider_);
}

inline QWidget* ControlPanel::createPaletteControls()
{
    auto* label = new QLabel("palette: 1.00", this);
    paletteSlider_ = new QSlider(Qt::Horizontal, this);
    paletteSlider_->setObjectName("palette");
    paletteSlider_->setRange(25, 400);
    paletteSlider_->setSingleStep(1);
    paletteSlider_->setPageStep(25);
    paletteSlider_->setValue(100);

    connect(paletteSlider_, &QSlider::valueChanged, this, [label](int value) {
        label->setText(QString("palette: %1").arg(value / 100.0, 0, 'f', 2));
    });
    return createSliderControls(label, paletteSlider_);
}

inline QWidget* ControlPanel::createSliderControls(QLabel *label, QSlider *slider)
{
    auto* controls = new QWidget(this);
    label->setMinimumWidth(110);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto* controlsLayout = new QHBoxLayout(controls);
    controlsLayout->setContentsMargins(0, 0, 64, 0);
    controlsLayout->addWidget(label);
    controlsLayout->addWidget(slider, 1);
    return controls;
}
