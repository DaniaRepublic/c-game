#include <stdbool.h>
#include <stdio.h>

#include "components/components.h"
#include "lib/ini.h"
#include "raylib.h"

#include "utils.h"

/*  Example settings.ini:
 *
 *  [settings]
 *  WindowBox000Active = 1
 *  Slider001Value = 0.80
 *  ColorPicker003Value = { 200, 200, 200, 255 }
 *  CheckBoxEx006Checked = 0
 */
static int settingsParser(void *user, const char *section, const char *name,
                          const char *value) {
  GuiLayoutJungleState *settings = (GuiLayoutJungleState *)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
  if (MATCH("settings", "WindowBox000Active")) {
    settings->WindowBox000Active = atoi(value) == 1;
  } else if (MATCH("settings", "Slider001Value")) {
    settings->Slider001Value = atof(value);
  } else if (MATCH("settings", "CheckBoxEx006Checked")) {
    settings->CheckBoxEx006Checked = atoi(value) == 1;
  } else if (MATCH("settings", "ColorPicker003Value")) {
    short val_pos = 0;
    char val[5];
    short part = 0;
    for (int i = 0; i < strlen(value); ++i) {
      char curr_char = value[i];
      switch (curr_char) {
      case '{': {
        break;
      }
      case '}': {
        break;
      }
      case ' ': {
        break;
      }
      case ',': {
        val[val_pos] = '\0';
        val_pos = 0;
        int ival = atoi(val);
        switch (part) {
        case 0: {
          settings->ColorPicker003Value.r = ival;
        }
        case 1: {
          settings->ColorPicker003Value.g = ival;
        }
        case 2: {
          settings->ColorPicker003Value.b = ival;
        }
        case 3: {
          settings->ColorPicker003Value.a = ival;
        }
        }
        ++part;
        break;
      }
      default: {
        val[val_pos] = curr_char;
        ++val_pos;
      }
      }
    }
  }

  return 1;
}

bool readSettingsFromFile(const char *filename, GuiLayoutJungleState *state) {
  int res = ini_parse(filename, settingsParser, state);
  if (res != 0) {
    switch (res) {
    case -1: {
      perror("Couldn't open settings.ini\n");
      break;
    }
    case -2: {
      perror("Memory alloc error reading settings.ini\n");
      break;
    }
    default: {
      printf("Error parsing settings.ini on line %d\n", res);
    }
    }
    printf("Using default settings\n");
    return false;
  }

  return true;
}

bool writeSettingsToFile(const char *filename,
                         const GuiLayoutJungleState *state) {
  FILE *file = fopen(filename, "w");
  if (!file) {
    return false;
  }

  fprintf(file, "[settings]\n");
  fprintf(file, "WindowBox000Active = %d\n", state->WindowBox000Active ? 1 : 0);
  fprintf(file, "Slider001Value = %.2f\n", state->Slider001Value);
  fprintf(file, "ColorPicker003Value = { %d, %d, %d, %d }\n",
          state->ColorPicker003Value.r, state->ColorPicker003Value.g,
          state->ColorPicker003Value.b, state->ColorPicker003Value.a);
  fprintf(file, "CheckBoxEx006Checked = %d\n",
          state->CheckBoxEx006Checked ? 1 : 0);

  fclose(file);
  return true;
}

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
Texture getCurrFrameAnimation(const Animation *anim, const AssetStore *store) {
  return getTex(anim->frames[anim->curr_frame_idx], store);
}

// Simply updates curr_frame_idx, wraping if nessesary.
void stepAnimation(Animation *anim) {
  anim->curr_frame_idx = (anim->curr_frame_idx + 1) % anim->total_frames;
}

// Adds delta_t and steps if needed. Returns true if step was made.
bool deltaTStepAnimation(Animation *anim, float delta_t,
                         const AssetStore *store) {
  anim->since_last_frame += delta_t;
  if (anim->since_last_frame >= anim->frame_duration) {
    anim->since_last_frame -= anim->frame_duration;
    stepAnimation(anim);
    return true;
  }
  return false;
}

// Assets management.
bool hasTex(TextureChoice choice, AssetStore *store) {
  return store->_texture_is_set[getTextureChoiceSeq(choice)];
}

Texture getTex(TextureChoice choice, const AssetStore *store) {
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
  // tile picker row or col out of range
  if ((tp->curr_row >= NUM_TILEPICKER_ROWS) ||
      (tp->curr_col >= NUM_TILEPICKER_COLS))
    return false;
  // tile dimension doesn't match
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
