#pragma once

#include <stdbool.h>

const float PHYS_TIME_STEP;

typedef struct {
  /// true if down
  bool w;
  /// true if down
  bool a;
  /// true if down
  bool s;
  /// true if down
  bool d;
  /// true if down
  bool l_shift;
  /// true if pressed
  bool e;
  /// true if pressed
  bool space;
} KeyboardInputs;
