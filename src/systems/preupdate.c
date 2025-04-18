#include <stdio.h>

#include "box2d/box2d.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "raylib.h"

#include "components/mycomponents.h"
#include "preupdate.h"

void UpdateInputsContextSystem(ecs_iter_t *it) {
  InputsContext new_ctx;
  new_ctx.delta_t = GetFrameTime();
  new_ctx.mouse_pos = GetMousePosition();
  new_ctx.kb_inputs = (KeyboardInputs){.w = IsKeyDown(KEY_W),
                                       .a = IsKeyDown(KEY_A),
                                       .s = IsKeyDown(KEY_S),
                                       .d = IsKeyDown(KEY_D),
                                       .l_shift = IsKeyDown(KEY_LEFT_SHIFT),
                                       .e = IsKeyPressed(KEY_E),
                                       .space = IsKeyPressed(KEY_SPACE)};

  InputsContext *ctx = ecs_field(it, InputsContext, 0);
  *ctx = new_ctx;
}
