#include <stdio.h>

#include "box2d/box2d.h"
#include "box2d/math_functions.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "raylib.h"

#include "components/components.h"
#include "lib/config.h"
#include "lib/utils.h"
#include "onupdate.h"

void SyncPhysicsSystem(ecs_iter_t *it) {
  Position *p = ecs_field(it, Position, 0);
  Velocity *v = ecs_field(it, Velocity, 1);
  PhysicsBodyId *pb = ecs_field(it, PhysicsBodyId, 2);

  for (int i = 0; i < it->count; i++) {
    b2Transform transform = b2Body_GetTransform(pb[i].body_id);
    p[i].x = transform.p.x;
    p[i].y = transform.p.y;
    b2Vec2 velocity = b2Body_GetLinearVelocity(pb[i].body_id);
    v[i].x = velocity.x;
    v[i].y = velocity.y;
  }
}

void ApplyControlsSystem(ecs_iter_t *it) {
  PhysicsBodyId *pb = ecs_field(it, PhysicsBodyId, 0);
  const InputsContext *ctx = ecs_field(it, InputsContext, 1);

  const float moveSpeed = 30.0f;  // world units per second
  const float dashBonus = 500.0f; // extra speed when dashing

  for (int i = 0; i < it->count; i++) {
    b2BodyId body = pb[i].body_id;

    // 1) Build raw input vector
    b2Vec2 dir = b2Vec2_zero;
    if (ctx->kb_inputs.d)
      dir.x += 1;
    if (ctx->kb_inputs.a)
      dir.x -= 1;
    if (ctx->kb_inputs.w)
      dir.y += 1;
    if (ctx->kb_inputs.s)
      dir.y -= 1;

    b2Vec2 targetVel = b2Vec2_zero;
    if (b2Length(dir) > 1e-3f) {
      dir = b2Normalize(dir);
      targetVel = b2MulSV(moveSpeed, dir);
    } else {
    }

    b2Body_SetLinearVelocity(body, b2Lerp(b2Body_GetLinearVelocity(body),
                                          targetVel, PHYS_TIME_STEP));

    if (ctx->kb_inputs.space) {
      b2Body_ApplyLinearImpulseToCenter(body, b2MulSV(dashBonus, dir), true);
    }
  }
}

void UpdateAnimation(ecs_iter_t *it) {
  Animation *anim = ecs_field(it, Animation, 0);
  // Singletons
  const InputsContext *ctx = ecs_field(it, InputsContext, 1);
  const AssetStore *asset_store = ecs_field(it, AssetStore, 2);

  // Progress animations
  for (int i = 0; i < it->count; i++) {
    deltaTStepAnimation(&anim[i], ctx->delta_t, asset_store);
  }
}

void UpdatePlayerCamera(ecs_iter_t *it) {
  const Position *p = ecs_field(it, Position, 0);
  PlayerCamera *cam = ecs_field(it, PlayerCamera, 1);
  // Singletons
  const ScreenDims *screen_dims = ecs_field(it, ScreenDims, 2);
  const GuiLayoutJungleState *settings_state =
      ecs_field(it, GuiLayoutJungleState, 3);

  PlayerCamera new_cam = {0};
  new_cam.offset =
      (Vector2){.x = (float)screen_dims->x / 2, .y = (float)screen_dims->y / 2};
  new_cam.target = (Vector2){.x = p->x * 10 - 10, .y = (p->y * 10 - 10) * -1};
  new_cam.zoom = settings_state->Slider001Value;

  *cam = new_cam;
}

void UpdateTilemap(ecs_iter_t *it) {
  Tilemap *tilemap = ecs_field(it, Tilemap, 0);
  const InputsContext *inputs_ctx = ecs_field(it, InputsContext, 1);
  const ScreenDims *screen_dims = ecs_field(it, ScreenDims, 2);
  const TextureConfig *tex_conf = ecs_field(it, TextureConfig, 3);
  const PlayerCamera *hero_cam = ecs_field(it, PlayerCamera, 4);
  const Tile *selected_tile = ecs_field(it, Tile, 5);

  Vector2 world_mouse_pos = GetScreenToWorld2D(
      (Vector2){.x = inputs_ctx->mouse_pos.x, .y = inputs_ctx->mouse_pos.y},
      *hero_cam);

  // handle tile placement (tilemap)
  if ((memcmp(selected_tile, &(Tile){0}, sizeof(Tile)) != 0) &&
      inputs_ctx->kb_inputs.l_down &&
      !CheckCollisionPointRec( // mouse not over tilepicker
          (Vector2){.x = inputs_ctx->mouse_pos.x, .y = inputs_ctx->mouse_pos.y},
          (Rectangle){
              .x = 0,
              .y = screen_dims->y -
                   tex_conf->tile_h * tex_conf->scale * NUM_TILEPICKER_ROWS,
              .width = tex_conf->tile_w * tex_conf->scale * NUM_TILEPICKER_COLS,
              .height =
                  tex_conf->tile_h * tex_conf->scale * NUM_TILEPICKER_ROWS,
          })) {
    int dest_x =
        (int)world_mouse_pos.x / (tex_conf->tile_w * tex_conf->scale) *
            (tex_conf->tile_w * tex_conf->scale) -
        (world_mouse_pos.x < 0 ? (tex_conf->tile_w * tex_conf->scale) : 0);
    int dest_y =
        (int)world_mouse_pos.y / (tex_conf->tile_h * tex_conf->scale) *
            (tex_conf->tile_h * tex_conf->scale) -
        (world_mouse_pos.y < 0 ? (tex_conf->tile_h * tex_conf->scale) : 0);
    Tile tile = *selected_tile;
    // hadle eraser tool first
    if (tile.tex_choice == __TEX_ERASER) {
      // if placing on another tile, remove it
      for (int i = 0; i < tilemap->num_tiles; ++i) {
        if ((tilemap->tiles[i].pos_x == dest_x) &&
            (tilemap->tiles[i].pos_y == dest_y)) {
          --tilemap->num_tiles;
          tilemap->tiles[i] = tilemap->tiles[tilemap->num_tiles];
        }
      }
    } else {
      // if placing on another tile, remove it
      for (int i = 0; i < tilemap->num_tiles; ++i) {
        if ((tilemap->tiles[i].pos_x == dest_x) &&
            (tilemap->tiles[i].pos_y == dest_y)) {
          --tilemap->num_tiles;
          tilemap->tiles[i] = tilemap->tiles[tilemap->num_tiles];
          break;
        }
      }
      // only can add tile if tilemap not full
      if (tilemap->num_tiles < NUM_TILEMAP_TILES) {
        tile.pos_x = dest_x;
        tile.pos_y = dest_y;
        tilemap->tiles[tilemap->num_tiles] = tile;
        ++tilemap->num_tiles;
      }
    }
  }
}

void UpdateTilepicker(ecs_iter_t *it) {
  const TilePicker *tp = ecs_field(it, TilePicker, 0);
  const InputsContext *inputs_ctx = ecs_field(it, InputsContext, 1);
  const ScreenDims *screen_dims = ecs_field(it, ScreenDims, 2);
  const TextureConfig *tex_conf = ecs_field(it, TextureConfig, 3);
  Tile *selected_tile = ecs_field(it, Tile, 4);

  // handle tilepicker
  for (int i = 0; i < NUM_TILEPICKER_ROWS; ++i) {
    for (int j = 0; j < NUM_TILEPICKER_COLS; ++j) {
      if (tp->has_tile[i][j]) {
        Tile tile = tp->tiles[i][j];
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
          *selected_tile = tp->tiles[i][j];
        }
      }
    }
  }
}
