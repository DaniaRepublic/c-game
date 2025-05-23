#pragma once

#include <stdio.h>

#include "box2d/box2d.h"
#include "flecs.h"
#include "raylib.h"

typedef Vector2 ScreenDims;

typedef Vector2 Gravity;

typedef Vector2 BoxDimensions;

typedef Vector2 Position;

typedef Vector2 Velocity;

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
  Vector2 mouse_pos;
} InputsContext;

typedef Camera2D PlayerCamera;

typedef enum {
  _TEX_ENTITIES_START = 0,
  // Game entities have a band of [1, 999)
  TEX_HERO = 1,
  TEX_GHOST1 = 2,
  TEX_GHOST2 = 3,
  TEX_GHOST3 = 4,
  TEX_GHOST4 = 5,
  _TEX_ENTITIES_END, // WARNING DON'T MOVE!!!

  _TEX_TILESETS_START = 1000,
  // Tilesets have a band of [1001, 1999)
  TEX_TILESET_CONCRETE = 1001,
  _TEX_TILESETS_END, // WARNING DON'T MOVE!!!

  // Special textures have a band of [10001, 10099)
  _TEX_SPECIAL_START = 10000,
  __TEX_ERASER = 10001, // An eraser for textures.
  _TEX_SPECIAL_END,     // WARNING DON'T MOVE!!!

  // ADD ADDITIONAL SECTIONS HERE

  _TEX_ENTITIES_LEN = _TEX_ENTITIES_END - _TEX_ENTITIES_START - 1,
  _TEX_TILESETS_LEN = _TEX_TILESETS_END - _TEX_TILESETS_START - 1,
  _TEX_SPECIAL_LEN = _TEX_SPECIAL_END - _TEX_SPECIAL_START - 1,

  _TEX_LEN = _TEX_ENTITIES_LEN + _TEX_TILESETS_LEN + _TEX_SPECIAL_LEN,
} TextureChoice;

typedef struct TextureConfig {
  int scale;
  int tile_w;
  int tile_h;
} TextureConfig;

typedef struct Drawable {
  TextureChoice tex_choice;
  Color tint;
} Drawable;

#define NUM_ANIMATION_FRAMES 15

typedef struct Animation {
  int frames[NUM_ANIMATION_FRAMES];
  int curr_frame_idx;
  int total_frames;
  // in sec
  float frame_duration;
  // in sec
  float since_last_frame;
} Animation;

typedef struct {
  Vector2 anchor01;

  bool WindowBox000Active;
  float Slider001Value;
  Color ColorPicker003Value;
  bool CheckBoxEx006Checked;
} GuiLayoutJungleState;
