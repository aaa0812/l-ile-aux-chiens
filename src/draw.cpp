#include "draw.hpp"

#include "app.hpp"

#include "generation.hpp"

#include "imgui.h"
#include "raylib.h"
#include "raymath.h"

void draw3DScene(AppContext &context)
{
    ClearBackground(RAYWHITE);

    BeginMode3D(context.camera);

    Matrix const terrainCentering{getTerrainCenteringMatrix(context)};
    Vector3 const terrainCenterOffset{terrainCentering.m12, terrainCentering.m13, terrainCentering.m14};

    DrawModel(context.model, terrainCenterOffset, 1.0f, WHITE);
    drawObjects(context, terrainCentering);
    DrawGrid(20, 1.0f);

    EndMode3D();
}

void drawCubes(AppContext const &context, Matrix const &terrainCentering)
{
    if (context.objectParams.empty())
    {
        return;
    }

    float const cubeHalfHeight{0.5f * context.cubeScale};

    for (ObjectParams const &obj : context.objectParams)
    {
        Matrix const objectTranslation{MatrixTranslate(
            obj.pos.x * context.terrainSize.x,
            obj.pos.z * context.terrainSize.y + cubeHalfHeight,
            obj.pos.y * context.terrainSize.z)};
        Matrix const centeredTranslation{MatrixMultiply(objectTranslation, terrainCentering)};
        Matrix const scale{MatrixScale(context.cubeScale, context.cubeScale, context.cubeScale)};
        Matrix const transform{MatrixMultiply(scale, centeredTranslation)};
        DrawMesh(context.cube, context.cubeMaterial, transform);
    }
}

void drawObjects(AppContext const &context, Matrix const &terrainCentering)
{
    if (context.objectParams.empty())
    {
        return;
    }

    for (ObjectParams const &obj : context.objectParams)
    {
        Vector3 initPos = {0, 0, 0}; // need this to apply matrices
        Matrix const objectTranslation{MatrixTranslate(
            obj.pos.x * context.terrainSize.x,
            obj.pos.z * context.terrainSize.y,
            obj.pos.y * context.terrainSize.z)};
        Matrix const centeredTranslation{MatrixMultiply(objectTranslation, terrainCentering)};
        Vector3 objPos = Vector3Transform(initPos, centeredTranslation); // apply matrices to Vector3
        switch (obj.nature)
        {
        case WINTER_TREE1:
            DrawModelEx(context.winterTree1, objPos, Vector3{0, 1, 0}, obj.angle, Vector3(obj.scale, obj.scale, obj.scale), WHITE);
            break;
        case WINTER_TREE2:
            DrawModelEx(context.winterTree2, objPos, Vector3{0, 1, 0}, obj.angle, Vector3(obj.scale, obj.scale, obj.scale), WHITE);
            break;
        case CANDYCANE:
            DrawModelEx(context.candyCane, objPos, Vector3{0.1, 1, 0.1}, obj.angle, Vector3(obj.scale, obj.scale, obj.scale), WHITE);
            break;
        case LOLIPOP:
            DrawModelEx(context.lolipop, objPos, Vector3{0.1, 1, 0.1}, obj.angle, Vector3(obj.scale, obj.scale, obj.scale), WHITE);
            break;
        case BOAT:
            DrawModelEx(context.boat, objPos, Vector3{0, 1, 0}, obj.angle, Vector3(obj.scale, obj.scale, obj.scale), WHITE);
            break;
        case CANDYLH:
            DrawModelEx(context.candyLighthouse, objPos, Vector3{0, 1, 0}, obj.angle, Vector3(obj.scale, obj.scale, obj.scale), WHITE);
            break;
        case DARKLH:
            DrawModelEx(context.darkLightHouse, objPos, Vector3{0, 1, 0}, obj.angle, Vector3(obj.scale, obj.scale, obj.scale), WHITE);
            break;

        default:
            break;
        }
    }
}

void drawImGui(AppContext &context)
{
    if (ImGui::Button("Generate random positions"))
    {
        generateObjectsPositions(context);
    }

    if (ImGui::CollapsingHeader("Couleurs", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Changer de mode"))
        {
            context.islandColors.lightMode ? context.islandColors.setColorsToDark() : context.islandColors.setColorsToLight();
            generateObjectsPositions(context);
            generateHeightmap(context);
        }
    }

    if (ImGui::CollapsingHeader("objects", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderFloat("Cube Scale", &context.cubeScale, 0.01f, 1.0f);
    }
    // SLIDER BRUIT FRACTAL
    if (ImGui::CollapsingHeader("Bruit Fractal", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderInt("Octaves", &context.octaves, 1, 10);
        ImGui::SliderFloat("Lacunarity", &context.lacunarity, 1.0f, 2.0f);
        ImGui::SliderFloat("Gain", &context.gain, 0.1f, 5.0f);
        ImGui::SliderFloat("Scale", &context.imageGenerationParameters.noiseScale, 0.1f, 5.0f);
        ImGui::SliderInt("Resolution", &context.imageGenerationParameters.resolution, 26, 256);
    }

    if (ImGui::Button("Appliquer"))
    {
        generateHeightmap(context);
        regenerateMeshFromImage(context);
        generateObjectsPositions(context);
    }

    // SLIDER HEIGHT MAP
    if (ImGui::CollapsingHeader("Height Map", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderFloat("Amplitude", &context.radius, 0.01f, 0.5f);
        ImGui::SliderFloat("Fit", &context.fit, 0.1f, 5.0f);
    }

    if (ImGui::Button("Generate Height"))
    {
        generateHeightmap(context);
        regenerateMeshFromImage(context);
        generateObjectsPositions(context);
    }
}

void drawRaylibUI(AppContext &context)
{
    int screenWidth{GetScreenWidth()};

    float wanted_size{400.f};
    float scale_factor{wanted_size / std::max(context.texture.width, context.texture.height)};
    float const preview_x{screenWidth - wanted_size - 20.f};
    float const preview_y{20.f};
    float const preview_w{context.texture.width * scale_factor};
    float const preview_h{context.texture.height * scale_factor};
    // DrawTexture(context.texture, screenWidth - context.texture.width - 20, 20, WHITE);
    DrawTextureEx(context.texture, {preview_x, preview_y}, 0.0f, scale_factor, WHITE);
    DrawRectangleLines(screenWidth - wanted_size - 20, 20, wanted_size, wanted_size, GREEN);

    // draw positions on top of the heightmap
    for (auto const &pos : context.objectParams)
    {
        // Remap normalized coordinates [0..1] to the preview image in screen space.
        float const px{preview_x + Clamp(pos.pos.x, 0.0f, 1.0f) * preview_w};
        float const py{preview_y + Clamp(pos.pos.y, 0.0f, 1.0f) * preview_h};

        DrawCircleV({px, py}, 2.0f, RED);
    }

    DrawFPS(10, 10);
}
