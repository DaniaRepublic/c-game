#include <stdio.h>

#include "box2d/box2d.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "raylib.h"

#include "components/components.h"
#include "lib/config.h"
#include "onload.h"

void SyncDisplayDataSystem(ecs_iter_t *it) {
  DisplayData *display_data = ecs_field(it, DisplayData, 0);

  display_data->render_dims.x = GetRenderWidth();
  display_data->render_dims.y = GetRenderHeight();

  int screen_w = GetScreenWidth();
  int screen_h = GetScreenHeight();

  float scale = (float)fmin(screen_w / (float)VIRTUAL_WIDTH,
                            screen_h / (float)VIRTUAL_HEIGHT);
  float offset_x = (screen_w - (VIRTUAL_WIDTH * scale)) / 2.0f;
  float offset_y = (screen_h - (VIRTUAL_HEIGHT * scale)) / 2.0f;

  display_data->screen_dims.x = screen_w;
  display_data->screen_dims.y = screen_h;
  display_data->virtual_scalar = scale;
  display_data->virtual_offset_x = offset_x;
  display_data->virtual_offset_y = offset_y;
}

void UpdateInputsContextSystem(ecs_iter_t *it) {
  InputsContext *ctx = ecs_field(it, InputsContext, 0);
  DisplayData *display_data = ecs_field(it, DisplayData, 1);

  InputsContext new_ctx;
  new_ctx.delta_t = it->delta_time;
  new_ctx.mouse_pos = GetMousePosition();
  new_ctx.mouse_virtual =
      (Vector2){(new_ctx.mouse_pos.x - display_data->virtual_offset_x) /
                    display_data->virtual_scalar,
                (new_ctx.mouse_pos.y - display_data->virtual_offset_y) /
                    display_data->virtual_scalar};
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
