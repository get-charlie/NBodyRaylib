#include "widgets.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

Color ui_color_panel(void)  { return (Color){ 15, 15, 22, 235 }; }
Color ui_color_line(void)   { return (Color){ 70, 70, 90, 255 }; }
Color ui_color_text(void)   { return (Color){ 225, 225, 235, 255 }; }
Color ui_color_dim(void)    { return (Color){ 150, 150, 165, 255 }; }
Color ui_color_accent(void) { return (Color){ 90, 160, 235, 255 }; }

bool ui_mouse_in(Rectangle area)
{
    return CheckCollisionPointRec(GetMousePosition(), area);
}

void ui_panel(Rectangle area, const char* title)
{
    DrawRectangleRec(area, ui_color_panel());
    DrawRectangleLinesEx(area, 1.0f, ui_color_line());
    if(title != NULL){
        DrawText(title, (int)area.x + 10, (int)area.y + 8, UI_TITLE, ui_color_text());
    }
}

void ui_label(float x, float y, const char* text, int size, Color color)
{
    DrawText(text, (int)x, (int)y, size, color);
}

void ui_separator(Rectangle area, const char* text)
{
    float y = area.y + area.height * 0.5f;
    if(text != NULL){
        DrawText(text, (int)area.x, (int)(area.y + 2), UI_SMALL - 2, ui_color_dim());
        float left = area.x + MeasureText(text, UI_SMALL - 2) + 6.0f;
        DrawLine((int)left, (int)y, (int)(area.x + area.width), (int)y, ui_color_line());
    }
    else{
        DrawLine((int)area.x, (int)y, (int)(area.x + area.width), (int)y, ui_color_line());
    }
}

bool ui_button_ex(Rectangle area, const char* text, bool active, bool enabled)
{
    bool hover = enabled && ui_mouse_in(area);
    bool held  = hover && IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    Color fill = (Color){ 30, 30, 42, 255 };
    if(active){
        fill = (Color){ 40, 70, 110, 255 };
    }
    if(held){
        fill = (Color){ 60, 100, 150, 255 };
    }
    else if(hover){
        fill = (Color){ 45, 45, 62, 255 };
    }
    if(!enabled){
        fill = (Color){ 24, 24, 30, 255 };
    }

    DrawRectangleRec(area, fill);
    DrawRectangleLinesEx(area, 1.0f, active ? ui_color_accent() : ui_color_line());

    int size = UI_SMALL;
    int width = MeasureText(text, size);
    while(width > area.width - 8.0f && size > 8){
        size--;
        width = MeasureText(text, size);
    }
    Color label = enabled ? (active ? WHITE : ui_color_text()) : (Color){ 90, 90, 100, 255 };
    DrawText(text, (int)(area.x + (area.width - width) * 0.5f),
                   (int)(area.y + (area.height - size) * 0.5f), size, label);

    return enabled && hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

bool ui_button(Rectangle area, const char* text, bool active)
{
    return ui_button_ex(area, text, active, true);
}

void ui_field(Rectangle area, const char* label, const char* text, bool focused)
{
    if(label != NULL){
        DrawText(label, (int)(area.x - MeasureText(label, UI_SMALL) - 8),
                        (int)(area.y + (area.height - UI_SMALL) * 0.5f), UI_SMALL, ui_color_dim());
    }
    DrawRectangleRec(area, focused ? (Color){ 40, 40, 60, 255 } : (Color){ 24, 24, 32, 255 });
    DrawRectangleLinesEx(area, 1.0f, focused ? ui_color_accent() : ui_color_line());

    const char* shown = text;
    if(focused && fmod(GetTime(), 1.0) < 0.5){
        shown = TextFormat("%s_", text);
    }
    DrawText(shown, (int)area.x + 6, (int)(area.y + (area.height - UI_SMALL) * 0.5f), UI_SMALL, ui_color_text());
}

static bool valid_char(int key, bool numeric)
{
    if(!numeric){
        return key >= 32 && key < 127;
    }
    return isdigit(key) || key == '.' || key == '-' || key == '+' || key == 'e' || key == 'E';
}

void ui_edit(char* text, int capacity, bool numeric, double* repeat)
{
    int key = GetCharPressed();
    while(key > 0){
        size_t length = strlen(text);
        if(valid_char(key, numeric) && (int)length < capacity - 1){
            text[length] = (char)key;
            text[length + 1] = '\0';
        }
        key = GetCharPressed();
    }

    size_t length = strlen(text);
    if(IsKeyPressed(KEY_BACKSPACE)){
        if(length > 0){
            text[length - 1] = '\0';
        }
        *repeat = UI_REPEAT_DELAY;
    }
    else if(IsKeyDown(KEY_BACKSPACE)){
        *repeat -= GetFrameTime();
        if(*repeat <= 0.0){
            if(length > 0){
                text[length - 1] = '\0';
            }
            *repeat = UI_REPEAT_RATE;
        }
    }
}

bool ui_parse(const char* text, double* out)
{
    char* end = NULL;
    double value = strtod(text, &end);
    if(end == text){
        return false;
    }
    while(isspace((unsigned char)*end)){
        end++;
    }
    if(*end != '\0'){
        return false;
    }
    *out = value;
    return true;
}
