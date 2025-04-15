/*
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "box2d/box2d.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "flecs/private/addons.h"
#include "raylib.h"
#include "raymath.h"

#include "components/mycomponents.h"
#include "lib/utils.h"
#include "systems/onupdate.h"
#include "systems/preupdate.h"

float PHYS_TIME_STEP = 1.0f / 60.0f;

int main() {
  // ------ Setup ------
  int screen_width = 600;
  int screen_height = 400;

  // Raylib
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
  InitWindow(screen_width, screen_height, "Raylib + Flecs + Box2D");

  int curr_mon = GetCurrentMonitor();
  screen_width = GetMonitorWidth(curr_mon);
  screen_height = GetMonitorHeight(curr_mon);

#ifdef __APPLE__
  screen_height -= 65; // Reduce height because of the notch
#endif                 /* ifdef __APPLE__ */

  SetWindowSize(screen_width, screen_height);
  SetWindowPosition(0, 0);

  Camera2D hero_cam;
  hero_cam.target = Vector2Zero();
  hero_cam.rotation = 0.0f;
  hero_cam.zoom = 0.35f;

  // Flecs
  ecs_world_t *world = ecs_init();

  ECS_COMPONENT(world, InputsContext);
  ECS_COMPONENT(world, Position);
  ECS_COMPONENT(world, PhysicsBody);

  ECS_TAG(world, TagControllable);

  ECS_SYSTEM(world, UpdateInputsContextSystem, EcsOnLoad, InputsContext($));
  ECS_SYSTEM(world, ApplyControlsSystem, EcsOnUpdate, Position, PhysicsBody,
             InputsContext($), TagControllable);
  ECS_SYSTEM(world, SyncPhysicsSystem, EcsOnUpdate, Position, PhysicsBody);

  ecs_singleton_set(
      world, InputsContext,
      {0, {false, false, false, false, false, false}, Vector2Zero()});

  // Box2D
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = (b2Vec2){0.0f, -20.0f};
  b2WorldId worldId = b2CreateWorld(&worldDef);

  b2BodyDef groundBodyDef = b2DefaultBodyDef();
  groundBodyDef.position = (b2Vec2){0.0f, -10.0f};
  b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);
  b2Polygon groundBox = b2MakeBox(150.0f, 10.0f);
  b2ShapeDef groundShapeDef = b2DefaultShapeDef();
  groundShapeDef.material.restitution = 0.2f;
  b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = (b2Vec2){0.0f, 10.0f};
  bodyDef.linearDamping = 0.05f;
  bodyDef.angularDamping = 0.4f;
  b2BodyId dynamicId = b2CreateBody(worldId, &bodyDef);
  b2Polygon dynamicBox = b2MakeBox(1.0f, 1.0f);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.density = 1.0f;
  shapeDef.material.restitution = 0.9f;
  b2CreatePolygonShape(dynamicId, &shapeDef, &dynamicBox);

  // Create Flecs entity
  ecs_entity_t ent = ecs_new(world);
  ecs_set(world, ent, Position, {0, 0});
  ecs_set(world, ent, PhysicsBody, {dynamicId});
  ecs_add(world, ent, TagControllable);

  ecs_entity_t ent_arr[2000];

  for (int i = 0; i < 200; ++i) {
    for (int j = 0; j < 10; ++j) {
      b2BodyDef bodyDef2 = b2DefaultBodyDef();
      bodyDef2.type = b2_dynamicBody;
      bodyDef2.position = (b2Vec2){-25 + 5.0f * (j + 1), 15.0f + 5.0f * i};
      bodyDef2.linearDamping = 0.05f;
      bodyDef2.angularDamping = 0.4f;
      b2BodyId dynamicId2 = b2CreateBody(worldId, &bodyDef2);
      b2Polygon dynamicBox2 = b2MakeBox(1.0f, 1.0f);
      b2ShapeDef shapeDef2 = b2DefaultShapeDef();
      shapeDef2.density = 1.0f;
      shapeDef2.material.restitution = 0.9f;
      b2CreatePolygonShape(dynamicId2, &shapeDef2, &dynamicBox2);
      ecs_entity_t ent2 = ecs_new(world);
      ecs_set(world, ent2, Position, {100, 0});
      ecs_set(world, ent2, PhysicsBody, {dynamicId2});
      ent_arr[j + i * 10] = ent2;
    }
  }
  // ------

  float time_acc = 0.0f;
  while (!WindowShouldClose()) {
    // ------ Update ------
    float delta_t = GetFrameTime();
    time_acc += delta_t;

    setScreenDims(&screen_width, &screen_height);

    // Step physics fixed time step at a time
    if (time_acc >= PHYS_TIME_STEP) {
      b2World_Step(worldId, PHYS_TIME_STEP, 4);
      time_acc -= PHYS_TIME_STEP;
    }

    // Update ECS
    ecs_progress(world, delta_t);

    const Position *p = ecs_get(world, ent, Position);
    hero_cam.offset =
        (Vector2){.x = (float)screen_width, .y = (float)screen_height};
    hero_cam.target =
        (Vector2){.x = p->x * 10 - 10, .y = (p->y * 10 - 10) * -1};
    // ------
    // ------ Draw ------
    BeginDrawing();
    ClearBackground(DARKGRAY);

    BeginMode2D(hero_cam);
    DrawRectangle(-1500, 20, 3000, 200, GREEN);
    DrawRectangle(p->x * 10 - 10, (p->y * 10 - 10) * -1, 20, 20, RED);
    for (int i = 0; i < sizeof(ent_arr) / sizeof(ecs_entity_t); ++i) {
      const Position *p2 = ecs_get(world, ent_arr[i], Position);
      const PhysicsBody *pb = ecs_get(world, ent_arr[i], PhysicsBody);
      DrawRectangle(p2->x * 10 - 10, (p2->y * 10 - 10) * -1, 20, 20,
                    b2Body_IsAwake(pb->body) ? BLUE : BLACK);
    }
    EndMode2D();

    DrawFPS(20, 20);
    EndDrawing();
    // ------
  }

  // ------ Cleanup ------
  ecs_fini(world);
  b2DestroyWorld(worldId);
  CloseWindow();
  return EXIT_SUCCESS;
  // ------
}
