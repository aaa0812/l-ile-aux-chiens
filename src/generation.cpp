#include "generation.hpp"

#include "noise.hpp"
#include "raylib.h"

#include "utils/raylibUtils.hpp"
#include <algorithm> // for std::clamp
#include <iostream>
#include <math.h>
#include "utils/rand.hpp"
// #include <corecrt_math_defines.h>

std::vector<glm::vec2> generate2DPositions([[maybe_unused]] PointsGenerationParameters const &params)
{
    const float WIDTH = 1.f;
    const float HEIGHT = 1.f;
    const float cellSize = params.r / sqrt(2);      // We pick the cell size to be bounded by r/√n, so that each grid cell will contain at most one sample
    const int nbCols = std::ceil(WIDTH / cellSize); // on arrondit tjr au dessus pour pas avoir 2 points dans la même case
    const int nbRows = std::ceil(HEIGHT / cellSize);

    std::vector<glm::vec2> positions{};
    std::vector<glm::vec2> activeList{};
    std::vector<std::vector<int>> grid(nbRows, std::vector<int>(nbCols, -1)); // background grid for storing samples and accelerating spatial searches (-1 = no samples)
    glm::vec2 randomInitPos = {GetRandomFloat(0.f, 1.f), GetRandomFloat(0.f, 1.f)};

    // int index = static_cast<int>(randomInitPos.x / cellSize) + static_cast<int>(randomInitPos.y / cellSize) * nbCols;
    positions.push_back(randomInitPos);
    activeList.push_back(randomInitPos);
    grid[static_cast<int>(randomInitPos.x / cellSize)][static_cast<int>(randomInitPos.y / cellSize)] = positions.size() - 1;

    // TODO(student): implement Poisson disk sampling to replace the above naive random generation
    // points output should be in [0..1] range, where (0,0) is one corner of the terrain and (1,1) is the opposite corner, so they can be easily scaled to terrain size and sampled from heightmap.
    while (activeList.size() > 0)
    {
        int n = GetRandomValue(0, activeList.size() - 1);
        glm::vec2 activePos = activeList[n];
        bool found{false};

        for (int i = 0; i < params.limit; i++)
        {
            float randAngle = GetRandomFloat(0, 2 * M_PI);
            float dist = GetRandomFloat(params.r, 2 * params.r); // distance at which the point will be
            glm::vec2 dir = {cos(randAngle), sin(randAngle)};    // direction of the point

            glm::vec2 candidate = activePos + dir * dist;

            if (IsValid(candidate, cellSize, params.r, positions, grid))
            {
                positions.push_back(candidate);
                activeList.push_back(candidate);
                int candidateX = std::max(0.f, std::min(static_cast<float>(grid[1].size() - 1), static_cast<float>(candidate.x / cellSize)));
                int candidateY = std::max(0.f, std::min(static_cast<float>(grid[1].size() - 1), static_cast<float>(candidate.y / cellSize)));
                grid[candidateX][candidateY] = positions.size() - 1;
                found = true;
                break;
            }
        }

        if (!found)
        {
            activeList.erase(activeList.begin() + n);
        }
    }

    return positions;
}

bool IsValid(glm::vec2 candidate, float cellSize, float radius, std::vector<glm::vec2> points, std::vector<std::vector<int>> grid)
{
    if (candidate.x >= 0 && candidate.x <= 1 && candidate.y >= 0 && candidate.y <= 1)
    {
        int cellX = (int)(candidate.x / cellSize);
        int cellY = (int)(candidate.y / cellSize);
        int searchStartX = std::max(0, cellX - 2);
        int searchEndX = std::min(cellX + 2, static_cast<int>(grid[1].size() - 1));
        int searchStartY = std::max(0, cellY - 2);
        int searchEndY = std::min(cellY + 2, static_cast<int>(grid[1].size() - 1));

        for (int x{searchStartX}; x <= searchEndX; x++)
        {
            for (int y{searchStartY}; y <= searchEndY; y++)
            {
                int pointIndex = grid[x][y];
                if (pointIndex != -1)
                {
                    float sqrDst = glm::distance(candidate, points[pointIndex]);
                    if (sqrDst < radius)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }
    return false;
}

void generateObjectsPositions(AppContext &context)
{
    std::vector<glm::vec2> const positions{generate2DPositions(context.pointsGenerationParameters)};

    context.objectPositions.clear();
    context.objectPositions.reserve(positions.size());
    for (glm::vec2 const &pos : positions)
    {
        float z = sampleHeightmap(context, pos.x, pos.y);
        if (z > 0.3)
        {
            context.objectPositions.emplace_back(
                pos.x,
                pos.y,
                z);
        }
    }
}

float sampleHeightmap(AppContext const &context, float u, float v)
{
    if (!context.heightmapImage.data || context.heightmapImage.width <= 0 || context.heightmapImage.height <= 0)
        return 0.0f;

    int const px = std::clamp(static_cast<int>(u * static_cast<float>(context.heightmapImage.width - 1)), 0, context.heightmapImage.width - 1);
    int const py = std::clamp(static_cast<int>(v * static_cast<float>(context.heightmapImage.height - 1)), 0, context.heightmapImage.height - 1);

    // If the heightmap is in R32 format, we can directly read the height value as a float.
    if (context.heightmapImage.format == PIXELFORMAT_UNCOMPRESSED_R32)
    {
        float const *heightData = static_cast<float const *>(context.heightmapImage.data);
        int const idx = py * context.heightmapImage.width + px;
        return std::clamp(heightData[idx], 0.0f, 1.0f);
    }

    // Otherwise, we assume it's in a color format and we read the red channel as height (with normalization from [0..255] to [0..1]).
    Color const c = GetImageColor(context.heightmapImage, px, py);
    return static_cast<float>(c.r) / 255.0f;
}

void generateHeightmap(AppContext &context)
{

    if (context.texture.id > 0)
    {
        UnloadTexture(context.texture);
        context.texture = {};
    }

    if (context.image.data)
    {
        UnloadImage(context.image);
        context.image = {};
    }

    if (context.heightmapImage.data)
    {
        UnloadImage(context.heightmapImage);
        context.heightmapImage = {};
    }

    int const resolution = std::max(1, context.imageGenerationParameters.resolution);

    context.heightmapImage = GenImageFromNoiseFunction<float>(resolution, resolution, PIXELFORMAT_UNCOMPRESSED_R32,
                                                              [&](glm::vec2 const &p) -> float
                                                              {
                                                                  // TODO(student): implement stack based noise and island mask

                                                                  return (octaveNoise(p, perlinNoiseSeeded, context.octaves, context.lacunarity, context.gain, context.imageGenerationParameters.noiseScale, context.imageGenerationParameters.noiseSeed) * 0.5f + 0.5f);
                                                              });

    // exemple conversion from heightmap to color image
    context.image = TransformImage<float, Color>(context.heightmapImage, calculateColors, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    context.texture = LoadTextureFromImage(context.image);
    if (context.model.meshCount > 0)
    {
        context.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = context.texture;
    }
}

Color calculateColors(float const &v, int const, int const)
{
    float const waterLimit{0.3f};
    float const foamLimit{0.35f};
    float const sandLimit{0.45f};

    if (v < waterLimit)
    {
        return color_from({103, 119, 121});
    }
    else if (v < foamLimit)
    {
        glm::vec3 min = {103, 119, 121};
        glm::vec3 max = {189, 163, 76};
        return color_from(interpolateVec(std::pair{waterLimit, min}, std::pair{foamLimit, max}, v)); // water
    }
    else if (v < sandLimit)
    {
        return color_from({189, 163, 76}); // grass
    }
    else if (v < 0.6f)
    {
        glm::vec3 min = {189, 163, 76};
        glm::vec3 max = {226, 215, 177};
        return color_from(interpolateVec(std::pair{sandLimit, min}, std::pair{0.6, max}, v)); // transition
    }
    else
    {
        return color_from({226, 215, 177}); // sand
    }
}

glm::vec3 interpolateVec(std::pair<float, glm::vec3> min, std::pair<float, glm::vec3> max, float x)
{
    // interpolation formula : (y) = y1 + [(x-x1) × (y2-y1)]/ (x2-x1)
    return min.second + ((x - min.first) * (max.second - min.second)) / (max.first - min.first);
}