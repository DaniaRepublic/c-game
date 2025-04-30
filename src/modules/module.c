#include "box2d/box2d.h"
#include "box2d/math_functions.h"
#include "box2d/types.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "flecs/private/addons.h"
#include "flecs/private/api_defines.h"

#include "components/components.h"
#include "lib/utils.h"
#include "module.h"
#include "raylib.h"
#include "systems/onload.h"
#include "systems/onstore.h"
#include "systems/onupdate.h"

ECS_COMPONENT_DECLARE(b2BodyId);
ECS_COMPONENT_DECLARE(b2BodyType);
ECS_COMPONENT_DECLARE(Vector2);
ECS_COMPONENT_DECLARE(Gravity);
ECS_COMPONENT_DECLARE(BoxDimensions);
ECS_COMPONENT_DECLARE(InputsContext);
ECS_COMPONENT_DECLARE(AssetStore);
ECS_COMPONENT_DECLARE(ScreenDims);
ECS_COMPONENT_DECLARE(Drawable);
ECS_COMPONENT_DECLARE(PhysicsBody);
ECS_COMPONENT_DECLARE(PhysicsBodyShape);
ECS_COMPONENT_DECLARE(PhysicsBodyId);
ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Velocity);
ECS_COMPONENT_DECLARE(Animation);
ECS_COMPONENT_DECLARE(TextureConfig);
ECS_COMPONENT_DECLARE(PlayerCamera);
ECS_COMPONENT_DECLARE(GuiLayoutJungleState);
ECS_COMPONENT_DECLARE(Color);

ECS_TAG_DECLARE(TagControllable);
ECS_TAG_DECLARE(TagStatic);
ECS_TAG_DECLARE(TagCube);

void MainModuleImport(ecs_world_t *world) {
  ECS_MODULE(world, MainModule);

  ECS_COMPONENT_DEFINE(world, InputsContext);
  ECS_COMPONENT_DEFINE(world, AssetStore);
  ECS_COMPONENT_DEFINE(world, ScreenDims);
  ecs_struct(world, {.entity = ecs_id(ScreenDims),
                     .members = {{.name = "x", .type = ecs_id(ecs_f32_t)},
                                 {.name = "y", .type = ecs_id(ecs_f32_t)}}});
  ECS_COMPONENT_DEFINE(world, TextureConfig);
  ECS_COMPONENT_DEFINE(world, Drawable);
  ECS_COMPONENT_DEFINE(world, Vector2);
  ecs_struct(world, {.entity = ecs_id(Vector2),
                     .members = {{.name = "x", .type = ecs_id(ecs_f32_t)},
                                 {.name = "y", .type = ecs_id(ecs_f32_t)}}});
  ECS_COMPONENT_DEFINE(world, Position);
  ecs_struct(world, {.entity = ecs_id(Position),
                     .members = {{.name = "x", .type = ecs_id(ecs_f32_t)},
                                 {.name = "y", .type = ecs_id(ecs_f32_t)}}});
  ECS_COMPONENT_DEFINE(world, Velocity);
  ecs_struct(world, {.entity = ecs_id(Velocity),
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
  ECS_COMPONENT_DEFINE(world, PlayerCamera);
  ecs_struct(world,
             {.entity = ecs_id(PlayerCamera),
              .members = {{.name = "offset", .type = ecs_id(Vector2)},
                          {.name = "target", .type = ecs_id(Vector2)},
                          {.name = "rotation", .type = ecs_id(ecs_f32_t)},
                          {.name = "zoom", .type = ecs_id(ecs_f32_t)}}});
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
  ECS_COMPONENT_DEFINE(world, Animation);
  ecs_struct(
      world,
      {.entity = ecs_id(Animation),
       .members = {{.name = "frames",
                    .type = ecs_array(world, {.count = NUM_ANIMATION_FRAMES,
                                              .type = ecs_id(ecs_i32_t)})},
                   {.name = "curr_frame_idx", .type = ecs_id(ecs_i32_t)},
                   {.name = "total_frames", .type = ecs_id(ecs_i32_t)},
                   {.name = "frame_duration", .type = ecs_id(ecs_f32_t)},
                   {.name = "since_last_frame", .type = ecs_id(ecs_f32_t)}}});
  ECS_COMPONENT_DEFINE(world, Color);
  ecs_struct(world, {.entity = ecs_id(Color),
                     .members = {{.name = "r", .type = ecs_id(ecs_u8_t)},
                                 {.name = "g", .type = ecs_id(ecs_u8_t)},
                                 {.name = "b", .type = ecs_id(ecs_u8_t)},
                                 {.name = "a", .type = ecs_id(ecs_u8_t)}}});
  ECS_COMPONENT_DEFINE(world, GuiLayoutJungleState);
  ecs_struct(
      world,
      {.entity = ecs_id(GuiLayoutJungleState),
       .members = {
           {.name = "anchor01", .type = ecs_id(Vector2)},
           {.name = "WindowBox000Active", .type = ecs_id(ecs_bool_t)},
           {.name = "Slider001Value", .type = ecs_id(ecs_f32_t)},
           {.name = "ColorPicker003Value", .type = ecs_id(Color)},
           {.name = "CheckBoxEx006Checked", .type = ecs_id(ecs_bool_t)}}});

  ecs_singleton_set(world, InputsContext, {0});
  ecs_singleton_set(world, AssetStore, {0});
  ecs_singleton_set(world, ScreenDims, {0});
  ecs_singleton_set(world, TextureConfig, {0});
  ecs_singleton_set(world, GuiLayoutJungleState, {0});

  ECS_TAG_DEFINE(world, TagControllable);
  ECS_TAG_DEFINE(world, TagStatic);
  ECS_TAG_DEFINE(world, TagCube);

  ECS_SYSTEM(world, UpdateInputsContextSystem, EcsOnLoad, InputsContext($));
  ECS_SYSTEM(world, SyncScreenDims, EcsOnLoad, ScreenDims($));
  ECS_SYSTEM(world, ApplyControlsSystem, EcsOnUpdate, PhysicsBodyId,
             InputsContext($), TagControllable);
  ECS_SYSTEM(world, SyncPhysicsSystem, EcsOnUpdate, Position, Velocity,
             PhysicsBodyId);
  ECS_SYSTEM(world, UpdateAnimation, EcsOnUpdate, Animation, InputsContext($),
             AssetStore($));
  ECS_SYSTEM(world, UpdatePlayerCamera, EcsOnUpdate, Position, PlayerCamera,
             ScreenDims($), GuiLayoutJungleState($));
  ECS_SYSTEM(world, DrawBackground, EcsOnStore, GuiLayoutJungleState($));
  // ECS_SYSTEM(world, DrawEnemies, EcsOnStore, Position, PhysicsBody);
}

ecs_entity_t createEntityWithPhysicalBox(ecs_world_t *world,
                                         b2WorldId b2_world_id, Position p,
                                         Velocity v, BoxDimensions box_dims,
                                         PhysicsBody physics_body,
                                         PhysicsBodyShape physics_shape) {
  ecs_entity_t ent = ecs_new(world);
  ecs_set_id(world, ent, ecs_id(Position), sizeof(Position), &p);
  const Position *position = ecs_get(world, ent, Position);
  ecs_set_id(world, ent, ecs_id(Velocity), sizeof(Velocity), &v);
  const Velocity *velocity = ecs_get(world, ent, Velocity);
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
  b2Body_SetLinearVelocity(Id, (b2Vec2){.x = velocity->x, .y = velocity->y});

  ecs_set(world, ent, PhysicsBodyId, {.body_id = Id});

  return ent;
}

void createPhysicalBoxForEntity(ecs_world_t *world, ecs_entity_t ent,
                                b2WorldId b2_world_id) {
  const Position *position = ecs_get(world, ent, Position);
  const Velocity *velocity = ecs_get(world, ent, Velocity);
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
  b2Body_SetLinearVelocity(Id, (b2Vec2){.x = velocity->x, .y = velocity->y});

  ecs_set(world, ent, PhysicsBodyId, {.body_id = Id});

  return;
}
