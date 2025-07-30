#pragma once

#include "flecs.h"

void SyncPhysicsSystem(ecs_iter_t *it);
void ApplyControlsSystem(ecs_iter_t *it);
void UpdateAnimationSystem(ecs_iter_t *it);
void UpdatePlayerCameraSystem(ecs_iter_t *it);
void UpdateTilemapSystem(ecs_iter_t *it);
void UpdateTilePickerSystem(ecs_iter_t *it);
