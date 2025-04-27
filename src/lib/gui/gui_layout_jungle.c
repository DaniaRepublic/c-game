/***********************************************************************************
 *
 *   GUI_LAYOUT_NAME IMPLEMENTATION
 *
 ************************************************************************************/
#include "raygui.h"

#include "lib/gui/gui_layout_jungle.h"

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
  state.Slider001Value = 1.0f;
  state.ColorPicker003Value = (Color){0x40, 0x48, 0x60, 0xff};
  state.CheckBoxEx006Checked = true;

  // Custom variables initialization

  return state;
}

void GuiLayoutJungle(GuiLayoutJungleState *state) {
  const char *WindowBox000Text = "GAME VARIABLES";
  const char *Slider001Text = "";
  char Label002Text[32];
  sprintf(Label002Text, "CAMERA ZOOM: %.2f", state->Slider001Value);
  const char *ColorPicker003Text = "";
  const char *Label004Text = "BG COLOR";
  const char *CheckBoxEx006Text = "SHOW GRID";

  if (GuiWindowBox((Rectangle){state->anchor01.x + 0, state->anchor01.y + 0,
                               168, state->WindowBox000Active ? 312 : 0},
                   WindowBox000Text)) {
    state->WindowBox000Active = !state->WindowBox000Active;
  }

  if (state->WindowBox000Active) {
    GuiSlider(
        (Rectangle){state->anchor01.x + 24, state->anchor01.y + 72, 120, 24},
        Slider001Text, NULL, &state->Slider001Value, 0.25, 2);
    GuiLabel(
        (Rectangle){state->anchor01.x + 24, state->anchor01.y + 40, 120, 24},
        Label002Text);
    GuiColorPicker(
        (Rectangle){state->anchor01.x + 24, state->anchor01.y + 144, 96, 96},
        ColorPicker003Text, &state->ColorPicker003Value);
    GuiLabel(
        (Rectangle){state->anchor01.x + 24, state->anchor01.y + 112, 120, 24},
        Label004Text);
    GuiCheckBox(
        (Rectangle){state->anchor01.x + 24, state->anchor01.y + 264, 24, 24},
        CheckBoxEx006Text, &state->CheckBoxEx006Checked);
  }
}
