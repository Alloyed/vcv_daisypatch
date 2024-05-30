#pragma once

#include <cmath>

constexpr float cHalfPi = 1.5707964;

inline constexpr float clampf(float in, float min, float max)
{
    return in > max ? max : in < min ? min : in;
}

inline constexpr float lerpf(float a, float b, float t)
{
    return a + (b - a) * t;
}

inline constexpr float fadeCpowf(float a, float b, float t)
{
    return (a * sinf((1.0f - t) * cHalfPi)) + (b * sinf(t * cHalfPi));
}

inline constexpr float roundTof(float in, float increment)
{
    return static_cast<int32_t>(0.5 + (in / increment)) * increment;
}