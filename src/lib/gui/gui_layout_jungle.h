/*******************************************************************************************
 *
 *   LayoutName v1.0.0 - Tool Description
 *
 *   MODULE USAGE:
 *       #define GUI_LAYOUT_NAME_IMPLEMENTATION
 *       #include "gui_layout_name.h"
 *
 *       INIT: GuiLayoutJungleState state = InitGuiLayoutName();
 *       DRAW: GuiLayoutName(&state);
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

#pragma once

#include "raylib.h"
#include <stdio.h>

// WARNING: raygui implementation is expected to be defined before including
// this header
#undef RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <string.h> // Required for: strcpy()

#ifndef GUI_LAYOUT_NAME_H
#define GUI_LAYOUT_NAME_H

#include "components/components.h"

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

#endif // GUI_LAYOUT_NAME_H
