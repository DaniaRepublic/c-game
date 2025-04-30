#pragma once

#include "flecs.h"

void DrawBackground(ecs_iter_t *it);
void BeginCameraMode(ecs_iter_t *it);
void DrawColliders(ecs_iter_t *it);
void DrawAnimations(ecs_iter_t *it);
void EndCameraMode(ecs_iter_t *it);
