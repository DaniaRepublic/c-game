/*******************************************************************************************
 *
 *   LayoutJungle v1.0.0 - Tool Description
 *
 *   MODULE USAGE:
 *       #define GUI_LAYOUT_JUNGLE_IMPLEMENTATION
 *       #include "gui_layout_jungle.h"
 *
 *       INIT: GuiLayoutJungleState state = InitGuiLayoutJungle();
 *       DRAW: GuiLayoutJungle(&state);
 *
 *   LICENSE: Propietary License
 *
 *   Copyright (c) 2022 raylib technologies. All Rights Reserved.
 *
 *   Unauthorized copying of this file, via any medium is strictly prohibited
 *   This project is proprietary and confidential unless the owner allows
 *   usage in any other form by expresely written permission.
 *
 **********************************************************************************************/

#include "raylib.h"

// WARNING: raygui implementation is expected to be defined before including
// this header
#undef RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <string.h> // Required for: strcpy()

#ifndef GUI_LAYOUT_JUNGLE_H
#define GUI_LAYOUT_JUNGLE_H

typedef struct {
  Vector2 anchor01;

  bool WindowBox000Active;
  float Slider001Value;
  Color ColorPicker003Value;

  // Custom state variables (depend on development software)
  // NOTE: This variables should be added manually if required

} GuiLayoutJungleState;

#ifdef __cplusplus
extern "C" { // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
GuiLayoutJungleState InitGuiLayoutJungle(void);
void GuiLayoutJungle(GuiLayoutJungleState *state);

#ifdef __cplusplus
}
#endif

#endif // GUI_LAYOUT_JUNGLE_H

/***********************************************************************************
 *
 *   GUI_LAYOUT_JUNGLE IMPLEMENTATION
 *
 ************************************************************************************/
#if defined(GUI_LAYOUT_JUNGLE_IMPLEMENTATION)

#include "raygui.h"

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Internal Module Functions Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

GuiLayoutJungleState InitGuiLayoutJungle(void) {
  GuiLayoutJungleState state = {0};

  state.anchor01 = (Vector2){48, 72};

  state.WindowBox000Active = true;
  state.Slider001Value = 0.0f;
  state.ColorPicker003Value = (Color){0, 0, 0, 0};

  // Custom variables initialization

  return state;
}

void GuiLayoutJungle(GuiLayoutJungleState *state) {
  const char *WindowBox000Text = "SAMPLE TEXT";
  const char *Slider001Text = "";
  const char *Label002Text = "CAMERA ZOOM";
  const char *Label004Text = "BG COLOR";
  const char *ColorPicker003Text = "";

  if (state->WindowBox000Active) {
    state->WindowBox000Active = !GuiWindowBox(
        (Rectangle){state->anchor01.x + 0, state->anchor01.y + 0, 168, 264},
        WindowBox000Text);
    GuiSlider(
        (Rectangle){state->anchor01.x + 24, state->anchor01.y + 72, 120, 16},
        Slider001Text, NULL, &state->Slider001Value, 0.2, 2);
    GuiLabel(
        (Rectangle){state->anchor01.x + 24, state->anchor01.y + 40, 120, 24},
        Label002Text);
    GuiColorPicker(
        (Rectangle){state->anchor01.x + 24, state->anchor01.y + 144, 96, 96},
        ColorPicker003Text, &state->ColorPicker003Value);
    GuiLabel(
        (Rectangle){state->anchor01.x + 24, state->anchor01.y + 112, 120, 24},
        Label004Text);
  }
}

#endif // GUI_LAYOUT_JUNGLE_IMPLEMENTATION
