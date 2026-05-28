
#include "raylib.h"
#include "rlImGui.h"

#include "app.hpp"
#include "draw.hpp"
#include "generation.hpp"
#include <filesystem>
#include "utils/pathUtils.hpp"

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "IMAC island viewer");

    // Force a regular resizable window state at startup.
    ClearWindowState(FLAG_FULLSCREEN_MODE);
    SetWindowMinSize(800, 450);

    int const monitor { GetCurrentMonitor() };
    int const monitor_width { GetMonitorWidth(monitor) };
    int const monitor_height { GetMonitorHeight(monitor) };
    if (monitor_width > 0 && monitor_height > 0) {
        SetWindowSize(monitor_width/2, monitor_height/2);
        SetWindowPosition(monitor_width/4, monitor_height/4);
    }
    SetTargetFPS(30);

    rlImGuiSetup(true);

    AppContext context {};

    context.cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    context.cubeMaterial = LoadMaterialDefault();
    context.cubeMaterial.maps[MATERIAL_MAP_DIFFUSE].color = RED;
    
    // Define our custom camera to look into our 3d world
    context.camera = {
        .position={ 18.0f, 21.0f, 18.0f },
        .target={ 0.0f, 0.0f, 0.0f },
        .up={ 0.0f, 1.0f, 0.0f },
        .fovy=45.0f,
        .projection=CAMERA_PERSPECTIVE
    };

    // loading texture from file exemple, not used for now since we generate the heightmap texture from code, but it can be useful for testing or as a fallback
    // std::filesystem::path path { pathUtils::make_absolute_path("resources/heightmap.png") };
    // context.image = LoadImage(path.string().c_str());     // Load heightmap image (RAM)
    // context.texture = LoadTextureFromImage(context.image);        // Convert image to texture (VRAM)
    // context.mesh = GenMeshHeightmap(context.image, context.terrainSize); // Generate heightmap mesh (RAM and VRAM)
    // context.model = LoadModelFromMesh(context.mesh);                  // Load model from generated mesh
    // context.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = context.texture; // Set map diffuse texture
    // std::filesystem::path path { pathUtils::make_absolute_path("resources/chien.glb") };
    /* std::filesystem::path path { pathUtils::make_absolute_path("resources/chien.glb") };
    std::filesystem::path textPath { pathUtils::make_absolute_path("resources/heightmap.png") };
    context.dog = LoadModel(path.string().c_str());
    Texture2D texture = LoadTexture(textPath.string().c_str());         // Load model texture and set material
    context.dog.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture; */
    
    std::filesystem::path treePath { pathUtils::make_absolute_path("resources/snow-dead-tree.glb") };
    context.winterTree1 = LoadModel(treePath.string().c_str());

    std::filesystem::path tree2Path { pathUtils::make_absolute_path("resources/snow-dead-tree2.glb") };
    context.winterTree2 = LoadModel(tree2Path.string().c_str());

    std::filesystem::path boatPath { pathUtils::make_absolute_path("resources/boat.glb") };
    context.boat = LoadModel(boatPath.string().c_str());
    
    std::filesystem::path candycanePath { pathUtils::make_absolute_path("resources/models/candy.glb") };
    std::filesystem::path lolipopPath { pathUtils::make_absolute_path("resources/models/lolipop.glb") };
    context.candyCane = LoadModel(candycanePath.string().c_str());
    context.lolipop = LoadModel(lolipopPath.string().c_str());

    generateHeightmap(context);
    regenerateMeshFromImage(context);
    generateObjectsPositions(context);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateCamera(&context.camera, CAMERA_ORBITAL);
        BeginDrawing();

        ClearBackground(RAYWHITE);
        draw3DScene(context);
        drawRaylibUI(context);

        rlImGuiBegin();
        drawImGui(context);
        rlImGuiEnd();

        EndDrawing();
    }

    unload(context);
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}