#include "generation.hpp"

#include "noise.hpp"
#include "raylib.h"

#include "utils/raylibUtils.hpp"
#include <algorithm> // for std::clamp
#include <iostream>
#include <corecrt_math_defines.h>
#include <algorithm>

std::vector<glm::vec2> generate2DPositions([[maybe_unused]] PointsGenerationParameters const &params)
{
    const int WIDTH = 100;
    const int HEIGHT = 100;
    // std::vector<glm::vec2> grid = std::vector<glm::vec2>(1000, {-1, -1}); // background grid for storing samples and accelerating spatial searches (-1 = no samples)
    const int nbCols = WIDTH / params.cellSize;
    const int nbRows = HEIGHT / params.cellSize;
    
    std::vector<glm::vec2> positions{};
    std::vector<glm::vec2> activeList{};
    std::vector<std::vector<int>> grid(nbRows, std::vector<int>(nbCols, -1)); // background grid for storing samples and accelerating spatial searches (-1 = no samples)
    glm::vec2 randomInitPos = {static_cast<float>(GetRandomValue(0, INT_MAX)) / static_cast<float>(INT_MAX), static_cast<float>(GetRandomValue(0, INT_MAX)) / static_cast<float>(INT_MAX)};

    
    positions.reserve(1000);
    positions.emplace_back(randomInitPos);
    activeList.push_back(randomInitPos);
    grid[randomInitPos.x/params.cellSize][randomInitPos.y/params.cellSize] = randomInitPos.x + randomInitPos.y * nbCols;

    // TODO(student): implement Poisson disk sampling to replace the above naive random generation
    // points output should be in [0..1] range, where (0,0) is one corner of the terrain and (1,1) is the opposite corner, so they can be easily scaled to terrain size and sampled from heightmap.
    while (!activeList.empty())
    {
        int n = GetRandomValue(0, activeList.size());
        glm::vec2 activePos = activeList[n];
        bool founded{false};

        for (int i = 0; i < params.limit; i++)
        {
            float randAngle = static_cast<float>(GetRandomValue(0, 2 * M_PI));
            float dist = static_cast<float>(GetRandomValue(params.r, 2 * params.r)); // distance at which the point will be
            glm::vec2 dir = {cos(randAngle), sin(randAngle)};                        // direction of the point

            glm::vec2 candidate = activePos + dir * dist;

            bool valid{true};
            for (int i = -1; i <= 1; i++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    int neighbor = grid[candidate.x / params.cellSize + i][candidate.y / params.cellSize + j];
                    if (neighbor != -1)
                    {
                        float dist = calcDistancePoints(positions[neighbor], candidate);
                        if (dist < params.r)
                        {
                            valid = false;
                        }
                    }
                }
            }

            if (valid)
            {
                founded = true;
                activeList.push_back(candidate);
                positions.push_back(candidate);
                grid[(int)candidate.x / params.cellSize][(int)candidate.y / params.cellSize] = positions.size();
                break;
            }
        }

        if (!founded)
        {
            activeList.erase(activeList.begin() + n);
        }
    }

    return positions;
}

float calcDistancePoints(glm::vec2 p1, glm::vec2 p2)
{
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;

    return sqrt((dx * dx) + (dy * dy));
}

void generateObjectsPositions(AppContext &context)
{
    std::vector<glm::vec2> const positions{generate2DPositions(context.pointsGenerationParameters)};

    context.objectPositions.clear();
    context.objectPositions.reserve(positions.size());
    for (glm::vec2 const &p : positions)
    {
        context.objectPositions.emplace_back(
            p.x, // x
            p.y, // y
            // sample height from heightmap for each point (asuming positions are normalized in [0..1] range)
            sampleHeightmap(context, p.x, p.y));
    }
    // TODO(student): extension - filter positions by sampled height range.
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

                                                                  return (perlinNoiseSeeded(p * context.imageGenerationParameters.noiseScale, context.imageGenerationParameters.noiseSeed) * 0.5f + 0.5f);
                                                              });

    // exemple conversion from heightmap to color image
    context.image = TransformImage<float, Color>(context.heightmapImage, [&](float const &v, int const, int const)
                                                 {
                                                     if (v < 0.3f)
                                                     {
                                                         return color_from({70, 130, 180}); // water
                                                     }
                                                     else if (v < 0.5f)
                                                     {
                                                         return color_from({238, 214, 175}); // sand
                                                     }
                                                     else
                                                     {
                                                         return color_from({34, 139, 34}); // grass
                                                     } }, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    context.texture = LoadTextureFromImage(context.image);
    if (context.model.meshCount > 0)
    {
        context.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = context.texture;
    }
}