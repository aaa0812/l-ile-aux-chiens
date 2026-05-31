#pragma once

#include "raylib.h"
#include "glm/glm.hpp"
#include "utils/rand.hpp"
#include <vector>

struct ImageGenerationParameters
{
    int noiseSeed{GetRandomValue(0, 1000)};
    float noiseScale{4.0f};
    int resolution{256};
};

struct PointsGenerationParameters
{
    // TODO(student): add parameters for points generation (ex: poisson disk radius, etc).

    float r{0.03}; // minimum distance between samples
    int limit{30}; // limit of samples to choose before rejection (constant k)
};

enum Nature
{
    SUMMER_TREE1,
    SUMMER_TREE2,
    WINTER_TREE1,
    WINTER_TREE2,
    BOAT,
    CANDYCANE,
    LOLIPOP
};

struct ObjectParams
{
    glm::vec3 pos{};
    int angle{};
    float scale{};
    Nature nature{SUMMER_TREE1};
};

struct Colors
{
    bool lightMode{true};

    std::pair<float, glm::vec3> darkWater{0.0f, {12, 9, 22}};   // water from 0 to 0.3
    std::pair<float, glm::vec3> lightWater{0.3f, {119, 12, 9}};   // water from 0 to 0.3
    std::pair<float, glm::vec3> foam{0.35f, {12, 9, 22}}; // transition from 0.3 to 0.35
    std::pair<float, glm::vec3> sand{0.4f, {12, 9, 22}}; // beach from 0.35 to 0.45
    std::pair<float, glm::vec3> dirt{0.8f, {38, 38, 43}};  // top color from 0.6 to 1 (transition from 0.45 to 0.6)

    void setColorsToDark();
    void setColorsToLight();
};

struct AppContext
{
    Camera camera{};

    // Store the heightmap as a raylib Image, which is easy to sample from CPU side when generating object positions.
    Image heightmapImage{};

    // This is the image we use for texturing the terrain. It can be the same as heightmapImage, but it doesn't have to be (for example, you could use a color image with RGB channels representing different material types instead of height).
    Image image{};

    // The generated texture from the image, stored here so we can easily bind it when generating the model.
    Texture2D texture{};

    glm::vec3 terrainSize{16.0f, 5.0f, 16.0f};

    // The generated terrain mesh and model.
    Mesh mesh{};
    Model model{};

    std::vector<ObjectParams> objectParams{};

    // A simple cube mesh and material we use to draw objects on the terrain.
    Mesh cube{};
    Material cubeMaterial{};
    float cubeScale{0.1f};

    Model dog{};
    Material dogMaterial{};
    float dogScale{0.1f};

    Model winterTree1{};
    Model winterTree2{};

    Model summerTree1{};
    Model summerTree2{};

    Model autumnTree1{};
    Model autumnTree2{};
    
    Model springTree1{};
    Model springTree2{};

    Model candyCane{};
    Model lolipop{};
    
    Model boat{};

    Colors islandColors;

    // Parameters for object positions generation
    PointsGenerationParameters pointsGenerationParameters;

    // Parameters for island generation
    ImageGenerationParameters imageGenerationParameters;

    int octaves{4};
    float lacunarity{1.3f};
    float gain{2.f};

    float radius{0.3f};
    float fit{1.0f};
};

Matrix getTerrainCenteringMatrix(AppContext const &context);
float sampleHeightmap(AppContext const &context, float u, float v);
void unload(AppContext &context);
void regenerateMeshFromImage(AppContext &context);