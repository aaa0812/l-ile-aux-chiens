#pragma once

#include "app.hpp"

std::vector<glm::vec2> generate2DPositions(PointsGenerationParameters const& params);

void generateObjectsPositions(AppContext& context);

float sampleHeightmap(AppContext const& context, float u, float v);

void generateHeightmap(AppContext& context);

bool IsValid(glm::vec2 candidate, float cellSize, float radius, std::vector<glm::vec2> points, std::vector<std::vector<int>> grid);

Color calculateColors(float const &v, int const, int const);

glm::vec3 interpolateVec(std::pair<float, glm::vec3> min, std::pair<float, glm::vec3> max, float x);