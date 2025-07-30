#pragma once

#include "box2d/id.h"
#include "flecs.h"

#include "components/components.h"
#include "flecs/addons/flecs_c.h"

ECS_COMPONENT_DECLARE(b2WorldId);
ECS_COMPONENT_DECLARE(b2BodyId);
ECS_COMPONENT_DECLARE(b2BodyType);
ECS_COMPONENT_DECLARE(VirtualScreen);
ECS_COMPONENT_DECLARE(Vector2);
ECS_COMPONENT_DECLARE(Rotation);
ECS_COMPONENT_DECLARE(Gravity);
ECS_COMPONENT_DECLARE(BoxDimensions);
ECS_COMPONENT_DECLARE(InputsContext);
ECS_COMPONENT_DECLARE(AssetStore);
ECS_COMPONENT_DECLARE(DisplayData);
ECS_COMPONENT_DECLARE(Drawable);
ECS_COMPONENT_DECLARE(PhysicsWorld);
ECS_COMPONENT_DECLARE(PhysicsBody);
ECS_COMPONENT_DECLARE(PhysicsBodyShape);
ECS_COMPONENT_DECLARE(PhysicsBodyId);
ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Velocity);
ECS_COMPONENT_DECLARE(Animation);
ECS_COMPONENT_DECLARE(TextureChoice);
ECS_COMPONENT_DECLARE(TextureConfig);
ECS_COMPONENT_DECLARE(Tile);
ECS_COMPONENT_DECLARE(TilePicker);
ECS_COMPONENT_DECLARE(PlayerCamera);
ECS_COMPONENT_DECLARE(GuiLayoutJungleState);
ECS_COMPONENT_DECLARE(Color);

ECS_TAG_DECLARE(TagControllable);
ECS_TAG_DECLARE(TagStatic);
ECS_TAG_DECLARE(TagCube);
ECS_TAG_DECLARE(TagTile);

void MainModuleImport(ecs_world_t *world);

// Creates an new entity and a new physical box for it.
ecs_entity_t createEntityWithPhysicalBox(ecs_world_t *world,
                                         b2WorldId b2_world_id, Position p,
                                         Rotation r, Velocity v,
                                         BoxDimensions box_dims,
                                         PhysicsBody physics_body,
                                         PhysicsBodyShape physics_shape);

// Creates a new physical box for an existing entity.
void createPhysicalBoxForEntity(ecs_world_t *world, ecs_entity_t ent,
                                b2WorldId b2_world_id);
