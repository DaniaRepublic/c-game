#pragma once

#include <stdio.h>

#include "box2d/box2d.h"
#include "flecs.h"
#include "raylib.h"

#include "lib/utils.h"

typedef struct Vec2 {
  float x;
  float y;
} Vec2;

typedef Vec2 Gravity;

typedef Vec2 BoxDimensions;

typedef Vec2 Position;

typedef struct PhysicsBody {
  b2BodyType body_type;
} PhysicsBody;

typedef struct PhysicsBodyShape {
  float density;
  float mat_friction;
  float mat_restitution;
} PhysicsBodyShape;

typedef struct PhysicsBodyId {
  b2BodyId body_id;
} PhysicsBodyId;

typedef struct KeyboardInputs {
  // true if down
  bool w;
  // true if down
  bool a;
  // true if down
  bool s;
  // true if down
  bool d;
  // true if down
  bool l_shift;
  // true if pressed
  bool e;
  // true if pressed
  bool space;
  // true if pressed
  bool r_click;
  // true if pressed
  bool l_click;
} KeyboardInputs;

typedef struct InputsContext {
  float delta_t;
  KeyboardInputs kb_inputs;
  Vec2 mouse_pos;
} InputsContext;

typedef struct Drawable {
  TextureChoice tex_choice;
  Color tint;
} Drawable;
