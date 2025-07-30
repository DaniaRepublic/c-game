#include "box2d/box2d.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "lib/config.h"
#include "lib/utils.h"
#include "raylib.h"
#include "rlgl.h"

#include "components/components.h"
#include "systems/onstore.h"

void BeginDrawingSystem(ecs_iter_t *it) { BeginDrawing(); }

void EndDrawingSystem(ecs_iter_t *it) { EndDrawing(); }

void BeginCameraModeSystem(ecs_iter_t *it) {
  const PlayerCamera *cam = ecs_field(it, PlayerCamera, 0);
  BeginMode2D(*cam);
}

void EndCameraModeSystem(ecs_iter_t *it) { EndMode2D(); }

void BeginTextureModeSystem(ecs_iter_t *it) {
  VirtualScreen *virtual_screen = ecs_field(it, VirtualScreen, 0);
  BeginTextureMode(*virtual_screen);
}

void EndTextureModeSystem(ecs_iter_t *it) { EndTextureMode(); }

void DrawVirtualScreenSystem(ecs_iter_t *it) {
  const VirtualScreen *virtual_screen = ecs_field(it, VirtualScreen, 0);
  DisplayData *display_data = ecs_field(it, DisplayData, 1);

  ClearBackground(BLACK);

  Rectangle source = {0.0f, 0.0f, (float)VIRTUAL_WIDTH, -(float)VIRTUAL_HEIGHT};
  Rectangle dest = {display_data->virtual_offset_x,
                    display_data->virtual_offset_y,
                    (float)VIRTUAL_WIDTH * display_data->virtual_scalar,
                    (float)VIRTUAL_HEIGHT * display_data->virtual_scalar};
  DrawTexturePro(virtual_screen->texture, source, dest, (Vector2){0.0f, 0.0f},
                 0.0f, WHITE);
}

void DrawBackgroundSystem(ecs_iter_t *it) {
  // singletons
  const GuiLayoutJungleState *settings_state =
      ecs_field(it, GuiLayoutJungleState, 0);
  ClearBackground(settings_state->ColorPicker003Value);
}

void DrawCollidersSystem(ecs_iter_t *it) {
  const Position *p = ecs_field(it, Position, 0);
  const Rotation *r = ecs_field(it, Rotation, 1);
  const PhysicsBodyId *pb = ecs_field(it, PhysicsBodyId, 2);
  const BoxDimensions *bd = ecs_field(it, BoxDimensions, 3);

  for (int i = 0; i < it->count; ++i) {
    Rectangle rect = {
        .x = p[i].x,          // Center x position
        .y = p[i].y,          // Center y position
        .width = bd[i].x * 2, // Width scaled
        .height = bd[i].y * 2 // Height scaled
    };
    Vector2 origin = {bd[i].x, bd[i].y};
    float rotation = r[i].rads / PI * 180.0f;

    DrawRectanglePro(rect, origin, rotation,
                     b2Body_IsAwake(pb[i].body_id) ? BLUE : BLACK);
  }
}

void DrawAnimationsSystem(ecs_iter_t *it) {
  const Position *p = ecs_field(it, Position, 0);
  const Animation *anim = ecs_field(it, Animation, 1);
  // singletons
  const AssetStore *asset_store = ecs_field(it, AssetStore, 2);
  const TextureConfig *tex_conf = ecs_field(it, TextureConfig, 3);

  Texture tex = getCurrFrameAnimation(anim, asset_store);
  DrawTexturePro(tex,
                 (Rectangle){
                     .x = 0,
                     .y = 0,
                     .width = tex.width,
                     .height = tex.height,
                 },
                 (Rectangle){
                     .x = (int)p->x,
                     .y = (int)p->y,
                     .width = tex.width,
                     .height = tex.height,
                 },
                 (Vector2){
                     .x = tex.width / 2.0f,
                     .y = tex.height / 2.0f,
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
    rlTranslatef(0, tex_conf->tile_w / 2.0f * 500, 0);
    rlRotatef(90, 1, 0, 0);
    DrawGrid(1000, tex_conf->tile_w);
    rlPopMatrix();
  }
}

void DrawTilemapSystem(ecs_iter_t *it) {
  Position *positions = ecs_field(it, Position, 0);
  Tile *tiles = ecs_field(it, Tile, 1);
  const TextureConfig *tex_conf = ecs_field(it, TextureConfig, 2);
  const AssetStore *asset_store = ecs_field(it, AssetStore, 3);

  // Draw tiles here
  for (int i = 0; i < it->count; ++i) {
    Tile tile = tiles[i];
    DrawTexturePro(getTex(tile.tex_choice, asset_store),
                   (Rectangle){
                       .x = tile.offset_x,
                       .y = tile.offset_y,
                       .width = tile.tile_w,
                       .height = tile.tile_h,
                   },
                   (Rectangle){
                       .x = positions[i].x,
                       .y = positions[i].y,
                       .width = tile.tile_w,
                       .height = tile.tile_h,
                   },
                   (Vector2){
                       .x = 0.0f,
                       .y = 0.0f,
                   },
                   0.0f, WHITE);
  }
}

void DrawHoldingTileSystem(ecs_iter_t *it) {
  const AssetStore *asset_store = ecs_field(it, AssetStore, 0);
  const InputsContext *inputs_ctx = ecs_field(it, InputsContext, 1);
  const TextureConfig *tex_conf = ecs_field(it, TextureConfig, 2);
  const PlayerCamera *hero_cam = ecs_field(it, PlayerCamera, 3);
  const Tile *selected_tile = ecs_field(it, Tile, 4);

  Vector2 world_mouse =
      GetScreenToWorld2D(inputs_ctx->mouse_virtual, *hero_cam);

  if (memcmp(selected_tile, &(Tile){0}, sizeof(Tile)) != 0) {
    Tile tile = *selected_tile;
    int dest_x = (int)world_mouse.x / (tex_conf->tile_w) * (tex_conf->tile_w) -
                 (world_mouse.x < 0 ? (tex_conf->tile_w) : 0);
    int dest_y = (int)world_mouse.y / (tex_conf->tile_h) * (tex_conf->tile_h) -
                 (world_mouse.y < 0 ? (tex_conf->tile_h) : 0);
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
                       .width = tile.tile_w,
                       .height = tile.tile_h,
                   },
                   (Vector2){
                       .x = 0.0f,
                       .y = 0.0f,
                   },
                   0.0f, WHITE);
  }
}

void DrawTilePickerSystem(ecs_iter_t *it) {
  const TilePicker *tp = ecs_field(it, TilePicker, 0);
  const InputsContext *inputs_ctx = ecs_field(it, InputsContext, 1);
  const TextureConfig *tex_conf = ecs_field(it, TextureConfig, 2);
  const PlayerCamera *hero_cam = ecs_field(it, PlayerCamera, 3);
  const Tile *selected_tile = ecs_field(it, Tile, 4);
  const AssetStore *asset_store = ecs_field(it, AssetStore, 5);

  DrawRectangleRec(
      (Rectangle){
          .x = 0,
          .y = VIRTUAL_HEIGHT - tex_conf->tile_h * NUM_TILEPICKER_ROWS,
          .width = tex_conf->tile_w * NUM_TILEPICKER_COLS,
          .height = tex_conf->tile_h * NUM_TILEPICKER_ROWS,
      },
      WHITE);

  for (int i = 0; i < NUM_TILEPICKER_COLS; ++i) {
    for (int j = 0; j < NUM_TILEPICKER_ROWS; ++j) {
      DrawRectangleLinesEx(
          (Rectangle){
              .x = i * tex_conf->tile_w,
              .y = VIRTUAL_HEIGHT - ((j + 1) * tex_conf->tile_h),
              .width = tex_conf->tile_w + tp->outline_thickness,
              .height = tex_conf->tile_h + tp->outline_thickness,
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
                           .x = tile.pos_x,
                           .y = VIRTUAL_HEIGHT - (tile.pos_y + tile.tile_h),
                           .width = tile.tile_w,
                           .height = tile.tile_h,
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

void DrawGuiSystem(ecs_iter_t *it) {
  GuiLayoutJungleState *settings_state = ecs_field(it, GuiLayoutJungleState, 0);

  GuiLayoutJungle(settings_state);

  DrawFPS(20, 20);
}
