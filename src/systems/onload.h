#pragma once

#include "box2d/box2d.h"
#include "flecs.h"
#include "raylib.h"

void SyncScreenDims(ecs_iter_t *it);
void UpdateInputsContextSystem(ecs_iter_t *it);
