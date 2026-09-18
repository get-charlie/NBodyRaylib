#ifndef _WIDGETS_H
#define _WIDGETS_H

#include <stdbool.h>

#include "raylib.h"

// Small immediate mode widget kit shared by the toolbar, the menu and the
// forms. Every widget draws itself and reports what the mouse did with it
// on the same call, there is no retained state beyond text editing.

#define UI_TEXT         18
#define UI_SMALL        14
#define UI_TITLE        20
#define UI_ROW          26.0f
#define UI_PAD          10.0f
#define UI_REPEAT_DELAY 0.4
#define UI_REPEAT_RATE  0.04

Color ui_color_panel(void);
Color ui_color_line(void);
Color ui_color_text(void);
Color ui_color_dim(void);
Color ui_color_accent(void);

bool ui_mouse_in(Rectangle area);

void ui_panel(Rectangle area, const char* title);
void ui_label(float x, float y, const char* text, int size, Color color);
void ui_separator(Rectangle area, const char* text);

// Buttons report a click on release, active draws them as pressed, a
// disabled button is drawn greyed out and never reports anything.
bool ui_button(Rectangle area, const char* text, bool active);
bool ui_button_ex(Rectangle area, const char* text, bool active, bool enabled);

// Text box with a label on its left, focused draws the caret
void ui_field(Rectangle area, const char* label, const char* text, bool focused);

// Keyboard editing of a text buffer, numeric only accepts what strtod can
// read back. repeat carries the backspace autorepeat timer of the caller.
void ui_edit(char* text, int capacity, bool numeric, double* repeat);

// Reads a number the way the forms expect it, returns false if the text is
// not a valid number
bool ui_parse(const char* text, double* out);

#endif
