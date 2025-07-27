#pragma once

#include "box2d/box2d.h"
#include "flecs.h"
#include "raylib.h"

void SyncPhysicsSystem(ecs_iter_t *it);
void ApplyControlsSystem(ecs_iter_t *it);
void UpdateAnimationSystem(ecs_iter_t *it);
void UpdatePlayerCameraSystem(ecs_iter_t *it);
void UpdateTilemapSystem(ecs_iter_t *it);
void UpdateTilePickerSystem(ecs_iter_t *it);
