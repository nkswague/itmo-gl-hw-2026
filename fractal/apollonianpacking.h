#pragma once

#include "circle.h"

#include <array>
#include <cmath>
#include <map>
#include <vector>

class ApollonianPacking
{
    struct CircleCandidate
    {
        Circle circle;
        std::array<Circle, 3> boundaries{};
        bool fillsGap = false;
    };

    using CircleQueue = std::multimap<float, CircleCandidate>;

public:
    void generateCircles(int generationCount);
    const std::vector<Circle>& circles() const;

private:
    static std::array<Circle, 3> createInitialCircles();
    static void enqueueInitialGaps(
        CircleQueue &pending,
        const std::array<Circle, 3> &initialCircles
    );
    void appendNextGeneration(CircleQueue &pending);
    static void enqueueGap(
        CircleQueue &pending,
        const Circle &a,
        const Circle &b,
        const Circle &c,
        const Circle &known
    );
    static Circle oppositeCircle(
        const Circle &a,
        const Circle &b,
        const Circle &c,
        const Circle &known
    );

    std::vector<Circle> circles_;
};

inline void ApollonianPacking::generateCircles(int generationCount)
{
    const auto initialCircles = createInitialCircles();

    circles_.clear();

    CircleQueue pending;
    for (const auto &circle : initialCircles) {
        pending.emplace(std::abs(circle.curvature), CircleCandidate{circle});
    }

    enqueueInitialGaps(pending, initialCircles);

    for (int generation = 0; generation < generationCount && !pending.empty(); ++generation) {
        appendNextGeneration(pending);
    }
}

inline const std::vector<Circle>& ApollonianPacking::circles() const
{
    return circles_;
}

inline std::array<Circle, 3> ApollonianPacking::createInitialCircles()
{
    const Circle outer{QVector2D(0.0f, 0.0f), -1.0f};
    const Circle left{QVector2D(-0.5f, 0.0f), 2.0f};
    const Circle right{QVector2D(0.5f, 0.0f), 2.0f};
    return {outer, left, right};
}

inline void ApollonianPacking::enqueueInitialGaps(
    CircleQueue &pending,
    const std::array<Circle, 3> &initialCircles
) {
    const auto &outer = initialCircles[0];
    const auto &left = initialCircles[1];
    const auto &right = initialCircles[2];

    const float outerRadius = 1.0f / std::abs(outer.curvature);
    const float sideRadius = 1.0f / std::abs(left.curvature);

    const float halfDistance = (right.center - left.center).length() / 2.0f;
    const float upperRadius = (halfDistance * halfDistance + outerRadius * outerRadius - sideRadius * sideRadius) / (2.0f * (outerRadius + sideRadius));

    const Circle upper{
        outer.center + QVector2D(0.0f, outerRadius - upperRadius),
        1.0f / upperRadius
    };

    pending.emplace(
        std::abs(upper.curvature),
        CircleCandidate{upper, {outer, left, right}, true}
    );

    enqueueGap(
        pending,
        outer,
        left,
        right,
        upper
    );
}

inline void ApollonianPacking::appendNextGeneration(CircleQueue &pending)
{
    const float curvature = pending.begin()->first;
    while (!pending.empty() && pending.begin()->first == curvature) {
        const CircleCandidate candidate = pending.begin()->second;
        pending.erase(pending.begin());
        circles_.push_back(candidate.circle);

        if (!candidate.fillsGap) {
            continue;
        }

        const auto &a = candidate.boundaries[0];
        const auto &b = candidate.boundaries[1];
        const auto &c = candidate.boundaries[2];
        const auto &next = candidate.circle;

        enqueueGap(pending, a, b, next, c);
        enqueueGap(pending, a, c, next, b);
        enqueueGap(pending, b, c, next, a);
    }
}

inline void ApollonianPacking::enqueueGap(
    CircleQueue &pending,
    const Circle &a,
    const Circle &b,
    const Circle &c,
    const Circle &known
) {
    const Circle next = oppositeCircle(a, b, c, known);
    pending.emplace(
        std::abs(next.curvature),
        CircleCandidate{next, {a, b, c}, true}
    );
}

inline Circle ApollonianPacking::oppositeCircle(
    const Circle &a,
    const Circle &b,
    const Circle &c,
    const Circle &known
) {
    const float curvature = 2.0f * (a.curvature + b.curvature + c.curvature)
        - known.curvature;

    const QVector2D weightedCenter = 2.0f * (a.curvature * a.center + b.curvature * b.center + c.curvature * c.center)
        - known.curvature * known.center;

    return {weightedCenter / curvature, curvature};
}
