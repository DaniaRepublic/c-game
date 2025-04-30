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

  cam->offset =
      (Vector2){.x = (float)screen_dims->x / 2, .y = (float)screen_dims->y / 2};
  cam->target = (Vector2){.x = p->x * 10 - 10, .y = (p->y * 10 - 10) * -1};
  cam->zoom = settings_state->Slider001Value;
}
