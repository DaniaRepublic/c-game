/*
 * Da faq me makin'?
 */
#include <stdio.h>
#include <stdlib.h>

#include "box2d/box2d.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "flecs/private/addons.h"
#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "components/mycomponents.h"
#include "lib/utils.h"
#include "systems/onupdate.h"
#include "systems/preupdate.h"

// gui implementations
#include "lib/gui/jungle.h"
#define GUI_LAYOUT_JUNGLE_IMPLEMENTATION
#include "lib/gui/gui_layout_jungle.h"

int main() {
  // ------ Setup ------
  int screen_width = 600;
  int screen_height = 400;

  // Raylib
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
  InitWindow(screen_width, screen_height, "Raylib + Flecs + Box2D");

  Vector2 dpi_factor = GetWindowScaleDPI();
  printf("%f %f\n", dpi_factor.x, dpi_factor.y);

  int curr_mon = GetCurrentMonitor();
  screen_width = GetMonitorWidth(curr_mon);
  screen_height = GetMonitorHeight(curr_mon);

#ifdef __APPLE__
  screen_height -= 65; // Reduce height because of the notch
#endif                 /* ifdef __APPLE__ */

  printf("monitor_height = %d\n", screen_height);
  printf("screen_height = %d\n", GetScreenHeight());
  printf("render_height = %d\n", GetRenderHeight());

  SetWindowSize(screen_width, screen_height);
  SetWindowPosition(0, 0);

  // Init GUI
  GuiLoadStyleJungle();
  GuiLayoutJungleState settings_state = InitGuiLayoutJungle();

  // Flecs
  ecs_world_t *world = ecs_init();

  ECS_COMPONENT(world, InputsContext);
  ECS_COMPONENT(world, AssetStore);
  ECS_COMPONENT(world, Position);
  ECS_COMPONENT(world, PhysicsBody);
  ECS_COMPONENT(world, Drawable);

  ECS_TAG(world, TagControllable);

  ECS_SYSTEM(world, UpdateInputsContextSystem, EcsOnLoad, InputsContext($));
  ECS_SYSTEM(world, ApplyControlsSystem, EcsOnUpdate, Position, PhysicsBody,
             InputsContext($), TagControllable);
  ECS_SYSTEM(world, SyncPhysicsSystem, EcsOnUpdate, Position, PhysicsBody);

  ecs_singleton_set(world, InputsContext, {0});
  ecs_singleton_set(world, AssetStore, {0});

  AssetStore *asset_store = ecs_singleton_ensure(world, AssetStore);

  // load ghost animation
  freeSetTex(TEX_TILEMAP_CONCRETE, LoadTexture("assets/concrete-tiles.png"),
             asset_store);
  SetTextureFilter(asset_store->_textures[TEX_TILEMAP_CONCRETE],
                   TEXTURE_FILTER_POINT);
  freeSetTex(TEX_GHOST1, LoadTexture("assets/slime-ghost1.png"), asset_store);
  SetTextureFilter(asset_store->_textures[TEX_GHOST1], TEXTURE_FILTER_POINT);
  freeSetTex(TEX_GHOST2, LoadTexture("assets/slime-ghost2.png"), asset_store);
  SetTextureFilter(asset_store->_textures[TEX_GHOST2], TEXTURE_FILTER_POINT);
  freeSetTex(TEX_GHOST3, LoadTexture("assets/slime-ghost3.png"), asset_store);
  SetTextureFilter(asset_store->_textures[TEX_GHOST3], TEXTURE_FILTER_POINT);
  freeSetTex(TEX_GHOST4, LoadTexture("assets/slime-ghost4.png"), asset_store);
  SetTextureFilter(asset_store->_textures[TEX_GHOST4], TEXTURE_FILTER_POINT);

  printf("tilemap dims: %d, %d\n",
         asset_store->_textures[TEX_TILEMAP_CONCRETE].width,
         asset_store->_textures[TEX_TILEMAP_CONCRETE].height);
  printf("ghost dims: %d, %d\n", asset_store->_textures[TEX_GHOST1].width,
         asset_store->_textures[TEX_GHOST1].height);
  Tilemap tm_concrete = {
      .tex_choice = TEX_TILEMAP_CONCRETE,
      .tile_w = 32,
      .tile_h = 32,
      .num_x = 3,
      .num_y = 3,
  };

  Animation6 ghost_anim = {
      .frames =
          {
              TEX_GHOST1,
              TEX_GHOST2,
              TEX_GHOST3,
              TEX_GHOST4,
          },
      .curr_frame_idx = 0,
      .total_frames = 4,
      .frame_duration = 0.2,
      .since_last_frame = 0,
  };

  // Box2D
  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = (b2Vec2){0};
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
  ecs_set(world, ent, Position, {0});
  ecs_set(world, ent, PhysicsBody, {dynamicId});
  ecs_add(world, ent, TagControllable);

  ecs_entity_t ent_arr[2500];

  for (int i = 0; i < 50; ++i) {
    for (int j = 0; j < 50; ++j) {
      b2BodyDef bodyDef2 = b2DefaultBodyDef();
      bodyDef2.type = b2_dynamicBody;
      bodyDef2.position = (b2Vec2){-125 + 5.0f * (j + 1), 15.0f + 5.0f * i};
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
      ent_arr[j + i * 50] = ent2;
    }
  }

  Camera2D hero_cam;
  hero_cam.target = Vector2Zero();
  hero_cam.rotation = 0.0f;
  hero_cam.zoom = settings_state.Slider001Value;
  // ------

  float time_acc = 0.0f;
  while (!WindowShouldClose()) {
    // ------ Update ------
    float delta_t = GetFrameTime();
    time_acc += delta_t;

    setScreenDims(&screen_width, &screen_height);

    // Step physics one fixed time step at a time for accumulated #steps
    while (time_acc >= PHYS_TIME_STEP) {
      b2World_Step(worldId, PHYS_TIME_STEP, 4);
      time_acc -= PHYS_TIME_STEP;
    }

    // Progress animations
    deltaTStepAnimation6(&ghost_anim, delta_t, asset_store);

    // Update ECS
    ecs_progress(world, delta_t);

    const Position *p = ecs_get(world, ent, Position);
    hero_cam.offset =
        (Vector2){.x = (float)screen_width / 2, .y = (float)screen_height / 2};
    hero_cam.target =
        (Vector2){.x = p->x * 10 - 10, .y = (p->y * 10 - 10) * -1};
    hero_cam.zoom = settings_state.Slider001Value;
    // ------
    // ------ Draw ------
    BeginDrawing();
    ClearBackground(settings_state.ColorPicker003Value);

    BeginMode2D(hero_cam);
    DrawRectangle(-1500, 20, 3000, 200, GREEN);
    Texture ghost = getCurrFrameAnimation6(ghost_anim, asset_store);
    DrawTexturePro(
        ghost,
        (Rectangle){
            .x = 0, .y = 0, .width = ghost.width, .height = ghost.height},
        (Rectangle){.x = (int)(p->x * 10 - 10),
                    .y = (int)(p->y * 10 - 10) * -1,
                    .width = ghost.width * 4,
                    .height = ghost.height * 4},
        (Vector2){.x = ghost.width * 2, .y = ghost.height * 2}, 0.0f, WHITE);

    for (int i = 0; i < sizeof(ent_arr) / sizeof(ecs_entity_t); ++i) {
      const Position *p2 = ecs_get(world, ent_arr[i], Position);
      const PhysicsBody *pb = ecs_get(world, ent_arr[i], PhysicsBody);
      DrawRectangle(p2->x * 10 - 10, (p2->y * 10 - 10) * -1, 20, 20,
                    b2Body_IsAwake(pb->body) ? BLUE : BLACK);
    }
    EndMode2D();

    for (int i = 0; i < tm_concrete.num_x; ++i) {
      for (int j = 0; j < tm_concrete.num_y; ++j) {
        float offset_x = tm_concrete.tile_w * i;
        float offset_y = tm_concrete.tile_h * j;
        DrawTexturePro(asset_store->_textures[tm_concrete.tex_choice],
                       (Rectangle){
                           .x = offset_x,
                           .y = offset_y,
                           .width = tm_concrete.tile_w,
                           .height = tm_concrete.tile_h,
                       },
                       (Rectangle){
                           .x = (offset_x * tm_concrete.num_x + offset_y) * 2,
                           .y = screen_height - tm_concrete.tile_h * 2,
                           .width = tm_concrete.tile_w * 2,
                           .height = tm_concrete.tile_h * 2,
                       },
                       (Vector2){
                           .x = 0,
                           .y = 0,
                       },
                       0.0f, WHITE);
      }
    }

    // gui
    GuiLayoutJungle(&settings_state);

    DrawFPS(20, 20);
    EndDrawing();
    // ------
  }

  // ------ Cleanup ------
  ecs_fini(world);
  b2DestroyWorld(worldId);
  freeAssetStore(asset_store);
  CloseWindow();
  return EXIT_SUCCESS;
  // ------
}
