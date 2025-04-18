#include "box2d/box2d.h"
#include "flecs.h"
#include "raylib.h"

#include "components/mycomponents.h"
#include "hero.h"
#include "lib/utils.h"

void createHero(TextureChoice tex_choice) {
  Texture2D slime_tex = LoadTexture("./assets/slime.png");
  // freeSetTex(TEX_HERO, slime_tex, &asset_store);
}
