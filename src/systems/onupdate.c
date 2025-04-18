#include <stdio.h>

#include "box2d/box2d.h"
#include "box2d/math_functions.h"
#include "flecs.h"
#include "lib/config.h"
#include "raylib.h"

#include "components/mycomponents.h"
#include "onupdate.h"

void SyncPhysicsSystem(ecs_iter_t *it) {
  Position *p = ecs_field(it, Position, 0);
  PhysicsBody *pb = ecs_field(it, PhysicsBody, 1);

  for (int i = 0; i < it->count; i++) {
    b2Transform transform = b2Body_GetTransform(pb[i].body);
    p[i].x = transform.p.x;
    p[i].y = transform.p.y;
  }
}

void ApplyControlsSystem(ecs_iter_t *it) {
  PhysicsBody *pb = ecs_field(it, PhysicsBody, 1);
  const InputsContext *ctx = ecs_field(it, InputsContext, 2);

  float maxSpeed = 30.0f;
  float dashExtra = 50.0f;

  for (int i = 0; i < it->count; i++) {
    b2BodyId body = pb[i].body;
    b2Vec2 linVel = b2Body_GetLinearVelocity(body);

    b2Vec2 inputDir = b2Vec2_zero;
    if (ctx->kb_inputs.d) {
      inputDir.x += 1;
    }
    if (ctx->kb_inputs.a) {
      inputDir.x -= 1;
    }
    if (ctx->kb_inputs.w) {
      inputDir.y += 1;
    }
    if (ctx->kb_inputs.s) {
      inputDir.y -= 1;
    }

    if (b2Length(inputDir) > 1e-6) {
      b2Vec2 normDir = b2Normalize(inputDir);

      float mass = b2Body_GetMass(body);
      // Impulse = mass * acceleration * dt.
      b2Vec2 accelImpulse = b2MulSV(mass * maxSpeed * PHYS_TIME_STEP, normDir);

      float currentSpeed = b2Length(linVel);
      if (currentSpeed < maxSpeed) {
        b2Body_ApplyLinearImpulseToCenter(body, accelImpulse, true);
      } else {
        if (b2Dot(linVel, normDir) < 0)
          b2Body_ApplyLinearImpulseToCenter(body, accelImpulse, true);
      }

      if (ctx->kb_inputs.space) {
        b2Vec2 dashImpulse = b2MulSV(mass * dashExtra, normDir);
        b2Body_ApplyLinearImpulseToCenter(body, dashImpulse, true);
      }
    }
  }
}
