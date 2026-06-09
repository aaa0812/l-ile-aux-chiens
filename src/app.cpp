#include "app.hpp"
#include "raymath.h"
#include "utils/raylibUtils.hpp"

void unload(AppContext &context)
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

    if (context.model.meshCount > 0)
    {
        UnloadModel(context.model);
        context.model = {};
        context.mesh = {};
    }

    if (context.cubeMaterial.shader.id > 0)
    {
        UnloadMaterial(context.cubeMaterial);
        context.cubeMaterial = {};
    }

    if (context.cube.vertexCount > 0)
    {
        UnloadMesh(context.cube);
        context.cube = {};
    }

    if (context.winterTree1.meshCount > 0)
    {
        UnloadModel(context.winterTree1);
        context.winterTree1 = {};
    }

    if (context.winterTree2.meshCount > 0)
    {
        UnloadModel(context.winterTree2);
        context.winterTree2 = {};
    }

    if (context.boat.meshCount > 0)
    {
        UnloadModel(context.boat);
        context.boat = {};
    }

    if (context.lolipop.meshCount > 0)
    {
        UnloadModel(context.lolipop);
        context.lolipop = {};
    }
    
    if (context.candyCane.meshCount > 0)
    {
        UnloadModel(context.candyCane);
        context.candyCane = {};
    }

    if (context.candyLighthouse.meshCount > 0)
    {
        UnloadModel(context.candyLighthouse);
        context.candyLighthouse = {};
    }

    if (context.darkLightHouse.meshCount > 0)
    {
        UnloadModel(context.darkLightHouse);
        context.darkLightHouse = {};
    }
}

Matrix getTerrainCenteringMatrix(AppContext const &context)
{
    glm::vec3 const half_size{context.terrainSize * 0.5f};
    return MatrixTranslate(-half_size.x, 0.0f, -half_size.z);
}

void regenerateMeshFromImage(AppContext &context)
{

    if (context.model.meshCount > 0)
    {
        UnloadModel(context.model);
        context.model = {};
        context.mesh = {};
    }

    // We need to convert the heightmap image to a format that GenMeshHeightmap can use (uncompressed R8G8B8A8), so we create a new image and convert the height values to grayscale colors.
    Image const meshHeightImage = TransformImage<float, Color, Colors>(
        context.heightmapImage,
        [](float const &h, int const, int const, Colors const &)
        {
            unsigned char const v{static_cast<unsigned char>(Clamp(h, 0.0f, 1.0f) * 255.0f)};
            return Color{v, v, v, 255};
        },
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, context.islandColors);

    context.mesh = GenMeshHeightmap(meshHeightImage, vec_from_glm(context.terrainSize));
    UnloadImage(meshHeightImage);
    context.model = LoadModelFromMesh(context.mesh);

    context.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = context.texture;
}

void Colors::setColorsToDark()
{
    lightMode = false;
    darkWater = {0.0f, {12, 9, 22}};   // water from 0 to 0.3
    lightWater = {0.3f, {119, 12, 9}}; // water from 0 to 0.3
    foam = {0.35f, {12, 9, 22}};       // transition from 0.3 to 0.35
    sand = {0.4f, {12, 9, 22}};        // beach from 0.35 to 0.45
    dirt = {0.8f, {38, 38, 43}};       // top color from 0.6 to 1 (transition from 0.45 to 0.6)
}

void Colors::setColorsToLight()
{
    lightMode = true;
    darkWater = {0.0f, {84, 153, 207}};
    lightWater = {0.3f, {116, 200, 194}};
    foam = {0.35f, {235, 223, 152}};
    sand = {0.4f, {235, 223, 152}};
    dirt = {0.8f, {228, 160, 161}};
}