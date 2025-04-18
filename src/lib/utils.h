#pragma once

#include "raylib.h"

void setScreenDims(int *w, int *h);

typedef enum {
  TEX_TILEMAP_CONCRETE,
  TEX_HERO,
  TEX_GHOST1,
  TEX_GHOST2,
  TEX_GHOST3,
  TEX_GHOST4,
  // This has to be the last enum here !!!
  TEX_LEN,
} TextureChoice;

typedef enum SoundChoice {
  SOUND_SHOT,
  SOUND_HIT,
  // This has to be the last enum here !!!
  SOUND_LEN,
} SoundChoice;

/// Don't update fields directly, call has/set functions on the struct.
typedef struct {
  Texture2D _textures[TEX_LEN];
  bool _texture_is_set[TEX_LEN];
  Sound _sounds[SOUND_LEN];
  bool _sound_is_set[SOUND_LEN];
} AssetStore;

/// Tiles must have the same size.
typedef struct {
  TextureChoice tex_choice;
  /// in pixels
  int tile_w, tile_h;
  int num_x, num_y;
} Tilemap;

/// Used for animation with a maximum of 6 frames.
typedef struct {
  int frames[6];
  int curr_frame_idx;
  int total_frames;
  /// in sec
  float frame_duration;
  /// in sec
  float since_last_frame;
} Animation6;

Texture getCurrFrameAnimation6(Animation6 anim, AssetStore *store);

void stepAnimation6(Animation6 *anim, AssetStore *store);

bool deltaTStepAnimation6(Animation6 *anim, float delta_t, AssetStore *store);

/// Returns true if choice is in AssetStore.
bool hasTex(TextureChoice choice, AssetStore *store);

/// Returns true if texture was replaced.
bool setTex(TextureChoice choice, Texture2D tex, AssetStore *store);

/// Returns true if texture was removed.
bool removeTex(TextureChoice choice, AssetStore *store);

/// Returns true if texture was freed.
bool freeTex(TextureChoice choice, AssetStore *store);

/// Returns true if texture was freed.
bool freeSetTex(TextureChoice choice, Texture2D tex, AssetStore *store);

/// Returns true if choice is in AssetStore.
bool hasSound(SoundChoice choice, AssetStore *store);

/// Returns true if sound was replaced.
bool setSound(SoundChoice choice, Sound sound, AssetStore *store);

/// Returns true if sound was removed.
bool removeSound(SoundChoice choice, AssetStore *store);

/// Returns true if sound was freed.
bool freeSound(SoundChoice choice, AssetStore *store);

/// Returns true if texture was freed.
bool freeSetSound(SoundChoice choice, Sound sound, AssetStore *store);

void freeAssetStore(AssetStore *store);
