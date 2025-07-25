#include "box2d/box2d.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "lib/utils.h"
#include "raylib.h"
#include "rlgl.h"

#include "components/components.h"
#include "systems/onstore.h"

void BeginDrawingSystem(ecs_iter_t *it) { BeginDrawing(); }

void EndDrawingSystem(ecs_iter_t *it) { EndDrawing(); }

void BeginCameraMode(ecs_iter_t *it) {
  const PlayerCamera *cam = ecs_field(it, PlayerCamera, 0);
  BeginMode2D(*cam);
}

void EndCameraMode(ecs_iter_t *it) { EndMode2D(); }

void DrawBackground(ecs_iter_t *it) {
  // singletons
  const GuiLayoutJungleState *settings_state =
      ecs_field(it, GuiLayoutJungleState, 0);
  ClearBackground(settings_state->ColorPicker003Value);
}

void DrawColliders(ecs_iter_t *it) {
  const Position *p = ecs_field(it, Position, 0);
  const PhysicsBodyId *pb = ecs_field(it, PhysicsBodyId, 1);
  const BoxDimensions *bd = ecs_field(it, BoxDimensions, 2);

  for (int i = 0; i < it->count; ++i) {
    DrawRectangle(p[i].x * 10 - bd->x * 10, (p[i].y * 10 * -1 - bd->y * 10),
                  bd->x * 20, bd->y * 20,
                  b2Body_IsAwake(pb[i].body_id) ? BLUE : BLACK);
  }
}

void DrawAnimations(ecs_iter_t *it) {
  const Position *p = ecs_field(it, Position, 0);
  const Animation *ghost_anim = ecs_field(it, Animation, 1);
  // singletons
  const AssetStore *asset_store = ecs_field(it, AssetStore, 2);
  const TextureConfig *tex_conf = ecs_field(it, TextureConfig, 3);

  Texture ghost = getCurrFrameAnimation(ghost_anim, asset_store);
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
                     .width = ghost.width * tex_conf->scale,
                     .height = ghost.height * tex_conf->scale,
                 },
                 (Vector2){
                     .x = ghost.width * tex_conf->scale / 2.0f,
                     .y = ghost.height * tex_conf->scale / 2.0f,
                 },
                 0.0f, WHITE);
}

void DrawGridSystem(ecs_iter_t *it) {
  // singletons
  const GuiLayoutJungleState *settings_state =
      ecs_field(it, GuiLayoutJungleState, 0);
  const TextureConfig *tex_conf = ecs_field(it, TextureConfig, 1);

  // Draw grid lines
  if (settings_state->CheckBoxEx006Checked) {
    rlPushMatrix();
    rlTranslatef(0, tex_conf->tile_w * tex_conf->scale / 2.0f * 500, 0);
    rlRotatef(90, 1, 0, 0);
    DrawGrid(1000, tex_conf->tile_w * tex_conf->scale);
    rlPopMatrix();
  }
}

void DrawTilemap(ecs_iter_t *it) {
  Tilemap *tilemap = ecs_field(it, Tilemap, 0);
  const TextureConfig *tex_conf = ecs_field(it, TextureConfig, 1);
  const AssetStore *asset_store = ecs_field(it, AssetStore, 2);

  // Draw tiles here
  for (int i = 0; i < tilemap->num_tiles; ++i) {
    Tile tile = tilemap->tiles[i];
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
}

void DrawHoldingTile(ecs_iter_t *it) {
  const AssetStore *asset_store = ecs_field(it, AssetStore, 0);
  const InputsContext *inputs_ctx = ecs_field(it, InputsContext, 1);
  const TextureConfig *tex_conf = ecs_field(it, TextureConfig, 2);
  const PlayerCamera *hero_cam = ecs_field(it, PlayerCamera, 3);
  const Tile *selected_tile = ecs_field(it, Tile, 4);

  Vector2 world_mouse_pos = GetScreenToWorld2D(
      (Vector2){.x = inputs_ctx->mouse_pos.x, .y = inputs_ctx->mouse_pos.y},
      *hero_cam);

  if (memcmp(selected_tile, &(Tile){0}, sizeof(Tile)) != 0) {
    Tile tile = *selected_tile;
    int dest_x =
        (int)world_mouse_pos.x / (tex_conf->tile_w * tex_conf->scale) *
            (tex_conf->tile_w * tex_conf->scale) -
        (world_mouse_pos.x < 0 ? (tex_conf->tile_w * tex_conf->scale) : 0);
    int dest_y =
        (int)world_mouse_pos.y / (tex_conf->tile_h * tex_conf->scale) *
            (tex_conf->tile_h * tex_conf->scale) -
        (world_mouse_pos.y < 0 ? (tex_conf->tile_h * tex_conf->scale) : 0);
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
}

void DrawTilePicker(ecs_iter_t *it) {
  const TilePicker *tp = ecs_field(it, TilePicker, 0);
  const InputsContext *inputs_ctx = ecs_field(it, InputsContext, 1);
  const TextureConfig *tex_conf = ecs_field(it, TextureConfig, 2);
  const PlayerCamera *hero_cam = ecs_field(it, PlayerCamera, 3);
  const Tile *selected_tile = ecs_field(it, Tile, 4);
  const ScreenDims *screen_dims = ecs_field(it, ScreenDims, 5);
  const AssetStore *asset_store = ecs_field(it, AssetStore, 6);

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
              .width =
                  tex_conf->tile_w * tex_conf->scale + tp->outline_thickness,
              .height =
                  tex_conf->tile_h * tex_conf->scale + tp->outline_thickness,
          },
          tp->outline_thickness, BLACK);
    }
  }

  for (int i = 0; i < NUM_TILEPICKER_ROWS; ++i) {
    for (int j = 0; j < NUM_TILEPICKER_COLS; ++j) {
      if (tp->has_tile[i][j]) {
        Tile tile = tp->tiles[i][j];
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
}

void DrawGui(ecs_iter_t *it) {
  GuiLayoutJungleState *settings_state = ecs_field(it, GuiLayoutJungleState, 0);

  GuiLayoutJungle(settings_state);

  DrawFPS(20, 20);
}
