#include <stdio.h>

#include "box2d/box2d.h"
#include "box2d/math_functions.h"
#include "flecs.h"
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

  // Define maximum speed
  float maxSpeed = 30.0f; // Adjust as needed

  for (int i = 0; i < it->count; i++) {
    b2BodyId body = pb[i].body;
    b2Vec2 linVel = b2Body_GetLinearVelocity(body);

    // Determine target velocity based on input
    float targetVelX = 0.0f;
    if (ctx->kb_inputs.d) {
      targetVelX = maxSpeed;
    }
    if (ctx->kb_inputs.a) {
      targetVelX = -maxSpeed;
    }

    // Calculate the impulse required to move from current velocity to target
    // velocity. impulse = mass * (targetVel - currentVel)
    float mass = b2Body_GetMass(body);
    float impulseX = mass * (targetVelX - linVel.x);

    // Apply a horizontal impulse. Vertical impulses (like jump) should be
    // handled separately.
    if (b2AbsFloat(impulseX) > 0.001)
      b2Body_ApplyLinearImpulseToCenter(body, (b2Vec2){.x = impulseX, .y = 0},
                                        true);

    // For jumping or other one-off vertical impulses:
    if (ctx->kb_inputs.space)
      b2Body_ApplyLinearImpulseToCenter(body, (b2Vec2){.x = 0, .y = 200.0},
                                        true);
  }
}
