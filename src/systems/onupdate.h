#pragma once

#include "box2d/box2d.h"
#include "flecs.h"
#include "raylib.h"

void SyncPhysicsSystem(ecs_iter_t *it);
void ApplyControlsSystem(ecs_iter_t *it);
