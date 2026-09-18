#include "saver.h"

#include <stdio.h>
#include <string.h>

#include "widgets.h"

#define SAVER_WIDTH     560.0f
#define SAVER_HEIGHT    440.0f
#define LIST_ROWS       8
#define LIST_ROW        24.0f
#define MESSAGE_TIME    5.0

static void set_message(Saver* saver, const char* text, bool error)
{
    snprintf(saver->message, SAVE_MESSAGE_LEN, "%s", text);
    saver->message_error = error;
    saver->message_time = MESSAGE_TIME;
}

void saver_init(Saver* saver)
{
    *saver = (Saver){0};
    snprintf(saver->name, PATH_LEN, "simulation.json");
}

void saver_open(Saver* saver, const char* suggested)
{
    saver->active = true;
    saver->scroll = 0;
    saver->file_count = list_json_files(saver->files, MAX_FILES);
    if(suggested != NULL && suggested[0] != '\0'){
        snprintf(saver->name, PATH_LEN, "%s", suggested);
    }
    while(GetCharPressed() > 0){}
}

void saver_close(Saver* saver)
{
    saver->active = false;
}

Rectangle saver_area(void)
{
    return (Rectangle){
        (GetScreenWidth() - SAVER_WIDTH) * 0.5f,
        (GetScreenHeight() - SAVER_HEIGHT) * 0.5f,
        SAVER_WIDTH,
        SAVER_HEIGHT
    };
}

// The name always ends up with a json extension, that is what the loader
// and the dialogs look for
static void full_path(const Saver* saver, char* out, int capacity)
{
    const char* name = saver->name;
    if(strstr(name, ".json") != NULL){
        snprintf(out, capacity, "%s", name);
    }
    else{
        snprintf(out, capacity, "%.*s.json", capacity - 6, name);
    }
}

static bool target_exists(const Saver* saver)
{
    char path[PATH_LEN];
    full_path(saver, path, PATH_LEN);
    return FileExists(path);
}

static void store(Saver* saver, const Simulation* simulation)
{
    if(saver->name[0] == '\0'){
        set_message(saver, "the file needs a name", true);
        return;
    }
    char path[PATH_LEN];
    full_path(saver, path, PATH_LEN);

    bool replaced = FileExists(path);
    if(save_simulation(simulation, path)){
        set_message(saver, TextFormat("could not write %s", path), true);
        return;
    }
    set_message(saver, TextFormat("%s %s", replaced ? "replaced" : "saved", path), false);
    saver->file_count = list_json_files(saver->files, MAX_FILES);
    saver_close(saver);
}

bool saver_update(Saver* saver, const Simulation* simulation)
{
    if(saver->message_time > 0.0){
        saver->message_time -= GetFrameTime();
    }
    if(!saver->active){
        return false;
    }

    ui_edit(saver->name, PATH_LEN, false, &saver->repeat);

    if(IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)){
        store(saver, simulation);
    }
    if(IsKeyPressed(KEY_ESCAPE)){
        saver_close(saver);
    }
    return true;
}

// Buttons are resolved while drawing, the same way the toolbar does it
void saver_draw(Saver* saver, const Simulation* simulation)
{
    if(!saver->active){
        if(saver->message_time > 0.0){
            DrawText(saver->message, (int)TOOLBAR_WIDTH + 20, GetScreenHeight() - 36,
                     UI_TEXT, saver->message_error ? ORANGE : GREEN);
        }
        return;
    }

    Rectangle area = saver_area();
    ui_panel(area, "SAVE SIMULATION");

    float x = area.x + 20.0f;
    float width = area.width - 40.0f;
    float y = area.y + 48.0f;

    ui_label(x, y, TextFormat("%u bodies, scale %.6g", simulation->count, simulation->scale), UI_SMALL, ui_color_dim());
    y += 24.0f;

    ui_field((Rectangle){ x, y, width, 28.0f }, NULL, saver->name, true);
    y += 34.0f;

    bool exists = target_exists(saver);
    ui_label(x, y, exists ? "the file already exists, saving replaces it" : "a new file will be created",
             UI_SMALL, exists ? ORANGE : ui_color_dim());
    y += 26.0f;

    ui_label(x, y, "existing simulations (click to replace one)", UI_SMALL, ui_color_dim());
    y += 20.0f;

    Rectangle list = { x, y, width, LIST_ROWS * LIST_ROW + 8.0f };
    DrawRectangleRec(list, (Color){ 20, 20, 28, 255 });
    DrawRectangleLinesEx(list, 1.0f, ui_color_line());

    if(ui_mouse_in(list)){
        saver->scroll -= (int)GetMouseWheelMove();
    }
    int max_scroll = saver->file_count - LIST_ROWS;
    if(max_scroll < 0){
        max_scroll = 0;
    }
    if(saver->scroll > max_scroll){
        saver->scroll = max_scroll;
    }
    if(saver->scroll < 0){
        saver->scroll = 0;
    }

    for(int row = 0; row < LIST_ROWS; row++){
        int index = saver->scroll + row;
        if(index >= saver->file_count){
            break;
        }
        Rectangle item = { list.x + 4.0f, list.y + 4.0f + row * LIST_ROW, list.width - 8.0f, LIST_ROW };
        bool hover = ui_mouse_in(item);
        if(hover){
            DrawRectangleRec(item, (Color){ 40, 40, 60, 255 });
            if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
                snprintf(saver->name, PATH_LEN, "%s", saver->files[index]);
            }
        }
        DrawText(saver->files[index], (int)item.x + 6, (int)item.y + 5, UI_SMALL, ui_color_text());
    }
    if(saver->file_count == 0){
        ui_label(list.x + 8.0f, list.y + 8.0f, "no json files here", UI_SMALL, ui_color_dim());
    }
    y += list.height + 14.0f;

    if(saver->message_time > 0.0){
        ui_label(x, y, saver->message, UI_SMALL, saver->message_error ? ORANGE : GREEN);
    }

    float button = 120.0f;
    float bottom = area.y + area.height - 44.0f;
    if(ui_button((Rectangle){ area.x + area.width - 2 * button - 32.0f, bottom, button, 30.0f }, "Cancel", false)){
        saver_close(saver);
    }
    if(ui_button((Rectangle){ area.x + area.width - button - 20.0f, bottom, button, 30.0f },
                 exists ? "Replace" : "Save", false)){
        store(saver, simulation);
    }
}
