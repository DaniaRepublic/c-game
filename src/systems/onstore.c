#include "box2d/box2d.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "lib/utils.h"
#include "raylib.h"

#include "components/components.h"
#include "systems/onstore.h"

void DrawBackground(ecs_iter_t *it) {
  GuiLayoutJungleState *settings_state = ecs_field(it, GuiLayoutJungleState, 0);
  ClearBackground(settings_state->ColorPicker003Value);
}

void BeginCameraMode(ecs_iter_t *it) {
  const PlayerCamera *cam = ecs_field(it, PlayerCamera, 0);
  BeginMode2D(*cam);
}

void EndCameraMode(ecs_iter_t *it) { EndMode2D(); }

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
