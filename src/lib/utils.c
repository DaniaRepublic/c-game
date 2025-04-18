#include <stdio.h>

#include "raylib.h"

#include "utils.h"

// Window state management.
void setScreenDims(int *w, int *h) {
  *w = GetRenderWidth();
  *h = GetRenderHeight();
}

// Graphics
/// Returns texture at curr_frame_idx.
Texture getCurrFrameAnimation6(Animation6 anim, AssetStore *store) {
  return store->_textures[anim.frames[anim.curr_frame_idx]];
}

/// Simply updates curr_frame_idx, wraping if nessesary.
void stepAnimation6(Animation6 *anim, AssetStore *store) {
  anim->curr_frame_idx = (anim->curr_frame_idx + 1) % anim->total_frames;
}

/// Adds delta_t and steps if needed. Returns true if step was made.
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
  return store->_texture_is_set[choice];
}

bool setTex(TextureChoice choice, Texture2D tex, AssetStore *store) {
  bool replaced = false;
  if (store->_texture_is_set[choice])
    replaced = true;
  store->_texture_is_set[choice] = true;
  store->_textures[choice] = tex;
  return replaced;
}

bool removeTex(TextureChoice choice, AssetStore *store) {
  bool tex_is_set = store->_texture_is_set[choice];
  store->_texture_is_set[choice] = false;
  return tex_is_set;
}

bool freeTex(TextureChoice choice, AssetStore *store) {
  if (!removeTex(choice, store))
    return false;
  UnloadTexture(store->_textures[choice]);
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
  for (int i = 0; i < TEX_LEN; ++i)
    if (store->_texture_is_set[i])
      UnloadTexture(store->_textures[i]);

  for (int i = 0; i < SOUND_LEN; ++i)
    if (store->_sound_is_set[i])
      UnloadSound(store->_sounds[i]);
}
