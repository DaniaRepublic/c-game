#include <stdio.h>

#include "box2d/box2d.h"
#include "box2d/math_functions.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "raylib.h"

#include "components/components.h"
#include "lib/config.h"
#include "lib/utils.h"
#include "modules/module.h"
#include "onupdate.h"

void SyncPhysicsSystem(ecs_iter_t *it) {
  Position *p = ecs_field(it, Position, 0);
  Rotation *r = ecs_field(it, Rotation, 1);
  Velocity *v = ecs_field(it, Velocity, 2);
  PhysicsBodyId *pb = ecs_field(it, PhysicsBodyId, 3);

  for (int i = 0; i < it->count; i++) {
    b2Transform transform = b2Body_GetTransform(pb[i].body_id);
    p[i] = box2dToRaylibVec(transform.p);
    r[i].rads = box2dToRaylibRot(transform.q);
    b2Vec2 velocity = b2Body_GetLinearVelocity(pb[i].body_id);
    v[i].x = velocity.x;
    v[i].y = velocity.y;
  }
}

void ApplyControlsSystem(ecs_iter_t *it) {
  PhysicsBodyId *pb = ecs_field(it, PhysicsBodyId, 0);
  const InputsContext *ctx = ecs_field(it, InputsContext, 1);

  const float moveSpeed = 20.0f;  // world units per second
  const float dashBonus = 300.0f; // extra speed when dashing

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
    }

    b2Body_SetLinearVelocity(body, b2Lerp(b2Body_GetLinearVelocity(body),
                                          targetVel, PHYS_TIME_STEP));

    if (ctx->kb_inputs.space) {
      b2Body_ApplyLinearImpulseToCenter(body, b2MulSV(dashBonus, dir), true);
    }
  }
}

void UpdateAnimationSystem(ecs_iter_t *it) {
  Animation *anim = ecs_field(it, Animation, 0);
  const InputsContext *ctx = ecs_field(it, InputsContext, 1);
  const AssetStore *asset_store = ecs_field(it, AssetStore, 2);

  // Progress animations
  for (int i = 0; i < it->count; i++) {
    deltaTStepAnimation(&anim[i], ctx->delta_t, asset_store);
  }
}

void UpdatePlayerCameraSystem(ecs_iter_t *it) {
  const Position *p = ecs_field(it, Position, 0);
  PlayerCamera *cam = ecs_field(it, PlayerCamera, 1);
  const GuiLayoutJungleState *settings_state =
      ecs_field(it, GuiLayoutJungleState, 2);

  PlayerCamera new_cam = {0};
  new_cam.offset =
      (Vector2){.x = (float)VIRTUAL_WIDTH / 2, .y = (float)VIRTUAL_HEIGHT / 2};
  new_cam.target = (Vector2){.x = p->x, .y = p->y};
  new_cam.zoom = settings_state->Slider001Value;

  *cam = new_cam;
}

void UpdateTilemapSystem(ecs_iter_t *it) {
  const Position *positions = ecs_field(it, Position, 0);
  const PhysicsBodyId *body_ids = ecs_field(it, PhysicsBodyId, 1);
  const Tile *tiles = ecs_field(it, Tile, 2);
  const InputsContext *inputs_ctx = ecs_field(it, InputsContext, 3);
  const TextureConfig *tex_conf = ecs_field(it, TextureConfig, 4);
  const PlayerCamera *hero_cam = ecs_field(it, PlayerCamera, 5);
  const Tile *selected_tile = ecs_field(it, Tile, 6);
  const PhysicsWorld *physics_world = ecs_field(it, PhysicsWorld, 7);

  Vector2 world_mouse =
      GetScreenToWorld2D(inputs_ctx->mouse_virtual, *hero_cam);

  bool tile_selected = memcmp(selected_tile, &(Tile){0}, sizeof(Tile)) != 0;

  bool over_picker = CheckCollisionPointRec(
      inputs_ctx->mouse_virtual,
      (Rectangle){.x = 0,
                  .y =
                      VIRTUAL_HEIGHT - (tex_conf->tile_h * NUM_TILEPICKER_ROWS),
                  .width = tex_conf->tile_w * NUM_TILEPICKER_COLS,
                  .height = tex_conf->tile_h * NUM_TILEPICKER_ROWS});

  if (tile_selected && !over_picker && inputs_ctx->kb_inputs.l_down) {
    Tile tile = *selected_tile;
    Position dest = {
        .x = ((int)world_mouse.x / tex_conf->tile_w) * tex_conf->tile_w -
             (world_mouse.x < 0 ? tex_conf->tile_w : 0),
        .y = ((int)world_mouse.y / tex_conf->tile_h) * tex_conf->tile_h -
             (world_mouse.y < 0 ? tex_conf->tile_h : 0)};

    // hadle eraser tool first
    if (tile.tex_choice == __TEX_ERASER) {
      ecs_defer_begin(it->world);
      for (int i = 0; i < it->count; ++i) {
        if (positions[i].x == dest.x && positions[i].y == dest.y) {
          if (b2Body_IsValid(body_ids[i].body_id))
            b2DestroyBody(body_ids[i].body_id);
          ecs_delete(it->world, it->entities[i]);
        }
      }
      ecs_defer_end(it->world);
    } else {
      // remove existing
      // BUG: the problem with deleting here is that physics is not incorporated
      // into ecs, and so as fps is much higher, items are added to ecs and
      // attempted to be deleted before they are added to physics world.
      ecs_defer_begin(it->world);
      for (int i = 0; i < it->count; ++i) {
        if (positions[i].x == dest.x && positions[i].y == dest.y) {
          if (b2Body_IsValid(body_ids[i].body_id)) {
            printf("body generation=%d index1=%d world0=%d validated\n",
                   body_ids[i].body_id.generation, body_ids[i].body_id.index1,
                   body_ids[i].body_id.world0);
            b2DestroyBody(body_ids[i].body_id);
            printf("body generation=%d index1=%d world0=%d deleted\n",
                   body_ids[i].body_id.generation, body_ids[i].body_id.index1,
                   body_ids[i].body_id.world0);
          }
          ecs_delete(it->world, it->entities[i]);
        }
      }
      ecs_defer_end(it->world);

      if (true) { // TODO: (  < MAX_TILES_FOR_MAP) {
        // add new
        ecs_entity_t tile_ent = createEntityWithPhysicalBox(
            it->world, physics_world->b2_world_id, dest, (Rotation){.rads = 0},
            (Velocity){.x = 0, .y = 0},
            (BoxDimensions){.x = tex_conf->tile_w, .y = tex_conf->tile_h},
            (PhysicsBody){.body_type = b2_staticBody},
            (PhysicsBodyShape){
                .density = 1, .mat_friction = 0.1f, .mat_restitution = 0.1f});
        ecs_add(it->world, tile_ent, TagStatic);
        ecs_add(it->world, tile_ent, TagTile);
        ecs_set_id(it->world, tile_ent, ecs_id(Tile), sizeof(Tile), &tile);
      }
    }
  }
}

void UpdateTilePickerSystem(ecs_iter_t *it) {
  const TilePicker *tp = ecs_field(it, TilePicker, 0);
  const InputsContext *inputs_ctx = ecs_field(it, InputsContext, 1);
  const TextureConfig *tex_conf = ecs_field(it, TextureConfig, 2);
  Tile *selected_tile = ecs_field(it, Tile, 3);

  for (int i = 0; i < NUM_TILEPICKER_ROWS; ++i) {
    for (int j = 0; j < NUM_TILEPICKER_COLS; ++j) {
      if (!tp->has_tile[i][j])
        continue;
      Tile tile = tp->tiles[i][j];

      Rectangle rect = {.x = tile.pos_x,
                        .y = VIRTUAL_HEIGHT - (tile.pos_y + tile.tile_h),
                        .width = tile.tile_w,
                        .height = tile.tile_h};

      if (CheckCollisionPointRec(inputs_ctx->mouse_virtual, rect) &&
          inputs_ctx->kb_inputs.l_click) {
        *selected_tile = tile;
      }
    }
  }
}

// TODO: implement this system
void SpawnCubesSystem(ecs_iter_t *it) {
  const InputsContext *inputs_ctx = ecs_field(it, InputsContext, 1);

  if (inputs_ctx->kb_inputs.l_click) {
  }
}
