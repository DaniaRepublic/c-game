#pragma once

#include "flecs.h"

void BeginDrawingSystem(ecs_iter_t *it);
void EndDrawingSystem(ecs_iter_t *it);
void BeginCameraModeSystem(ecs_iter_t *it);
void EndCameraModeSystem(ecs_iter_t *it);
void BeginTextureModeSystem(ecs_iter_t *it);
void EndTextureModeSystem(ecs_iter_t *it);
void DrawBackgroundSystem(ecs_iter_t *it);
void DrawCollidersSystem(ecs_iter_t *it);
void DrawAnimationsSystem(ecs_iter_t *it);
void DrawGridSystem(ecs_iter_t *it);
void DrawTilemapSystem(ecs_iter_t *it);
void DrawHoldingTileSystem(ecs_iter_t *it);
void DrawTilePickerSystem(ecs_iter_t *it);
void DrawGuiSystem(ecs_iter_t *it);
void DrawVirtualScreenSystem(ecs_iter_t *it);
