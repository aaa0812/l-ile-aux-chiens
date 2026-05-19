#include "rand.hpp"

float GetRandomFloat(float min, float max)
{
    return static_cast<float>(GetRandomValue(0, INT_MAX)) / static_cast<float>(INT_MAX) * (max - min) + min;
}