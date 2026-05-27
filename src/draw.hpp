#pragma once

#include "app.hpp"
#include "raylib.h"

void draw3DScene(AppContext& context);
void drawRaylibUI(AppContext& context);
void drawCubes(AppContext const& context, Matrix const& terrainCentering);
void drawDogs(AppContext const &context, Matrix const &terrainCentering);
void drawTrees(AppContext const &context, Matrix const &terrainCentering);

void drawImGui(AppContext& context);