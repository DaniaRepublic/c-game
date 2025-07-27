#pragma once

#include "box2d/math_functions.h"
#include "raylib.h"

// gui implementations
#include "lib/gui/gui_layout_jungle.h"
#include "lib/gui/jungle.h"

#include "components/components.h"

Vector2 box2dToRaylibLengthUnit(b2Vec2 vec);

b2Vec2 raylibToBox2dLengthUnit(Vector2 vec);

Vector2 box2dToRaylibVec(b2Vec2 vec);

b2Vec2 raylibToBox2dVec(Vector2 vec);

float box2dToRaylibRot(b2Rot rot);

b2Rot raylibToBox2dRot(float rot);

static int settingsParser(void *user, const char *section, const char *name,
                          const char *value);
bool readSettingsFromFile(const char *filename, GuiLayoutJungleState *state);
bool writeSettingsToFile(const char *filename,
                         const GuiLayoutJungleState *state);

// Returns sequential idx if choice is in TextureChoice and -1 otherwise.
int getTextureChoiceSeq(TextureChoice choice);

typedef enum SoundChoice {
  // Sound bites have a band of [0, 1000)
  SOUND_SHOT = 0,
  SOUND_HIT = 1,
  // This has to be the last enum here !!!
  SOUND_LEN,
} SoundChoice;

// Don't update fields directly, call has/set functions on the struct.
typedef struct {
  Texture2D _textures[_TEX_LEN];
  bool _texture_is_set[_TEX_LEN];
  Sound _sounds[SOUND_LEN];
  bool _sound_is_set[SOUND_LEN];
} AssetStore;

// All tiles must have the same dimentions.
typedef struct {
  TextureChoice tex_choice;
  // in pixels
  int tile_w, tile_h;
  int num_x, num_y;
} Tileset;

#define NUM_TILEPICKER_ROWS 3
#define NUM_TILEPICKER_COLS 8

// Tiles are added top to bottom, left to right
typedef struct {
  Tile tiles[NUM_TILEPICKER_ROWS][NUM_TILEPICKER_COLS];
  bool has_tile[NUM_TILEPICKER_ROWS][NUM_TILEPICKER_COLS];
  int curr_row, curr_col;
  int tile_w, tile_h;
  float outline_thickness;
} TilePicker;

bool addTileToTilePicker(Tile, TilePicker *);

Texture getCurrFrameAnimation(const Animation *anim, const AssetStore *store);

void stepAnimation(Animation *anim);

bool deltaTStepAnimation(Animation *anim, float delta_t,
                         const AssetStore *store);

// Returns true if choice is in AssetStore.
bool hasTex(TextureChoice choice, AssetStore *store);

Texture getTex(TextureChoice choice, const AssetStore *store);

// Returns true if texture was replaced.
bool setTex(TextureChoice choice, Texture2D tex, AssetStore *store);

// Returns true if texture was removed.
bool removeTex(TextureChoice choice, AssetStore *store);

// Returns true if texture was freed.
bool freeTex(TextureChoice choice, AssetStore *store);

// Returns true if texture was freed.
bool freeSetTex(TextureChoice choice, Texture2D tex, AssetStore *store);

// Returns true if choice is in AssetStore.
bool hasSound(SoundChoice choice, AssetStore *store);

// Returns true if sound was replaced.
bool setSound(SoundChoice choice, Sound sound, AssetStore *store);

// Returns true if sound was removed.
bool removeSound(SoundChoice choice, AssetStore *store);

// Returns true if sound was freed.
bool freeSound(SoundChoice choice, AssetStore *store);

// Returns true if texture was freed.
bool freeSetSound(SoundChoice choice, Sound sound, AssetStore *store);

void freeAssetStore(AssetStore *store);
