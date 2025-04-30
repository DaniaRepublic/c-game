#include "box2d/box2d.h"
#include "components/components.h"
#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "raylib.h"

void DrawBackground(ecs_iter_t *it) {
  GuiLayoutJungleState *settings_state = ecs_field(it, GuiLayoutJungleState, 0);
  ClearBackground(settings_state->ColorPicker003Value);
}

void DrawEnemies(ecs_iter_t *it) {
  Position *p = ecs_field(it, Position, 0);
  PhysicsBodyId *pb = ecs_field(it, PhysicsBodyId, 1);

  for (int i = 0; i < it->count; ++i) {
    DrawRectangle(p[i].x * 10 - 10, (p[i].y * 10 - 10) * -1, 20, 20,
                  b2Body_IsAwake(pb[i].body_id) ? BLUE : BLACK);
  }
}
