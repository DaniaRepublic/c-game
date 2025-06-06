#include <_stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "box2d/box2d.h"
#include "box2d/id.h"
#include "box2d/math_functions.h"
#include "box2d/types.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "flecs/os_api.h"
#include "flecs/private/addons.h"
#include "lib/config.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "components/components.h"
#include "lib/utils.h"
#include "modules/module.h"

ECS_COMPONENT_DECLARE(Gravity);
ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(BoxDimensions);
ECS_COMPONENT_DECLARE(InputsContext);
ECS_COMPONENT_DECLARE(GuiLayoutJungleState);
ECS_COMPONENT_DECLARE(ScreenDims);
ECS_COMPONENT_DECLARE(AssetStore);
ECS_COMPONENT_DECLARE(PhysicsBody);
ECS_COMPONENT_DECLARE(PhysicsBodyShape);
ECS_COMPONENT_DECLARE(PhysicsBodyId);
ECS_COMPONENT_DECLARE(Animation);
ECS_COMPONENT_DECLARE(TextureConfig);
ECS_COMPONENT_DECLARE(PlayerCamera);

ECS_TAG_DECLARE(TagControllable);
ECS_TAG_DECLARE(TagStatic);
ECS_TAG_DECLARE(TagCube);

int main() {
// ------------ Setup ------------
#ifdef DEV_BUILD
  printf("Building in development mode.\n");
#elif defined(TARGET_BUILD)
  printf("Building for target.\n");
#else
#error "Build type not specified"
#endif

  ScreenDims init_screen_dims = {.x = 600, .y = 400};
  // Raylib
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
  InitWindow(init_screen_dims.x, init_screen_dims.y, "Da Game");

  Vector2 dpi_factor = GetWindowScaleDPI();

  int curr_mon = GetCurrentMonitor();
  init_screen_dims.x = GetMonitorWidth(curr_mon);
  init_screen_dims.y = GetMonitorHeight(curr_mon);

#ifdef __APPLE__
  init_screen_dims.y -= 65; // Reduce height because of the notch
#endif                      /* ifdef __APPLE__ */

  SetWindowSize(init_screen_dims.x, init_screen_dims.y);
  SetWindowPosition(0, 0);
  MaximizeWindow();
  SetExitKey(KEY_ESCAPE);

  // Flecs
  ecs_world_t *world = ecs_init();
  {
    ECS_IMPORT(world, MainModule);
  }

  ScreenDims *screen_dims = ecs_singleton_ensure(world, ScreenDims);
  screen_dims->x = init_screen_dims.x;
  screen_dims->y = init_screen_dims.y;
  AssetStore *asset_store = ecs_singleton_ensure(world, AssetStore);
  TextureConfig *tex_conf = ecs_singleton_ensure(world, TextureConfig);
  // Must be multiple of 1
  tex_conf->scale = 2;
  tex_conf->tile_w = 32;
  tex_conf->tile_h = 32;
  GuiLayoutJungleState *settings_state =
      ecs_singleton_ensure(world, GuiLayoutJungleState);

  // load ghost animation
  freeSetTex(TEX_TILESET_CONCRETE, LoadTexture("assets/concrete-tiles.png"),
             asset_store);
  freeSetTex(TEX_GHOST1, LoadTexture("assets/slime-ghost1.png"), asset_store);
  freeSetTex(TEX_GHOST2, LoadTexture("assets/slime-ghost2.png"), asset_store);
  freeSetTex(TEX_GHOST3, LoadTexture("assets/slime-ghost3.png"), asset_store);
  freeSetTex(TEX_GHOST4, LoadTexture("assets/slime-ghost4.png"), asset_store);
  freeSetTex(__TEX_ERASER, LoadTexture("assets/tiny-trash-can32x32.png"),
             asset_store);

  // Init GUI style
  GuiLoadStyleJungle();

  // Load settings config
  *settings_state = InitGuiLayoutJungle();
  if (readSettingsFromFile(SETTINGS_FILE, settings_state)) {
    printf("Read settings from %s successfully.\n", SETTINGS_FILE);
  } else {
    printf("Failed to read settings from %s.\n", SETTINGS_FILE);
  }

  float outline_thickness = 2.0f;

  Tilemap tilemap = {.num_tiles = 0};

  TilePicker tp = {0};
  tp.tile_w = tex_conf->tile_w;
  tp.tile_h = tex_conf->tile_h;
  Tile *selected_tile = NULL;

  Tileset ts_concrete = {
      .tex_choice = TEX_TILESET_CONCRETE,
      .tile_w = tex_conf->tile_w,
      .tile_h = tex_conf->tile_h,
      .num_x = 3,
      .num_y = 3,
  };

  // Fill TilePicker with tileset.
  Tile eraser = {
      .tex_choice = __TEX_ERASER,
      .tile_w = tex_conf->tile_w,
      .tile_h = tex_conf->tile_h,
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

  // Initialize Box2D
  ecs_entity_t world_ent = ecs_new(world);
  ecs_set(world, world_ent, Gravity, {.x = 0, .y = -10});
  const Gravity *gravity = ecs_get(world, world_ent, Gravity);

  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = (b2Vec2){.x = gravity->x, .y = gravity->y};
  b2WorldId worldId = b2CreateWorld(&worldDef);

  ecs_entity_t ground_ent;
  ecs_entity_t ent;
  int n_cubes = 10;
  int curr_cube = 0;
  ecs_entity_t ent_arr[n_cubes];

  // Load save if available
  if (ecs_world_from_json_file(world, "assets/save.json", NULL) != NULL) {
    printf("Loading save...\n");
    // Create box 2d
    ecs_query_t *q =
        ecs_query(world, {.terms = {{.id = ecs_id(Position)},
                                    {.id = ecs_id(BoxDimensions)},
                                    {.id = ecs_id(PhysicsBody)},
                                    {.id = ecs_id(PhysicsBodyShape)}},
                          .cache_kind = EcsQueryCacheNone});
    ecs_iter_t it = ecs_query_iter(world, q);
    while (ecs_query_next(&it)) {
      for (int i = 0; i < it.count; ++i) {
        ecs_entity_t q_ent = it.entities[i];
        createPhysicalBoxForEntity(world, q_ent, worldId);
        if (ecs_has(world, q_ent, TagControllable)) {
          ent = q_ent;
        }
        if (ecs_has(world, q_ent, TagStatic)) {
          ground_ent = q_ent;
        }
        if (ecs_has(world, q_ent, TagCube)) {
          ent_arr[curr_cube] = q_ent;
          ++curr_cube;
        }
      }
    }
  } else {
    printf("Initializing new game.\n");
    // define ground for ecs
    ground_ent = createEntityWithPhysicalBox(
        world, worldId, (Position){.x = 0, .y = -10},
        (Velocity){.x = 0, .y = 0}, (BoxDimensions){.x = 150.0f, .y = 10.0f},
        (PhysicsBody){.body_type = b2_staticBody},
        (PhysicsBodyShape){
            .density = 1, .mat_friction = 0.6f, .mat_restitution = 0.2f});
    ecs_add(world, ground_ent, TagStatic);

    // b2BodyDef bodyDef = b2DefaultBodyDef();
    // bodyDef.type = b2_dynamicBody;
    // bodyDef.position = (b2Vec2){0.0f, 10.0f};
    // bodyDef.linearDamping = 1.0f;
    // bodyDef.angularDamping = 0.4f;

    ent = createEntityWithPhysicalBox(
        world, worldId, (Position){.x = 0, .y = 10}, (Velocity){.x = 0, .y = 0},
        (BoxDimensions){.x = 1.0f, .y = 1.0f},
        (PhysicsBody){.body_type = b2_dynamicBody},
        (PhysicsBodyShape){
            .density = 1, .mat_friction = 0.6f, .mat_restitution = 0.9f});
    ecs_add(world, ent, TagControllable);
    ecs_set_id(world, ent, ecs_id(PlayerCamera), sizeof(PlayerCamera),
               &(PlayerCamera){.zoom = settings_state->Slider001Value,
                               .rotation = 0,
                               .target = Vector2Zero()});
    ecs_set_id(world, ent, ecs_id(Animation), sizeof(Animation),
               &(Animation){.frames =
                                {
                                    TEX_GHOST1,
                                    TEX_GHOST2,
                                    TEX_GHOST3,
                                    TEX_GHOST4,
                                },
                            .curr_frame_idx = 0,
                            .total_frames = 4,
                            .frame_duration = 0.2,
                            .since_last_frame = 0});

    for (int i = 0; i < 5; ++i) {
      for (int j = 0; j < 2; ++j) {
        ecs_entity_t cube_ent = createEntityWithPhysicalBox(
            world, worldId,
            (Position){.x = 5.0f * (j + 1), .y = 15.0f + 5.0f * i},
            (Velocity){.x = 0, .y = 0}, (BoxDimensions){.x = 1.0f, .y = 1.0f},
            (PhysicsBody){.body_type = b2_dynamicBody},
            (PhysicsBodyShape){
                .density = 1, .mat_friction = 0.6f, .mat_restitution = 0.9f});
        ecs_add(world, cube_ent, TagCube);
        ent_arr[j + i * 2] = cube_ent;
      }
    }
  }

  const Position *p = ecs_get(world, ent, Position);
  const PlayerCamera *hero_cam = ecs_get(world, ent, PlayerCamera);
  const InputsContext *inputs_ctx = ecs_singleton_get(world, InputsContext);

  float time_acc = 0.0f;
  // ------------------------
  while (!WindowShouldClose()) {
    BeginDrawing(); // NOTE: a temporary hack to allow drawing in ecs systems.

    // ------------ Update ------------
    float delta_t = GetFrameTime();
    time_acc += delta_t;

    // Step physics one fixed time step at a time for accumulated #steps
    while (time_acc >= PHYS_TIME_STEP) {
      b2World_Step(worldId, PHYS_TIME_STEP, 4);
      time_acc -= PHYS_TIME_STEP;
    }

    // Update ECS
    ecs_progress(world, delta_t);

    Vector2 world_mouse_pos = GetScreenToWorld2D(
        (Vector2){.x = inputs_ctx->mouse_pos.x, .y = inputs_ctx->mouse_pos.y},
        *hero_cam);

    // handle tilepicker
    for (int i = 0; i < NUM_TILEPICKER_ROWS; ++i) {
      for (int j = 0; j < NUM_TILEPICKER_COLS; ++j) {
        if (tp.has_tile[i][j]) {
          Tile tile = tp.tiles[i][j];
          if (CheckCollisionPointRec(
                  (Vector2){.x = inputs_ctx->mouse_pos.x,
                            .y = inputs_ctx->mouse_pos.y},
                  (Rectangle){
                      .x = tile.pos_x * tex_conf->scale,
                      .y = screen_dims->y -
                           (tile.pos_y + tile.tile_h) * tex_conf->scale,
                      .width = tile.tile_w * tex_conf->scale,
                      .height = tile.tile_h * tex_conf->scale,
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
            (Vector2){.x = inputs_ctx->mouse_pos.x,
                      .y = inputs_ctx->mouse_pos.y},
            (Rectangle){
                .x = 0,
                .y = screen_dims->y -
                     tex_conf->tile_h * tex_conf->scale * NUM_TILEPICKER_ROWS,
                .width =
                    tex_conf->tile_w * tex_conf->scale * NUM_TILEPICKER_COLS,
                .height =
                    tex_conf->tile_h * tex_conf->scale * NUM_TILEPICKER_ROWS,
            })) {
      int dest_x = (int)world_mouse_pos.x /
                   (tex_conf->tile_w * tex_conf->scale) *
                   (tex_conf->tile_w * tex_conf->scale);
      int dest_y = (int)world_mouse_pos.y /
                   (tex_conf->tile_h * tex_conf->scale) *
                   (tex_conf->tile_h * tex_conf->scale);
      Tile tile = *selected_tile;
      // hadle eraser tool first
      if (tile.tex_choice == __TEX_ERASER) {
        // if placing on another tile, remove it
        for (int i = 0; i < tilemap.num_tiles; ++i) {
          if ((tilemap.tiles[i].pos_x == dest_x) &&
              (tilemap.tiles[i].pos_y == dest_y)) {
            --tilemap.num_tiles;
            tilemap.tiles[i] = tilemap.tiles[tilemap.num_tiles];
          }
        }
      } else {
        // if placing on another tile, remove it
        for (int i = 0; i < tilemap.num_tiles; ++i) {
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
    // ------------------------
    // ------------ Draw ------------

    BeginMode2D(*hero_cam);
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
                         .width = tile.tile_w * tex_conf->scale,
                         .height = tile.tile_h * tex_conf->scale,
                     },
                     (Vector2){
                         .x = 0.0f,
                         .y = 0.0f,
                     },
                     0.0f, WHITE);
    }

    if (selected_tile != NULL) {
      Tile tile = *selected_tile;
      int dest_x = (int)world_mouse_pos.x /
                   (tex_conf->tile_w * tex_conf->scale) *
                   (tex_conf->tile_w * tex_conf->scale);
      int dest_y = (int)world_mouse_pos.y /
                   (tex_conf->tile_h * tex_conf->scale) *
                   (tex_conf->tile_h * tex_conf->scale);
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
                         .width = tile.tile_w * tex_conf->scale,
                         .height = tile.tile_h * tex_conf->scale,
                     },
                     (Vector2){
                         .x = 0.0f,
                         .y = 0.0f,
                     },
                     0.0f, WHITE);
    }

    // Draw grid lines
    if (settings_state->CheckBoxEx006Checked) {
      rlPushMatrix();
      rlTranslatef(0, tex_conf->tile_w * tex_conf->scale / 2.0f * 500, 0);
      rlRotatef(90, 1, 0, 0);
      DrawGrid(1000, tex_conf->tile_w * tex_conf->scale);
      rlPopMatrix();
    }

    EndMode2D();

    // Draw tilepicker
    DrawRectangleRec(
        (Rectangle){
            .x = 0,
            .y = screen_dims->y -
                 tex_conf->tile_h * tex_conf->scale * NUM_TILEPICKER_ROWS,
            .width = tex_conf->tile_w * tex_conf->scale * NUM_TILEPICKER_COLS,
            .height = tex_conf->tile_h * tex_conf->scale * NUM_TILEPICKER_ROWS,
        },
        YELLOW);

    for (int i = 0; i < NUM_TILEPICKER_COLS; ++i) {
      for (int j = 0; j < NUM_TILEPICKER_ROWS; ++j) {
        // fill with bg color
        DrawRectangleLinesEx(
            (Rectangle){
                .x = i * tex_conf->tile_w * tex_conf->scale,
                .y = screen_dims->y -
                     ((j + 1) * tex_conf->tile_h) * tex_conf->scale,
                .width = tex_conf->tile_w * tex_conf->scale + outline_thickness,
                .height =
                    tex_conf->tile_h * tex_conf->scale + outline_thickness,
            },
            outline_thickness, BLACK);
      }
    }

    for (int i = 0; i < NUM_TILEPICKER_ROWS; ++i) {
      for (int j = 0; j < NUM_TILEPICKER_COLS; ++j) {
        if (tp.has_tile[i][j]) {
          Tile tile = tp.tiles[i][j];
          DrawTexturePro(getTex(tile.tex_choice, asset_store),
                         (Rectangle){
                             .x = tile.offset_x,
                             .y = tile.offset_y,
                             .width = tile.tile_w,
                             .height = tile.tile_h,
                         },
                         (Rectangle){
                             .x = tile.pos_x * tex_conf->scale,
                             .y = screen_dims->y -
                                  (tile.pos_y + tile.tile_h) * tex_conf->scale,
                             .width = tile.tile_w * tex_conf->scale,
                             .height = tile.tile_h * tex_conf->scale,
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
    GuiLayoutJungle(settings_state);

    DrawFPS(20, 20);
    EndDrawing();
    // ------------------------
  }

  // Save game
  char *json = ecs_world_to_json(world, NULL);
  FILE *save_file_w = fopen("assets/save.json", "w");
  if (save_file_w == NULL) {
    printf("Couldn't open file for saving.\n");
  } else {
    fwrite(json, strlen(json), 1, save_file_w);
    printf("Saved successfully.\n");
  }
  fclose(save_file_w);
  ecs_os_free(json);

  // Save settings
  if (writeSettingsToFile(SETTINGS_FILE, settings_state)) {
    printf("Saved settings in %s successfully.\n", SETTINGS_FILE);
  } else {
    printf("Failed to save settings in %s.\n", SETTINGS_FILE);
  }

  // ------------ Cleanup ------------
  ecs_fini(world);
  b2DestroyWorld(worldId);
  freeAssetStore(asset_store);
  CloseWindow();
  return EXIT_SUCCESS;
  // ------------------------
}
