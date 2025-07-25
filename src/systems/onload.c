#include <stdio.h>

#include "box2d/box2d.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "raylib.h"

#include "components/components.h"
#include "onload.h"

void SyncScreenDims(ecs_iter_t *it) {
  ScreenDims *sd = ecs_field(it, ScreenDims, 0);

  sd->x = GetRenderWidth();
  sd->y = GetRenderHeight();
}

void UpdateInputsContextSystem(ecs_iter_t *it) {
  InputsContext *ctx = ecs_field(it, InputsContext, 0);

  InputsContext new_ctx;
  new_ctx.delta_t = it->delta_time;
  new_ctx.mouse_pos = GetMousePosition();
  new_ctx.kb_inputs = (KeyboardInputs){
      .w = IsKeyDown(KEY_W),
      .a = IsKeyDown(KEY_A),
      .s = IsKeyDown(KEY_S),
      .d = IsKeyDown(KEY_D),
      .l_shift = IsKeyDown(KEY_LEFT_SHIFT),
      .e = IsKeyPressed(KEY_E),
      .space = IsKeyPressed(KEY_SPACE),
      .r_click = IsMouseButtonPressed(MOUSE_RIGHT_BUTTON),
      .l_click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON),
      .l_down = IsMouseButtonDown(MOUSE_LEFT_BUTTON),
  };

  *ctx = new_ctx;
}
