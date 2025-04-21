#include <stdbool.h>
#include <stdio.h>

#include "raylib.h"

#include "utils.h"

// Window state management.
void setScreenDims(int *w, int *h) {
  *w = GetRenderWidth();
  *h = GetRenderHeight();
}

// Graphics

int getTextureChoiceSeq(TextureChoice choice) {
  if ((choice > _TEX_ENTITIES_START) && (choice < _TEX_ENTITIES_END)) {
    return choice - (_TEX_ENTITIES_START + 1);
  } else if ((choice > _TEX_TILESETS_START) && (choice < _TEX_TILESETS_END)) {
    return (choice - (_TEX_TILESETS_START + 1)) + _TEX_ENTITIES_LEN;
  } else if ((choice > _TEX_SPECIAL_START) && (choice < _TEX_SPECIAL_END)) {
    return (choice - (_TEX_SPECIAL_START + 1)) + _TEX_ENTITIES_LEN +
           _TEX_TILESETS_LEN;
  }
  return -1;
}

// Returns texture at curr_frame_idx.
Texture getCurrFrameAnimation6(Animation6 anim, AssetStore *store) {
  return getTex(anim.frames[anim.curr_frame_idx], store);
}

// Simply updates curr_frame_idx, wraping if nessesary.
void stepAnimation6(Animation6 *anim, AssetStore *store) {
  anim->curr_frame_idx = (anim->curr_frame_idx + 1) % anim->total_frames;
}

// Adds delta_t and steps if needed. Returns true if step was made.
bool deltaTStepAnimation6(Animation6 *anim, float delta_t, AssetStore *store) {
  anim->since_last_frame += delta_t;
  if (anim->since_last_frame >= anim->frame_duration) {
    anim->since_last_frame -= anim->frame_duration;
    stepAnimation6(anim, store);
    return true;
  }
  return false;
}

// Assets management.
bool hasTex(TextureChoice choice, AssetStore *store) {
  return store->_texture_is_set[getTextureChoiceSeq(choice)];
}

Texture getTex(TextureChoice choice, AssetStore *store) {
  return store->_textures[getTextureChoiceSeq(choice)];
}

bool setTex(TextureChoice choice, Texture2D tex, AssetStore *store) {
  bool replaced = false;
  if (store->_texture_is_set[getTextureChoiceSeq(choice)])
    replaced = true;
  SetTextureFilter(tex, TEXTURE_FILTER_POINT); // To prevent anti-aliasing
  store->_texture_is_set[getTextureChoiceSeq(choice)] = true;
  store->_textures[getTextureChoiceSeq(choice)] = tex;
  return replaced;
}

bool removeTex(TextureChoice choice, AssetStore *store) {
  bool tex_is_set = store->_texture_is_set[getTextureChoiceSeq(choice)];
  store->_texture_is_set[getTextureChoiceSeq(choice)] = false;
  return tex_is_set;
}

bool freeTex(TextureChoice choice, AssetStore *store) {
  if (!removeTex(choice, store))
    return false;
  UnloadTexture(store->_textures[getTextureChoiceSeq(choice)]);
  return true;
}

bool freeSetTex(TextureChoice choice, Texture2D tex, AssetStore *store) {
  bool was_freed = freeTex(choice, store);
  setTex(choice, tex, store);
  return was_freed;
}

bool hasSound(SoundChoice choice, AssetStore *store) {
  return store->_sound_is_set[choice];
}

bool setSound(SoundChoice choice, Sound sound, AssetStore *store) {
  bool replaced = false;
  if (store->_sound_is_set[choice])
    replaced = true;
  store->_sound_is_set[choice] = true;
  store->_sounds[choice] = sound;
  return replaced;
}

bool removeSound(SoundChoice choice, AssetStore *store) {
  bool sound_is_set = store->_sound_is_set[choice];
  store->_sound_is_set[choice] = false;
  return sound_is_set;
}

bool freeSound(SoundChoice choice, AssetStore *store) {
  if (!removeSound(choice, store))
    return false;
  UnloadSound(store->_sounds[choice]);
  return true;
}

bool freeSetSound(SoundChoice choice, Sound sound, AssetStore *store) {
  bool was_freed = freeSound(choice, store);
  setSound(choice, sound, store);
  return was_freed;
}

void freeAssetStore(AssetStore *store) {
  for (int i = 0; i < _TEX_LEN; ++i)
    if (store->_texture_is_set[i])
      UnloadTexture(store->_textures[i]);

  for (int i = 0; i < SOUND_LEN; ++i)
    if (store->_sound_is_set[i])
      UnloadSound(store->_sounds[i]);
}

// Tiles
bool addTileToTilePicker(Tile t, TilePicker *tp) {
  if ((tp->curr_row >= NUM_TILEPICKER_ROWS) ||
      (tp->curr_col >= NUM_TILEPICKER_COLS))
    return false;
  if ((t.tile_w != tp->tile_w) || (t.tile_h != tp->tile_h))
    return false;
  t.pos_x = tp->curr_col * tp->tile_w;
  t.pos_y = tp->curr_row * tp->tile_h;
  tp->tiles[tp->curr_row][tp->curr_col] = t;
  tp->has_tile[tp->curr_row][tp->curr_col] = true;
  bool need_shift = (tp->curr_row + 1 >= NUM_TILEPICKER_ROWS);
  tp->curr_row = need_shift ? 0 : tp->curr_row + 1;
  tp->curr_col = need_shift ? tp->curr_col + 1 : tp->curr_col;
  return true;
}
