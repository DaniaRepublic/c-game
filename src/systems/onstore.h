#pragma once

#include "flecs.h"

void BeginDrawingSystem(ecs_iter_t *it);
void EndDrawingSystem(ecs_iter_t *it);
void BeginCameraMode(ecs_iter_t *it);
void EndCameraMode(ecs_iter_t *it);
void DrawBackground(ecs_iter_t *it);
void DrawColliders(ecs_iter_t *it);
void DrawAnimations(ecs_iter_t *it);
void DrawGridSystem(ecs_iter_t *it);
void DrawTilemap(ecs_iter_t *it);
void DrawHoldingTile(ecs_iter_t *it);
void DrawTilePicker(ecs_iter_t *it);
void DrawGui(ecs_iter_t *it);
