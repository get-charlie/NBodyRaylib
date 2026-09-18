#include "creator.h"
#include "physics.h"
#include "widgets.h"

#include <stdio.h>
#include <string.h>

#define ROW_HEIGHT      26.0f
#define PANEL_WIDTH     320.0f
#define PANEL_TOP       210.0f
#define LABEL_WIDTH     110.0f
#define MESSAGE_TIME    4.0

static const char* labels[CREATOR_FIELDS] = {
    "name", "mass kg", "radius", "pos x AU", "pos y AU", "pos z AU",
    "vel x km/s", "vel y km/s", "vel z km/s", "color r", "color g", "color b"
};

Rectangle creator_area(void)
{
    return (Rectangle){
        GetScreenWidth() - PANEL_WIDTH - 20.0f,
        PANEL_TOP,
        PANEL_WIDTH,
        CREATOR_FIELDS * ROW_HEIGHT + 140.0f
    };
}

static Rectangle field_area(int field)
{
    Rectangle area = creator_area();
    return (Rectangle){
        area.x + LABEL_WIDTH,
        area.y + 40.0f + field * ROW_HEIGHT,
        area.width - LABEL_WIDTH - 12.0f,
        ROW_HEIGHT - 4.0f
    };
}

static void set_value(Creator* creator, int field, const char* text)
{
    snprintf(creator->values[field], CREATOR_TEXT_LEN, "%s", text);
}

static void set_message(Creator* creator, const char* text, bool error)
{
    snprintf(creator->message, MESSAGE_LEN, "%s", text);
    creator->message_error = error;
    creator->message_time = MESSAGE_TIME;
}

void creator_init(Creator* creator)
{
    *creator = (Creator){0};
    for(int i = 0; i < CREATOR_FIELDS; i++){
        set_value(creator, i, "0");
    }
    set_value(creator, FIELD_NAME, "Body");
    set_value(creator, FIELD_MASS, "1e24");
    set_value(creator, FIELD_COLOR_R, "255");
    set_value(creator, FIELD_COLOR_G, "200");
    set_value(creator, FIELD_COLOR_B, "80");
}

void creator_open(Creator* creator, const Simulation* simulation, const OrbitCamera* camera)
{
    creator->active = true;
    creator->field  = FIELD_NAME;

    set_value(creator, FIELD_NAME, TextFormat("Body %u", simulation->count + 1));
    // Radius is given in file units, like in the json, 0.002 AU is a size
    // that can be seen at the default zoom
    set_value(creator, FIELD_RADIUS, TextFormat("%g", simulation->scale * 0.002));

    // The camera focus is the most natural place to drop a new body
    Vec3 position = vec3_scale(camera->focus, 1.0 / AU);
    set_value(creator, FIELD_POS_X, TextFormat("%.6g", position.x));
    set_value(creator, FIELD_POS_Y, TextFormat("%.6g", position.y));
    set_value(creator, FIELD_POS_Z, TextFormat("%.6g", position.z));

    // While following a body, start from its velocity: adding a little on
    // top of it is enough to put the new body in orbit around it
    Vec3 velocity = vec3(0.0, 0.0, 0.0);
    if(camera->follow >= 0 && (unsigned)camera->follow < simulation->count){
        velocity = vec3_scale(simulation->bodies[camera->follow].velocity, 1.0 / KM);
    }
    set_value(creator, FIELD_VEL_X, TextFormat("%.6g", velocity.x));
    set_value(creator, FIELD_VEL_Y, TextFormat("%.6g", velocity.y));
    set_value(creator, FIELD_VEL_Z, TextFormat("%.6g", velocity.z));

    // The key that opened the form is still in the input queue
    while(GetCharPressed() > 0){}
}

void creator_close(Creator* creator)
{
    creator->active = false;
}

static bool read_field(Creator* creator, int field, double* out)
{
    if(!ui_parse(creator->values[field], out)){
        creator->field = field;
        set_message(creator, TextFormat("invalid %s", labels[field]), true);
        return false;
    }
    return true;
}

static unsigned char to_channel(double value)
{
    return (unsigned char)Clamp(value, 0.0, 255.0);
}

static void creator_submit(Creator* creator, Simulation* simulation, DisplayFlags* flags)
{
    double values[CREATOR_FIELDS] = {0};
    for(int field = FIELD_MASS; field < CREATOR_FIELDS; field++){
        if(!read_field(creator, field, &values[field])){
            return;
        }
    }
    if(values[FIELD_MASS] <= 0.0){
        creator->field = FIELD_MASS;
        set_message(creator, "mass must be positive", true);
        return;
    }
    if(values[FIELD_RADIUS] <= 0.0){
        creator->field = FIELD_RADIUS;
        set_message(creator, "radius must be positive", true);
        return;
    }

    const char* name = creator->values[FIELD_NAME];
    if(name[0] == '\0'){
        name = TextFormat("Body %u", simulation->count + 1);
    }

    Color color = {
        to_channel(values[FIELD_COLOR_R]),
        to_channel(values[FIELD_COLOR_G]),
        to_channel(values[FIELD_COLOR_B]),
        255
    };
    Body body = new_body(name, color,
        values[FIELD_MASS], values[FIELD_RADIUS],
        vec3(values[FIELD_POS_X], values[FIELD_POS_Y], values[FIELD_POS_Z]),
        vec3(values[FIELD_VEL_X], values[FIELD_VEL_Y], values[FIELD_VEL_Z]),
        simulation->scale);

    if(!add_simulation_body(simulation, body)){
        set_message(creator, "body limit reached", true);
        return;
    }

    flags->selected = (int)simulation->count - 1;
    set_message(creator, TextFormat("created %s", body.name), false);
    set_value(creator, FIELD_NAME, TextFormat("Body %u", simulation->count + 1));
    creator->field = FIELD_NAME;
}

static void move_field(Creator* creator, int step)
{
    creator->field = (creator->field + CREATOR_FIELDS + step) % CREATOR_FIELDS;
}

bool creator_update(Creator* creator, Simulation* simulation, DisplayFlags* flags)
{
    if(creator->message_time > 0.0){
        creator->message_time -= GetFrameTime();
    }
    if(!creator->active){
        return false;
    }

    ui_edit(creator->values[creator->field], CREATOR_TEXT_LEN, creator->field != FIELD_NAME, &creator->repeat);

    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    if(IsKeyPressed(KEY_TAB)){
        move_field(creator, shift ? -1 : 1);
    }
    if(IsKeyPressed(KEY_DOWN)){
        move_field(creator, 1);
    }
    if(IsKeyPressed(KEY_UP)){
        move_field(creator, -1);
    }
    if(IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)){
        creator_submit(creator, simulation, flags);
    }
    if(IsKeyPressed(KEY_ESCAPE)){
        creator_close(creator);
    }

    // Clicking a field selects it, clicking the panel never reaches the
    // camera or the body picker
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        for(int field = 0; field < CREATOR_FIELDS; field++){
            if(ui_mouse_in(field_area(field))){
                creator->field = field;
                break;
            }
        }
    }
    return true;
}

GhostBody creator_ghost(const Creator* creator, const Simulation* simulation)
{
    GhostBody ghost = {0};
    if(!creator->active){
        return ghost;
    }

    double position[3] = {0};
    for(int i = 0; i < 3; i++){
        if(!ui_parse(creator->values[FIELD_POS_X + i], &position[i])){
            return ghost;
        }
    }
    double radius = 0.0;
    if(!ui_parse(creator->values[FIELD_RADIUS], &radius)){
        return ghost;
    }
    double channels[3] = {0};
    for(int i = 0; i < 3; i++){
        ui_parse(creator->values[FIELD_COLOR_R + i], &channels[i]);
    }

    ghost.visible  = true;
    ghost.position = vec3_scale(vec3(position[0], position[1], position[2]), AU);
    ghost.radius   = simulation->scale > 0.0 ? radius / simulation->scale : radius;  // file units -> AU
    ghost.color    = (Color){ to_channel(channels[0]), to_channel(channels[1]), to_channel(channels[2]), 255 };
    return ghost;
}

// The buttons of the form are resolved here, the same way the toolbar does
void creator_draw(Creator* creator, Simulation* simulation, DisplayFlags* flags)
{
    if(!creator->active){
        // Keep the last result visible for a moment after closing the form
        if(creator->message_time > 0.0){
            DrawText(creator->message, (int)TEXT_X, TEXT_Y + 3 * TEXT_OFFSET, TEXT_SIZE - 4,
                creator->message_error ? ORANGE : GREEN);
        }
        return;
    }

    Rectangle area = creator_area();
    ui_panel(area, "NEW BODY");

    for(int field = 0; field < CREATOR_FIELDS; field++){
        ui_field(field_area(field), labels[field], creator->values[field], field == creator->field);
    }

    // Color preview
    float bottom = area.y + area.height;
    Rectangle swatch = { area.x + 12.0f, bottom - 78.0f, 24.0f, 24.0f };
    double channels[3] = {0};
    for(int i = 0; i < 3; i++){
        ui_parse(creator->values[FIELD_COLOR_R + i], &channels[i]);
    }
    DrawRectangleRec(swatch, (Color){ to_channel(channels[0]), to_channel(channels[1]), to_channel(channels[2]), 255 });
    DrawRectangleLinesEx(swatch, 1.0f, ui_color_line());

    float button = (area.width - 24.0f - 8.0f) * 0.5f;
    if(ui_button((Rectangle){ area.x + 12.0f, bottom - 44.0f, button, 30.0f }, "Close", false)){
        creator_close(creator);
    }
    if(ui_button((Rectangle){ area.x + 20.0f + button, bottom - 44.0f, button, 30.0f }, "Create", false)){
        creator_submit(creator, simulation, flags);
    }

    if(creator->message_time > 0.0){
        DrawText(creator->message, (int)area.x + 46, (int)bottom - 74, UI_SMALL,
            creator->message_error ? ORANGE : GREEN);
    }
}
