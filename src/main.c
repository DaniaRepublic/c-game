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
#include "flecs/private/api_defines.h"
#include "lib/config.h"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "components/components.h"
#include "lib/utils.h"
#include "modules/module.h"

int main() {

  // ------------ Setup ------------

#ifdef DEV_BUILD
  printf("Building in development mode.\n");
#elif defined(TARGET_BUILD)
  printf("Building for target.\n");
#else
#error "Build type not specified"
#endif

  DisplayData init_display_data = {0};
  init_display_data.screen_dims =
      (Vector2){.x = VIRTUAL_WIDTH, .y = VIRTUAL_HEIGHT};
  // Raylib
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(init_display_data.screen_dims.x, init_display_data.screen_dims.y,
             "Da Game");

  Vector2 dpi_factor = GetWindowScaleDPI();

  int curr_mon = GetCurrentMonitor();
  init_display_data.screen_dims.x = GetMonitorWidth(curr_mon);
  init_display_data.screen_dims.y = GetMonitorHeight(curr_mon);

#ifdef __APPLE__
  init_display_data.screen_dims.y -= 65; // Reduce height because of the notch
#endif                                   /* ifdef __APPLE__ */

  SetWindowSize(init_display_data.screen_dims.x,
                init_display_data.screen_dims.y);
  SetWindowPosition(0, 0);
  MaximizeWindow();
  SetExitKey(KEY_ESCAPE);

  // Create render target at virtual res
  RenderTexture2D render_target =
      LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

  // Set filter for crisp pixels (nearest-neighbor)
  SetTextureFilter(render_target.texture, TEXTURE_FILTER_POINT);

  // Flecs
  ecs_world_t *world = ecs_init();
  {
    ECS_IMPORT(world, MainModule);
  }

  PhysicsWorld *physics_world = ecs_singleton_ensure(world, PhysicsWorld);
  VirtualScreen *virtual_screen = ecs_singleton_ensure(world, VirtualScreen);
  *virtual_screen = render_target;
  DisplayData *display_data = ecs_singleton_ensure(world, DisplayData);
  display_data->screen_dims = init_display_data.screen_dims;
  AssetStore *asset_store = ecs_singleton_ensure(world, AssetStore);
  TextureConfig *tex_conf = ecs_singleton_ensure(world, TextureConfig);
  // Must be multiple of 1
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

  // Initialize Box2D
  ecs_entity_t world_ent = ecs_new(world);
  ecs_set(world, world_ent, Gravity, {.x = 0, .y = -10});
  const Gravity *gravity = ecs_get(world, world_ent, Gravity);

  b2WorldDef world_def = b2DefaultWorldDef();
  world_def.gravity = (b2Vec2){.x = gravity->x, .y = gravity->y};
  b2WorldId b2_world_id = b2CreateWorld(&world_def);
  physics_world->b2_world_id = b2_world_id;

  // Load save if available
  if (ecs_world_from_json_file(world, "assets/save.json", NULL) != NULL) {
    printf("Loading save...\n");

    // Create boxes for 2d
    ecs_query_t *q =
        ecs_query(world, {.terms = {{.id = ecs_id(Position)},
                                    {.id = ecs_id(Rotation)},
                                    {.id = ecs_id(Velocity)},
                                    {.id = ecs_id(BoxDimensions)},
                                    {.id = ecs_id(PhysicsBody)},
                                    {.id = ecs_id(PhysicsBodyShape)},
                                    {.id = ecs_id(PhysicsBodyId)}},
                          .cache_kind = EcsQueryCacheNone});
    ecs_iter_t it = ecs_query_iter(world, q);
    while (ecs_query_next(&it)) {
      for (int i = 0; i < it.count; ++i) {
        ecs_entity_t q_ent = it.entities[i];
        createPhysicalBoxForEntity(world, q_ent, b2_world_id);
      }
    }
  } else {
    printf("Initializing new game...\n");

    TilePicker tp = {0};
    tp.tile_w = tex_conf->tile_w;
    tp.tile_h = tex_conf->tile_h;
    tp.outline_thickness = 2.0f;

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

    ecs_entity_t tilepicker_ent = ecs_new(world);
    ecs_set_id(world, tilepicker_ent, ecs_id(TilePicker), sizeof(TilePicker),
               &tp);

    // Game objects
    ecs_entity_t ground_ent = createEntityWithPhysicalBox(
        world, b2_world_id, (Position){.x = 0, .y = 50}, (Rotation){.rads = 0},
        (Velocity){.x = 0, .y = 0}, (BoxDimensions){.x = 500, .y = 50},
        (PhysicsBody){.body_type = b2_staticBody},
        (PhysicsBodyShape){
            .density = 1, .mat_friction = 0.6f, .mat_restitution = 0.2f});
    ecs_add(world, ground_ent, TagStatic);

    ecs_entity_t hero = createEntityWithPhysicalBox(
        world, b2_world_id, (Position){.x = 0, .y = -100},
        (Rotation){.rads = 0}, (Velocity){.x = 0, .y = 0},
        (BoxDimensions){.x = 10, .y = 10},
        (PhysicsBody){.body_type = b2_dynamicBody},
        (PhysicsBodyShape){
            .density = 1, .mat_friction = 0.6f, .mat_restitution = 0.9f});
    ecs_add(world, hero, TagControllable);
    ecs_set_id(world, hero, ecs_id(Animation), sizeof(Animation),
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
            world, b2_world_id,
            (Position){.x = 50 * (j + 1), .y = -150 - 50 * i},
            (Rotation){.rads = 0}, (Velocity){.x = 0, .y = 0},
            (BoxDimensions){.x = 10, .y = 10},
            (PhysicsBody){.body_type = b2_dynamicBody},
            (PhysicsBodyShape){
                .density = 1, .mat_friction = 0.6f, .mat_restitution = 0.9f});
        ecs_add(world, cube_ent, TagCube);
      }
    }
  }

  float time_acc = 0.0f;

  // ------------------------

  while (!WindowShouldClose()) {

    // ------------ Main loop ------------

    // BUG: physics is not incorporated into ecs, and so as fps is much higher,
    // items are added to ecs and attempted to be deleted before they are added
    // to physics world.
    // TODO: incorporate physics into ecs.
    float delta_t = GetFrameTime();
    time_acc += delta_t;

    // Step physics one fixed time step at a time for accumulated #steps
    while (time_acc >= PHYS_TIME_STEP) {
      b2World_Step(b2_world_id, PHYS_TIME_STEP, 4);
      time_acc -= PHYS_TIME_STEP;
    }

    // Update ECS
    ecs_progress(world, delta_t);

    // ------------------------
  }

  // ------------ Cleanup ------------

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

  ecs_fini(world);
  b2DestroyWorld(b2_world_id);
  freeAssetStore(asset_store);
  UnloadRenderTexture(render_target);
  CloseWindow();

  return EXIT_SUCCESS;

  // ------------------------
}
