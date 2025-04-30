#pragma once

#include "box2d/id.h"
#include "flecs.h"

#include "components/components.h"

void MainModuleImport(ecs_world_t *world);

// Creates an new entity and a new physical box for it.
ecs_entity_t createEntityWithPhysicalBox(ecs_world_t *world,
                                         b2WorldId b2_world_id, Position p,
                                         Velocity v, BoxDimensions box_dims,
                                         PhysicsBody physics_body,
                                         PhysicsBodyShape physics_shape);

// Creates a new physical box for an existing entity.
void createPhysicalBoxForEntity(ecs_world_t *world, ecs_entity_t ent,
                                b2WorldId b2_world_id);
