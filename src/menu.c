#include "menu.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "physics.h"
#include "widgets.h"

#define MENU_WIDTH      620.0f
#define MENU_HEIGHT     520.0f
#define LIST_ROWS       10
#define LIST_ROW        26.0f
#define DEFAULT_SCALE   250000.0

static const char* labels[RANDOM_FIELDS] = {
    "bodies", "star mass kg", "mass min kg", "mass max kg",
    "orbit min AU", "orbit max AU", "thickness AU", "radius", "scale Un/AU", "seed"
};

static const char* defaults[RANDOM_FIELDS] = {
    "200", "1.989e30", "1e23", "5e25",
    "0.4", "6.0", "0.35", "300", "250000", "1"
};

static void set_message(Menu* menu, const char* text, bool error)
{
    snprintf(menu->message, sizeof(menu->message), "%s", text);
    menu->message_error = error;
}

void menu_init(Menu* menu)
{
    *menu = (Menu){0};
    menu->selected = -1;
    for(int i = 0; i < RANDOM_FIELDS; i++){
        snprintf(menu->values[i], RANDOM_TEXT_LEN, "%s", defaults[i]);
    }
}

void menu_open(Menu* menu, bool can_resume)
{
    menu->page = PAGE_MAIN;
    menu->can_resume = can_resume;
    menu->message[0] = '\0';
    while(GetCharPressed() > 0){}
}

static Rectangle menu_area(void)
{
    return (Rectangle){
        (GetScreenWidth() - MENU_WIDTH) * 0.5f,
        (GetScreenHeight() - MENU_HEIGHT) * 0.5f,
        MENU_WIDTH,
        MENU_HEIGHT
    };
}

// Stars sit at the origin of the system they light, the rest of the bodies
// are placed on a disc around them with the velocity of a circular orbit,
// so a random simulation starts as something that holds together.
static void generate_random(Menu* menu, Simulation* simulation)
{
    double value[RANDOM_FIELDS];
    for(int i = 0; i < RANDOM_FIELDS; i++){
        if(!ui_parse(menu->values[i], &value[i])){
            menu->field = i;
            set_message(menu, TextFormat("invalid %s", labels[i]), true);
            return;
        }
    }

    int count = (int)value[RANDOM_BODIES];
    double star_mass = value[RANDOM_STAR_MASS];
    double mass_min  = value[RANDOM_MASS_MIN];
    double mass_max  = value[RANDOM_MASS_MAX];
    double orbit_min = value[RANDOM_ORBIT_MIN];
    double orbit_max = value[RANDOM_ORBIT_MAX];
    double thickness = value[RANDOM_THICKNESS];
    double radius    = value[RANDOM_RADIUS];
    double scale     = value[RANDOM_SCALE];

    if(count < 1 || count > MAX_BODIES - 1){
        menu->field = RANDOM_BODIES;
        set_message(menu, TextFormat("bodies must be between 1 and %d", MAX_BODIES - 1), true);
        return;
    }
    if(mass_min <= 0.0 || mass_max < mass_min){
        menu->field = RANDOM_MASS_MIN;
        set_message(menu, "the mass range is not valid", true);
        return;
    }
    if(orbit_min <= 0.0 || orbit_max < orbit_min){
        menu->field = RANDOM_ORBIT_MIN;
        set_message(menu, "the orbit range is not valid", true);
        return;
    }
    if(radius <= 0.0 || scale <= 0.0){
        menu->field = RANDOM_RADIUS;
        set_message(menu, "radius and scale must be positive", true);
        return;
    }

    *simulation = (Simulation){0};
    simulation->scale = scale;
    srand((unsigned)value[RANDOM_SEED]);

    if(star_mass > 0.0){
        Body star = new_body("Star", (Color){ 255, 220, 60, 255 }, star_mass, radius * 6.0,
            vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), scale);
        add_simulation_body(simulation, star);
    }

    for(int i = 0; i < count; i++){
        double u = (double)rand() / RAND_MAX;
        double angle = 2.0 * PI * ((double)rand() / RAND_MAX);
        double height = thickness * (((double)rand() / RAND_MAX) * 2.0 - 1.0);
        double distance = orbit_min + (orbit_max - orbit_min) * sqrt(u);

        Vec3 position = vec3(distance * cos(angle), distance * sin(angle), height);

        // Masses are drawn on a logarithmic scale so a wide range does not
        // end up being all heavy bodies
        double t = (double)rand() / RAND_MAX;
        double mass = mass_min * pow(mass_max / mass_min, t);

        Vec3 velocity = vec3(0.0, 0.0, 0.0);
        if(star_mass > 0.0){
            double speed = sqrt(G_CONST * star_mass / (vec3_length(position) * AU)) / KM;
            double jitter = 0.95 + 0.1 * ((double)rand() / RAND_MAX);
            velocity = vec3(-sin(angle) * speed * jitter, cos(angle) * speed * jitter, 0.0);
        }
        else{
            double speed = 0.5 + 2.0 * ((double)rand() / RAND_MAX);
            velocity = vec3(-sin(angle) * speed, cos(angle) * speed, 0.0);
        }

        Color color = ColorFromHSV((float)(360.0 * ((double)rand() / RAND_MAX)), 0.55f, 1.0f);
        Body body = new_body(TextFormat("B%d", i + 1), color, mass, radius, position, velocity, scale);
        if(!add_simulation_body(simulation, body)){
            break;
        }
    }

    snprintf(menu->source, PATH_LEN, "random.json");
    set_message(menu, "", false);
}

static bool load_selected(Menu* menu, Simulation* simulation)
{
    if(menu->selected < 0 || menu->selected >= menu->file_count){
        set_message(menu, "pick a file first", true);
        return false;
    }
    const char* path = menu->files[menu->selected];
    if(load_simulation(simulation, path)){
        set_message(menu, TextFormat("could not load %s", path), true);
        return false;
    }
    snprintf(menu->source, PATH_LEN, "%s", GetFileName(path));
    set_message(menu, "", false);
    return true;
}

static void draw_title(Rectangle area, const char* subtitle)
{
    DrawText("BodySim", (int)area.x + 24, (int)area.y + 22, 40, ui_color_text());
    DrawText(subtitle, (int)area.x + 24, (int)area.y + 68, UI_TEXT, ui_color_dim());
}

static MenuResult page_main(Menu* menu, Simulation* simulation)
{
    Rectangle area = menu_area();
    ui_panel(area, NULL);
    draw_title(area, "n-body simulation");

    float x = area.x + 24.0f;
    float width = area.width - 48.0f;
    float y = area.y + 120.0f;
    MenuResult result = MENU_NONE;

    if(ui_button((Rectangle){ x, y, width, 44.0f }, "New empty simulation", false)){
        *simulation = (Simulation){0};
        simulation->scale = DEFAULT_SCALE;
        snprintf(menu->source, PATH_LEN, "simulation.json");
        result = MENU_START;
    }
    DrawText("start with nothing and add bodies with the creator",
             (int)x + 8, (int)(y + 48.0f), UI_SMALL, ui_color_dim());
    y += 76.0f;

    if(ui_button((Rectangle){ x, y, width, 44.0f }, "Open simulation...", false)){
        menu->page = PAGE_OPEN;
        menu->file_count = list_json_files(menu->files, MAX_FILES);
        menu->selected = -1;
        menu->scroll = 0;
        set_message(menu, "", false);
    }
    DrawText("load one of the json files of this folder or of examples/",
             (int)x + 8, (int)(y + 48.0f), UI_SMALL, ui_color_dim());
    y += 76.0f;

    if(ui_button((Rectangle){ x, y, width, 44.0f }, "Random simulation...", false)){
        menu->page = PAGE_RANDOM;
        set_message(menu, "", false);
    }
    DrawText("generate bodies with the mass, orbit and size you choose",
             (int)x + 8, (int)(y + 48.0f), UI_SMALL, ui_color_dim());
    y += 86.0f;

    if(menu->can_resume){
        if(ui_button((Rectangle){ x, y, width * 0.5f - 6.0f, 36.0f }, "Back to simulation", false)){
            result = MENU_RESUME;
        }
    }
    if(ui_button((Rectangle){ x + width * 0.5f + 6.0f, y, width * 0.5f - 6.0f, 36.0f }, "Quit", false)){
        result = MENU_QUIT;
    }

    if(menu->message[0] != '\0'){
        DrawText(menu->message, (int)x, (int)(area.y + area.height - 32.0f), UI_SMALL,
                 menu->message_error ? ORANGE : ui_color_dim());
    }
    return result;
}

static MenuResult page_open(Menu* menu, Simulation* simulation)
{
    Rectangle area = menu_area();
    ui_panel(area, NULL);
    draw_title(area, "open a simulation");

    float x = area.x + 24.0f;
    float width = area.width - 48.0f;
    float y = area.y + 110.0f;
    MenuResult result = MENU_NONE;

    Rectangle list = { x, y, width, LIST_ROWS * LIST_ROW + 8.0f };
    DrawRectangleRec(list, (Color){ 20, 20, 28, 255 });
    DrawRectangleLinesEx(list, 1.0f, ui_color_line());

    if(ui_mouse_in(list)){
        menu->scroll -= (int)GetMouseWheelMove();
    }
    int max_scroll = menu->file_count - LIST_ROWS;
    if(max_scroll < 0){
        max_scroll = 0;
    }
    menu->scroll = menu->scroll < 0 ? 0 : (menu->scroll > max_scroll ? max_scroll : menu->scroll);

    for(int row = 0; row < LIST_ROWS; row++){
        int index = menu->scroll + row;
        if(index >= menu->file_count){
            break;
        }
        Rectangle item = { list.x + 4.0f, list.y + 4.0f + row * LIST_ROW, list.width - 8.0f, LIST_ROW };
        bool hover = ui_mouse_in(item);
        if(index == menu->selected){
            DrawRectangleRec(item, (Color){ 40, 70, 110, 255 });
        }
        else if(hover){
            DrawRectangleRec(item, (Color){ 40, 40, 60, 255 });
        }
        if(hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
            menu->selected = index;
        }
        DrawText(menu->files[index], (int)item.x + 8, (int)item.y + 6, UI_SMALL, ui_color_text());
    }
    if(menu->file_count == 0){
        DrawText("no json files found", (int)list.x + 8, (int)list.y + 8, UI_SMALL, ui_color_dim());
    }
    y += list.height + 16.0f;

    if(menu->message[0] != '\0'){
        DrawText(menu->message, (int)x, (int)y, UI_SMALL, menu->message_error ? ORANGE : ui_color_dim());
    }

    float button = 130.0f;
    float bottom = area.y + area.height - 54.0f;
    if(ui_button((Rectangle){ x, bottom, button, 34.0f }, "Back", false) || IsKeyPressed(KEY_ESCAPE)){
        menu->page = PAGE_MAIN;
    }
    bool ready = menu->selected >= 0;
    if(ui_button_ex((Rectangle){ area.x + area.width - button - 24.0f, bottom, button, 34.0f }, "Open", false, ready)
       || (ready && IsKeyPressed(KEY_ENTER))){
        if(load_selected(menu, simulation)){
            result = MENU_START;
        }
    }
    return result;
}

static MenuResult page_random(Menu* menu, Simulation* simulation)
{
    Rectangle area = menu_area();
    ui_panel(area, NULL);
    draw_title(area, "random simulation");

    float x = area.x + 150.0f;
    float width = area.width - 174.0f;
    float y = area.y + 104.0f;
    MenuResult result = MENU_NONE;

    for(int i = 0; i < RANDOM_FIELDS; i++){
        Rectangle field = { x, y + i * 28.0f, width, 24.0f };
        if(ui_mouse_in(field) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
            menu->field = i;
        }
        ui_field(field, labels[i], menu->values[i], menu->field == i);
    }

    ui_edit(menu->values[menu->field], RANDOM_TEXT_LEN, true, &menu->repeat);
    if(IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_DOWN)){
        bool back = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        menu->field = (menu->field + RANDOM_FIELDS + (back ? -1 : 1)) % RANDOM_FIELDS;
    }
    if(IsKeyPressed(KEY_UP)){
        menu->field = (menu->field + RANDOM_FIELDS - 1) % RANDOM_FIELDS;
    }

    float bottom = area.y + area.height - 54.0f;
    if(menu->message[0] != '\0'){
        DrawText(menu->message, (int)(area.x + 24.0f), (int)(bottom - 26.0f), UI_SMALL,
                 menu->message_error ? ORANGE : ui_color_dim());
    }

    float button = 130.0f;
    if(ui_button((Rectangle){ area.x + 24.0f, bottom, button, 34.0f }, "Back", false) || IsKeyPressed(KEY_ESCAPE)){
        menu->page = PAGE_MAIN;
    }
    if(ui_button((Rectangle){ area.x + area.width - button - 24.0f, bottom, button, 34.0f }, "Generate", false)
       || IsKeyPressed(KEY_ENTER)){
        generate_random(menu, simulation);
        if(simulation->count > 0){
            result = MENU_START;
        }
    }
    return result;
}

MenuResult menu_run(Menu* menu, Simulation* simulation)
{
    switch(menu->page){
        case PAGE_OPEN:   return page_open(menu, simulation);
        case PAGE_RANDOM: return page_random(menu, simulation);
        default:          return page_main(menu, simulation);
    }
}
