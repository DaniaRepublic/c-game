#include "box2d/types.h"
#include "flecs/addons/flecs_c.h"
#include "flecs/private/api_defines.h"

#include "components/components.h"
#include "systems/onload.h"
#include "systems/onstore.h"
#include "systems/onupdate.h"

#include "module.h"

ECS_COMPONENT_DECLARE(b2BodyId);
ECS_COMPONENT_DECLARE(b2BodyType);
ECS_COMPONENT_DECLARE(Gravity);
ECS_COMPONENT_DECLARE(Vec2);
ECS_COMPONENT_DECLARE(BoxDimensions);
ECS_COMPONENT_DECLARE(InputsContext);
ECS_COMPONENT_DECLARE(AssetStore);
ECS_COMPONENT_DECLARE(Drawable);
ECS_COMPONENT_DECLARE(PhysicsBody);
ECS_COMPONENT_DECLARE(PhysicsBodyShape);
ECS_COMPONENT_DECLARE(PhysicsBodyId);
ECS_COMPONENT_DECLARE(Position);

ECS_TAG_DECLARE(TagControllable);
ECS_TAG_DECLARE(TagStatic);
ECS_TAG_DECLARE(TagCube);

void MainModuleImport(ecs_world_t *world) {
  ECS_MODULE(world, MainModule);

  ECS_COMPONENT_DEFINE(world, InputsContext);
  ECS_COMPONENT_DEFINE(world, AssetStore);
  ECS_COMPONENT_DEFINE(world, Drawable);

  ECS_COMPONENT_DEFINE(world, Vec2);
  ecs_struct(world, {.entity = ecs_id(Vec2),
                     .members = {{.name = "x", .type = ecs_id(ecs_f32_t)},
                                 {.name = "y", .type = ecs_id(ecs_f32_t)}}});

  ECS_COMPONENT_DEFINE(world, Position);
  ecs_struct(world, {.entity = ecs_id(Position),
                     .members = {{.name = "x", .type = ecs_id(ecs_f32_t)},
                                 {.name = "y", .type = ecs_id(ecs_f32_t)}}});

  ECS_COMPONENT_DEFINE(world, Gravity);
  ecs_struct(world, {.entity = ecs_id(Gravity),
                     .members = {{.name = "x", .type = ecs_id(ecs_f32_t)},
                                 {.name = "y", .type = ecs_id(ecs_f32_t)}}});

  ECS_COMPONENT_DEFINE(world, BoxDimensions);
  ecs_struct(world, {.entity = ecs_id(BoxDimensions),
                     .members = {{.name = "x", .type = ecs_id(ecs_f32_t)},
                                 {.name = "y", .type = ecs_id(ecs_f32_t)}}});

  ECS_COMPONENT_DEFINE(world, b2BodyId);
  ecs_struct(world,
             {.entity = ecs_id(b2BodyId),
              .members = {{.name = "index1", .type = ecs_id(ecs_i32_t)},
                          {.name = "world0", .type = ecs_id(ecs_u16_t)},
                          {.name = "generation", .type = ecs_id(ecs_u16_t)}}});

  ECS_COMPONENT_DEFINE(world, b2BodyType);
  ecs_enum(world, {.entity = ecs_id(b2BodyType),
                   .constants = {
                       {.name = "b2_staticBody", .value = b2_staticBody},
                       {.name = "b2_kinematicBody", .value = b2_kinematicBody},
                       {.name = "b2_dynamicBody", .value = b2_dynamicBody},
                   }});

  ECS_COMPONENT_DEFINE(world, PhysicsBody);
  ecs_struct(world,
             {.entity = ecs_id(PhysicsBody),
              .members = {{.name = "body_type", .type = ecs_id(b2BodyType)}}});

  ECS_COMPONENT_DEFINE(world, PhysicsBodyShape);
  ecs_struct(
      world,
      {.entity = ecs_id(PhysicsBodyShape),
       .members = {{.name = "density", .type = ecs_id(ecs_f32_t)},
                   {.name = "mat_friction", .type = ecs_id(ecs_f32_t)},
                   {.name = "mat_restitution", .type = ecs_id(ecs_f32_t)}}});

  ECS_COMPONENT_DEFINE(world, PhysicsBodyId);
  ecs_struct(world,
             {.entity = ecs_id(PhysicsBodyId),
              .members = {{.name = "body_id", .type = ecs_id(b2BodyId)}}});

  ECS_TAG_DEFINE(world, TagControllable);
  ECS_TAG_DEFINE(world, TagStatic);
  ECS_TAG_DEFINE(world, TagCube);

  ECS_SYSTEM(world, UpdateInputsContextSystem, EcsOnLoad, InputsContext($));
  ECS_SYSTEM(world, ApplyControlsSystem, EcsOnUpdate, Position, PhysicsBodyId,
             InputsContext($), TagControllable);
  ECS_SYSTEM(world, SyncPhysicsSystem, EcsOnUpdate, Position, PhysicsBodyId);
  // ECS_SYSTEM(world, DrawEnemiesSystem, EcsOnStore, Position, PhysicsBody);

  ecs_singleton_set(world, InputsContext, {0});
  ecs_singleton_set(world, AssetStore, {0});
}

ecs_entity_t createEntityWithPhysicalBox(ecs_world_t *world,
                                         b2WorldId b2_world_id, Position p,
                                         BoxDimensions box_dims,
                                         PhysicsBody physics_body,
                                         PhysicsBodyShape physics_shape) {
  ecs_entity_t ent = ecs_new(world);
  ecs_set_id(world, ent, ecs_id(Position), sizeof(Position), &p);
  const Position *position = ecs_get(world, ent, Position);
  ecs_set_id(world, ent, ecs_id(BoxDimensions), sizeof(BoxDimensions),
             &box_dims);
  const BoxDimensions *dims = ecs_get(world, ent, BoxDimensions);
  ecs_set_id(world, ent, ecs_id(PhysicsBody), sizeof(PhysicsBody),
             &physics_body);
  const PhysicsBody *body = ecs_get(world, ent, PhysicsBody);
  ecs_set_id(world, ent, ecs_id(PhysicsBodyShape), sizeof(PhysicsBodyShape),
             &physics_shape);
  const PhysicsBodyShape *shape = ecs_get(world, ent, PhysicsBodyShape);

  b2BodyDef BodyDef = b2DefaultBodyDef();
  BodyDef.type = body->body_type;
  BodyDef.position = (b2Vec2){position->x, position->y};
  b2BodyId Id = b2CreateBody(b2_world_id, &BodyDef);
  b2Polygon Box = b2MakeBox(dims->x, dims->y);
  b2ShapeDef ShapeDef = b2DefaultShapeDef();
  ShapeDef.density = shape->density;
  ShapeDef.material.friction = shape->mat_friction;
  ShapeDef.material.restitution = shape->mat_restitution;
  b2CreatePolygonShape(Id, &ShapeDef, &Box);

  ecs_set(world, ent, PhysicsBodyId, {.body_id = Id});

  return ent;
}

void createPhysicalBoxForEntity(ecs_world_t *world, ecs_entity_t ent,
                                b2WorldId b2_world_id) {
  const Position *position = ecs_get(world, ent, Position);
  const BoxDimensions *dims = ecs_get(world, ent, BoxDimensions);
  const PhysicsBody *body = ecs_get(world, ent, PhysicsBody);
  const PhysicsBodyShape *shape = ecs_get(world, ent, PhysicsBodyShape);

  b2BodyDef BodyDef = b2DefaultBodyDef();
  BodyDef.type = body->body_type;
  BodyDef.position = (b2Vec2){position->x, position->y};
  b2BodyId Id = b2CreateBody(b2_world_id, &BodyDef);
  b2Polygon Box = b2MakeBox(dims->x, dims->y);
  b2ShapeDef ShapeDef = b2DefaultShapeDef();
  ShapeDef.density = shape->density;
  ShapeDef.material.friction = shape->mat_friction;
  ShapeDef.material.restitution = shape->mat_restitution;
  b2CreatePolygonShape(Id, &ShapeDef, &Box);

  ecs_set(world, ent, PhysicsBodyId, {.body_id = Id});

  return;
}
