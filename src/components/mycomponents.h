#pragma once

#include "box2d/box2d.h"
#include "flecs.h"
#include "raylib.h"

#include "lib/config.h"
#include "lib/utils.h"

// Components
typedef struct {
  float delta_t;
  KeyboardInputs kb_inputs;
  Vector2 mouse_pos;
} InputsContext;

typedef struct {
  float x, y;
} Position;

typedef struct {
  b2BodyId body;
} PhysicsBody;

typedef struct {
  TextureChoice tex_choice;
  Color tint;
} Drawable;
