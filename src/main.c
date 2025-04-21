#include <_stdio.h>
#include <stdio.h>
#include <stdlib.h>

#include "box2d/box2d.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "flecs/private/addons.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "components/mycomponents.h"
#include "lib/utils.h"
#include "systems/onupdate.h"
#include "systems/preupdate.h"

// gui implementations
#include "lib/gui/gui_layout_jungle.h"
#include "lib/gui/jungle.h"

int main() {
  // ------ Setup ------
  int screen_width = 600;
  int screen_height = 400;

  // Raylib
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
  InitWindow(screen_width, screen_height, "Da Game");

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

  // Must be multiple of 2
  int tex_scale = 2;

  // Init GUI
  GuiLoadStyleJungle();
  GuiLayoutJungleState settings_state = InitGuiLayoutJungle();

  unsigned int tile_w = 32;
  unsigned int tile_h = 32;
  float outline_thickness = 2.0f;

  Tilemap tilemap = {.num_tiles = 0};

  TilePicker tp = {0};
  tp.tile_w = tile_w;
  tp.tile_h = tile_h;
  Tile *selected_tile = NULL;

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
  freeSetTex(TEX_TILESET_CONCRETE, LoadTexture("assets/concrete-tiles.png"),
             asset_store);
  freeSetTex(TEX_GHOST1, LoadTexture("assets/slime-ghost1.png"), asset_store);
  freeSetTex(TEX_GHOST2, LoadTexture("assets/slime-ghost2.png"), asset_store);
  freeSetTex(TEX_GHOST3, LoadTexture("assets/slime-ghost3.png"), asset_store);
  freeSetTex(TEX_GHOST4, LoadTexture("assets/slime-ghost4.png"), asset_store);
  freeSetTex(__TEX_ERASER, LoadTexture("assets/tiny-trash-can32x32.png"),
             asset_store);

  Tileset ts_concrete = {
      .tex_choice = TEX_TILESET_CONCRETE,
      .tile_w = tile_w,
      .tile_h = tile_h,
      .num_x = 3,
      .num_y = 3,
  };

  // Fill TilePicker with tileset.
  Tile eraser = {
      .tex_choice = __TEX_ERASER,
      .tile_w = tile_w,
      .tile_h = tile_h,
      .offset_x = 0,
      .offset_y = 0,
  };
  addTileToTilePicker(eraser, &tp);

  for (int i = 0; i < ts_concrete.num_x; ++i) {
    for (int j = 0; j < ts_concrete.num_y; ++j) {
      float offset_x = ts_concrete.tile_w * i + ts_concrete.tile_w;
      float offset_y = ts_concrete.tile_h * j + ts_concrete.tile_h;
      Tile tile = {
          .tex_choice = ts_concrete.tex_choice,
          .offset_x = offset_x,
          .offset_y = offset_y,
          .tile_w = ts_concrete.tile_w,
          .tile_h = ts_concrete.tile_h,
      };
      addTileToTilePicker(tile, &tp);
    }
  }

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
  bodyDef.linearDamping = 1.0f;
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
    const InputsContext *inputs_ctx = ecs_singleton_get(world, InputsContext);

    const Position *p = ecs_get(world, ent, Position);
    hero_cam.offset =
        (Vector2){.x = (float)screen_width / 2, .y = (float)screen_height / 2};
    hero_cam.target =
        (Vector2){.x = p->x * 10 - 10, .y = (p->y * 10 - 10) * -1};
    hero_cam.zoom = settings_state.Slider001Value;

    Vector2 world_mouse_pos =
        GetScreenToWorld2D(inputs_ctx->mouse_pos, hero_cam);

    // handle tilepicker
    for (int i = 0; i < NUM_TILEPICKER_ROWS; ++i) {
      for (int j = 0; j < NUM_TILEPICKER_COLS; ++j) {
        if (tp.has_tile[i][j]) {
          Tile tile = tp.tiles[i][j];
          if (CheckCollisionPointRec(
                  inputs_ctx->mouse_pos,
                  (Rectangle){
                      .x = tile.pos_x * tex_scale,
                      .y = screen_height -
                           (tile.pos_y + tile.tile_h) * tex_scale,
                      .width = tile.tile_w * tex_scale,
                      .height = tile.tile_h * tex_scale,
                  }) &&
              inputs_ctx->kb_inputs.l_click) {
            selected_tile = &tp.tiles[i][j];
          }
        }
      }
    }

    // handle tile placement (tilemap)
    if ((selected_tile != NULL) && inputs_ctx->kb_inputs.l_click &&
        !CheckCollisionPointRec( // mouse not over tilepicker
            inputs_ctx->mouse_pos,
            (Rectangle){
                .x = 0,
                .y = screen_height - tile_h * tex_scale * NUM_TILEPICKER_ROWS,
                .width = tile_w * tex_scale * NUM_TILEPICKER_COLS,
                .height = tile_h * tex_scale * NUM_TILEPICKER_ROWS,
            })) {
      int dest_x =
          (int)world_mouse_pos.x / (tile_w * tex_scale) * (tile_w * tex_scale);
      int dest_y =
          (int)world_mouse_pos.y / (tile_h * tex_scale) * (tile_h * tex_scale);
      Tile tile = *selected_tile;
      if (tile.tex_choice == __TEX_ERASER) {
        for (int i = 0; i < tilemap.num_tiles; ++i) {
          if ((tilemap.tiles[i].pos_x == dest_x) &&
              (tilemap.tiles[i].pos_y == dest_y)) {
            --tilemap.num_tiles;
            tilemap.tiles[i] = tilemap.tiles[tilemap.num_tiles];
          }
        }
      } else {
        for (int i = 0; i < tilemap.num_tiles; ++i) {
          // prevent tile placement if there's a tile there already
          if ((tilemap.tiles[i].pos_x == dest_x) &&
              (tilemap.tiles[i].pos_y == dest_y)) {
            --tilemap.num_tiles;
            tilemap.tiles[i] = tilemap.tiles[tilemap.num_tiles];
            break;
          }
        }
        // only can add tile if tilemap not full
        if (tilemap.num_tiles < NUM_TILEMAP_TILES) {
          tile.pos_x = dest_x;
          tile.pos_y = dest_y;
          tilemap.tiles[tilemap.num_tiles] = tile;
          ++tilemap.num_tiles;
        }
      }
    }
    // ------
    // ------ Draw ------
    BeginDrawing();
    ClearBackground(settings_state.ColorPicker003Value);

    BeginMode2D(hero_cam);
    // Draw tiles here
    for (int i = 0; i < tilemap.num_tiles; ++i) {
      Tile tile = tilemap.tiles[i];
      DrawTexturePro(getTex(tile.tex_choice, asset_store),
                     (Rectangle){
                         .x = tile.offset_x,
                         .y = tile.offset_y,
                         .width = tile.tile_w,
                         .height = tile.tile_h,
                     },
                     (Rectangle){
                         .x = tile.pos_x,
                         .y = tile.pos_y,
                         .width = tile.tile_w * tex_scale,
                         .height = tile.tile_h * tex_scale,
                     },
                     (Vector2){
                         .x = 0.0f,
                         .y = 0.0f,
                     },
                     0.0f, WHITE);
    }

    if (selected_tile != NULL) {
      Tile tile = *selected_tile;
      int dest_x =
          (int)world_mouse_pos.x / (tile_w * tex_scale) * (tile_w * tex_scale);
      int dest_y =
          (int)world_mouse_pos.y / (tile_h * tex_scale) * (tile_h * tex_scale);
      DrawTexturePro(getTex(tile.tex_choice, asset_store),
                     (Rectangle){
                         .x = tile.offset_x,
                         .y = tile.offset_y,
                         .width = tile.tile_w,
                         .height = tile.tile_h,
                     },
                     (Rectangle){
                         .x = dest_x,
                         .y = dest_y,
                         .width = tile.tile_w * tex_scale,
                         .height = tile.tile_h * tex_scale,
                     },
                     (Vector2){
                         .x = 0.0f,
                         .y = 0.0f,
                     },
                     0.0f, WHITE);
    }

    DrawRectangle(-1500, 20, 3000, 200, GREEN);

    Texture ghost = getCurrFrameAnimation6(ghost_anim, asset_store);
    DrawTexturePro(ghost,
                   (Rectangle){
                       .x = 0,
                       .y = 0,
                       .width = ghost.width,
                       .height = ghost.height,
                   },
                   (Rectangle){
                       .x = (int)(p->x * 10 - 10),
                       .y = (int)(p->y * 10 - 10) * -1,
                       .width = ghost.width * tex_scale,
                       .height = ghost.height * tex_scale,
                   },
                   (Vector2){
                       .x = ghost.width * tex_scale / 2.0f,
                       .y = ghost.height * tex_scale / 2.0f,
                   },
                   0.0f, WHITE);

    for (int i = 0; i < sizeof(ent_arr) / sizeof(ecs_entity_t); ++i) {
      const Position *p2 = ecs_get(world, ent_arr[i], Position);
      const PhysicsBody *pb = ecs_get(world, ent_arr[i], PhysicsBody);
      DrawRectangle(p2->x * 10 - 10, (p2->y * 10 - 10) * -1, 20, 20,
                    b2Body_IsAwake(pb->body) ? BLUE : BLACK);
    }

    // Draw grid lines
    if (settings_state.CheckBoxEx006Checked) {
      rlPushMatrix();
      rlTranslatef(0, tile_w * tex_scale / 2.0f * 500, 0);
      rlRotatef(90, 1, 0, 0);
      DrawGrid(1000, tile_w * tex_scale);
      rlPopMatrix();
    }

    EndMode2D();

    // Draw tilepicker
    DrawRectangleRec(
        (Rectangle){
            .x = 0,
            .y = screen_height - tile_h * tex_scale * NUM_TILEPICKER_ROWS,
            .width = tile_w * tex_scale * NUM_TILEPICKER_COLS,
            .height = tile_h * tex_scale * NUM_TILEPICKER_ROWS,
        },
        LIGHTGRAY);

    for (int i = 0; i < NUM_TILEPICKER_COLS; ++i) {
      for (int j = 0; j < NUM_TILEPICKER_ROWS; ++j) {
        // fill with bg color
        DrawRectangleLinesEx(
            (Rectangle){
                .x = i * tile_w * tex_scale,
                .y = screen_height - ((j + 1) * tile_h) * tex_scale,
                .width =
                    tile_w * tex_scale +
                    (i == (NUM_TILEPICKER_ROWS - 1) ? 0 : outline_thickness),
                .height =
                    tile_h * tex_scale +
                    (i == (NUM_TILEPICKER_COLS - 1) ? 0 : outline_thickness),
            },
            outline_thickness, BLACK);
      }
    }

    for (int i = 0; i < NUM_TILEPICKER_ROWS; ++i) {
      for (int j = 0; j < NUM_TILEPICKER_COLS; ++j) {
        if (tp.has_tile[i][j]) {
          Tile tile = tp.tiles[i][j];
          DrawTexturePro(
              getTex(tile.tex_choice, asset_store),
              (Rectangle){
                  .x = tile.offset_x,
                  .y = tile.offset_y,
                  .width = tile.tile_w,
                  .height = tile.tile_h,
              },
              (Rectangle){
                  .x = tile.pos_x * tex_scale,
                  .y = screen_height - (tile.pos_y + tile.tile_h) * tex_scale,
                  .width = tile.tile_w * tex_scale,
                  .height = tile.tile_h * tex_scale,
              },
              (Vector2){
                  .x = 0,
                  .y = 0,
              },
              0.0f, WHITE);
        }
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
