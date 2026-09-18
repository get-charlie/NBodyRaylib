#include "gui.h"

#define MAX_SIM_SPEED   1000000000.0
#define MIN_BODY_SCALE  0.02f
#define MAX_BODY_SCALE  500.0f

// Simulation space is z up, render space is y up: swapping both axes puts
// the poles of the spheres raylib draws on the vertical axis of the scene.
static Vector3 swap_axes(Vec3 v)
{
    return (Vector3){ (float)v.x, (float)v.z, (float)v.y };
}

Vector3 to_render(const OrbitCamera* camera, Vec3 position)
{
    return swap_axes(vec3_scale(vec3_sub(position, camera->focus), RENDER_SCALE));
}

// Render offset -> simulation offset, the swap is its own inverse
static Vec3 to_simulation(Vector3 offset)
{
    return vec3(offset.x, offset.z, offset.y);
}

static Vector3 camera_offset(const OrbitCamera* camera)
{
    double cp = cos(camera->pitch);
    return (Vector3){
        (float)(camera->distance * cp * cos(camera->yaw)),
        (float)(camera->distance * sin(camera->pitch)),
        (float)(camera->distance * cp * sin(camera->yaw))
    };
}

// The focus sits on the render origin, so the raylib camera only has to
// describe the orbit around it.
static void sync_camera(OrbitCamera* camera)
{
    camera->camera.target     = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera->camera.position   = camera_offset(camera);
    camera->camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera->camera.fovy       = FOVY;
    camera->camera.projection = CAMERA_PERSPECTIVE;
}

void focus_point(OrbitCamera* camera, Vec3 point)
{
    camera->focus  = point;
    camera->follow = -1;
}

void follow_body(OrbitCamera* camera, const Simulation* simulation, int index)
{
    if(index < 0 || (unsigned)index >= simulation->count){
        camera->follow = -1;
        return;
    }
    camera->follow = index;
    camera->focus  = simulation->bodies[index].position;
}

void cycle_selection(const Simulation* simulation, OrbitCamera* camera, DisplayFlags* flags, int step)
{
    if(simulation->count == 0){
        return;
    }
    int count = (int)simulation->count;
    int next = flags->selected < 0 ? 0 : (flags->selected + count + step) % count;
    flags->selected = next;
    follow_body(camera, simulation, next);
}

// Places the camera so the whole system fits on screen
void frame_simulation(OrbitCamera* camera, const Simulation* simulation)
{
    camera->follow = -1;
    camera->focus  = simulation_center_of_mass(simulation);

    double extent = 0.0;
    for(unsigned i = 0; i < simulation->count; i++){
        double distance = vec3_distance(simulation->bodies[i].position, camera->focus);
        if(distance > extent){
            extent = distance;
        }
    }
    double units = extent * RENDER_SCALE;
    if(units <= 0.0){
        units = UNITS_PER_AU;
    }
    // Half the vertical field of view has to cover the radius of the system
    camera->distance = Clamp(units / tan(FOVY * 0.5f * DEG2RAD) * 1.2, MIN_DISTANCE, MAX_DISTANCE);
    sync_camera(camera);
}

void init_camera(OrbitCamera* camera, const Simulation* simulation)
{
    *camera = (OrbitCamera){0};
    camera->follow = -1;
    camera->yaw    = 0.0f;          // looking down the +x axis
    camera->pitch  = 0.6f;          // from above, so the plane is readable
    frame_simulation(camera, simulation);
}

// Drag with the right (or middle) button: moves the focus on the camera
// plane. The offset is built in render units and converted back to meters,
// the focus itself never leaves double precision.
static void pan_camera(OrbitCamera* camera, Vector2 delta)
{
    if(delta.x == 0.0f && delta.y == 0.0f){
        return;
    }
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera->camera.target, camera->camera.position));
    Vector3 right   = Vector3Normalize(Vector3CrossProduct(forward, camera->camera.up));
    Vector3 up      = Vector3CrossProduct(right, forward);

    // Render size of one pixel at the distance of the focus
    double pixel = 2.0 * camera->distance * tan(FOVY * 0.5f * DEG2RAD) / GetScreenHeight();
    Vector3 move = Vector3Add(Vector3Scale(right, (float)(-delta.x * pixel)),
                              Vector3Scale(up, (float)(delta.y * pixel)));

    camera->focus  = vec3_add(camera->focus, vec3_scale(to_simulation(move), 1.0 / RENDER_SCALE));
    camera->follow = -1;
}

static void zoom_camera(OrbitCamera* camera, double amount)
{
    camera->distance = Clamp(camera->distance * pow(ZOOM_STEP, -amount), MIN_DISTANCE, MAX_DISTANCE);
}

void update_camera(OrbitCamera* camera, const Simulation* simulation, bool mouse, bool keyboard)
{
    if(mouse){
        Vector2 delta = GetMouseDelta();
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            camera->drag = 0.0f;
        }
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            camera->drag += fabsf(delta.x) + fabsf(delta.y);
            camera->yaw   -= delta.x * ORBIT_SPEED;
            camera->pitch += delta.y * ORBIT_SPEED;
        }
        if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
            pan_camera(camera, delta);
        }
        zoom_camera(camera, GetMouseWheelMove());
    }

    // Keyboard fallback for the same controls, off while a form takes the
    // keyboard
    if(keyboard){
        float step = KEY_ORBIT_SPEED * GetFrameTime();
        if(IsKeyDown(KEY_A)) camera->yaw   += step;
        if(IsKeyDown(KEY_D)) camera->yaw   -= step;
        if(IsKeyDown(KEY_W)) camera->pitch += step;
        if(IsKeyDown(KEY_S)) camera->pitch -= step;
        if(IsKeyDown(KEY_E)) zoom_camera(camera,  2.0 * GetFrameTime());
        if(IsKeyDown(KEY_Q)) zoom_camera(camera, -2.0 * GetFrameTime());
    }

    camera->pitch = Clamp(camera->pitch, -MAX_PITCH, MAX_PITCH);

    if(camera->follow >= 0 && (unsigned)camera->follow < simulation->count){
        camera->focus = simulation->bodies[camera->follow].position;
    }
    sync_camera(camera);
}

void speed_up(DisplayFlags* flags)
{
    if(flags->t_speed <= 0.0){
        flags->t_speed = 1.0;
    }
    else if(flags->t_speed < MAX_SIM_SPEED){
        flags->t_speed *= 10.0;
    }
}

void speed_down(DisplayFlags* flags)
{
    if(flags->t_speed > 1.0){
        flags->t_speed /= 10.0;
    }
    else{
        flags->t_speed = 0.0;
    }
}

void scale_bodies(DisplayFlags* flags, float factor)
{
    flags->body_scale = Clamp(flags->body_scale * factor, MIN_BODY_SCALE, MAX_BODY_SCALE);
}

// Radius of a body in pixels, used both to draw and to pick it
static float screen_radius(const OrbitCamera* camera, Vector3 position, float radius)
{
    float distance = Vector3Distance(position, camera->camera.position);
    if(distance <= 0.0f){
        return (float)GetScreenHeight();
    }
    return radius * (GetScreenHeight() * 0.5f) / (tanf(FOVY * 0.5f * DEG2RAD) * distance);
}

static float render_radius(const DisplayFlags* flags, const Body* body)
{
    return (float)(body->radius * UNITS_PER_AU) * flags->body_scale;
}

static bool in_front(const OrbitCamera* camera, Vector3 position)
{
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera->camera.target, camera->camera.position));
    return Vector3DotProduct(Vector3Subtract(position, camera->camera.position), forward) > 0.0f;
}

int pick_body(const OrbitCamera* camera, const Simulation* simulation, Vector2 point)
{
    int picked = -1;
    float best = 0.0f;
    for(unsigned i = 0; i < simulation->count; i++){
        Vector3 position = to_render(camera, simulation->bodies[i].position);
        if(!in_front(camera, position)){
            continue;
        }
        Vector2 screen = GetWorldToScreen(position, camera->camera);
        float distance = Vector2Distance(screen, point);
        float radius = screen_radius(camera, position, (float)(simulation->bodies[i].radius * UNITS_PER_AU));
        if(distance <= fmaxf(radius, PICK_RADIUS) && (picked < 0 || distance < best)){
            picked = (int)i;
            best = distance;
        }
    }
    return picked;
}

// raylib clips at a fixed 0.01..1000 range, which cannot hold a solar
// system, so the projection is built here with clip planes that follow the
// camera distance.
static void begin_scene(const OrbitCamera* camera, double extent)
{
    double far_plane  = (camera->distance + extent) * 4.0 + 1.0;
    double near_plane = fmax(camera->distance * 1.0e-3, far_plane * 1.0e-6);

    rlDrawRenderBatchActive();
    rlMatrixMode(RL_PROJECTION);
    rlPushMatrix();
    rlLoadIdentity();

    double top   = near_plane * tan(FOVY * 0.5f * DEG2RAD);
    double right = top * ((double)GetScreenWidth() / (double)GetScreenHeight());
    rlFrustum(-right, right, -top, top, near_plane, far_plane);

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    Matrix view = MatrixLookAt(camera->camera.position, camera->camera.target, camera->camera.up);
    rlMultMatrixf(MatrixToFloat(view));
    rlEnableDepthTest();
}

static void end_scene(void)
{
    rlDrawRenderBatchActive();
    rlMatrixMode(RL_PROJECTION);
    rlPopMatrix();
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    rlDisableDepthTest();
}

// Height of the z = 0 plane in render space
static float ground_level(const OrbitCamera* camera)
{
    return (float)(-camera->focus.z * RENDER_SCALE);
}

// Grid on the horizontal plane, snapped to world coordinates so it does not
// slide when the camera moves, with a spacing that follows the zoom level
static void draw_grid(const OrbitCamera* camera)
{
    double step = pow(10.0, floor(log10(camera->distance / 4.0)));
    if(!(step > 0.0)){
        return;
    }
    double length = step * GRID_LINES;
    Vec3 focus = vec3_scale(camera->focus, RENDER_SCALE);
    double ox = -fmod(focus.x, step);
    double oz = -fmod(focus.y, step);      // the simulation y is the render z
    float y = ground_level(camera);
    Color line = { 40, 40, 48, 255 };

    for(int i = -GRID_LINES; i <= GRID_LINES; i++){
        float x = (float)(ox + i * step);
        float z = (float)(oz + i * step);
        DrawLine3D((Vector3){ x, y, (float)(oz - length) }, (Vector3){ x, y, (float)(oz + length) }, line);
        DrawLine3D((Vector3){ (float)(ox - length), y, z }, (Vector3){ (float)(ox + length), y, z }, line);
    }

    // Axes of the system, drawn only while the origin is close enough for
    // the float coordinates to stay accurate
    if(fabs(focus.x) < length && fabs(focus.y) < length && fabs(focus.z) < length){
        Vector3 origin = { (float)-focus.x, (float)-focus.z, (float)-focus.y };
        float l = (float)length;
        DrawLine3D((Vector3){ origin.x - l, origin.y, origin.z }, (Vector3){ origin.x + l, origin.y, origin.z }, (Color){ 180, 60, 60, 255 });
        DrawLine3D((Vector3){ origin.x, origin.y, origin.z - l }, (Vector3){ origin.x, origin.y, origin.z + l }, (Color){ 60, 160, 60, 255 });
        DrawLine3D((Vector3){ origin.x, origin.y - l, origin.z }, (Vector3){ origin.x, origin.y + l, origin.z }, (Color){ 60, 100, 200, 255 });
    }
}

static void draw_dashed(Vector3 from, Vector3 to, Color color)
{
    for(int i = 0; i < DASHES; i++){
        float t0 = (float)i / DASHES;
        float t1 = t0 + 0.5f / DASHES;
        DrawLine3D(Vector3Lerp(from, to, t0), Vector3Lerp(from, to, t1), color);
    }
}

// Vertical line from a body down to the horizontal plane, so its height can
// be read at a glance. Bodies under the plane get a dashed line.
static void draw_height_line(const OrbitCamera* camera, const Body* body)
{
    Vector3 position = to_render(camera, body->position);
    Vector3 base = { position.x, ground_level(camera), position.z };
    Color color = Fade(body->color, 0.6f);

    if(body->position.z >= 0.0){
        DrawLine3D(position, base, color);
    }
    else{
        draw_dashed(position, base, color);
    }

    // Small cross marking where the body stands on the plane
    float tick = (float)(camera->distance * 0.008);
    DrawLine3D((Vector3){ base.x - tick, base.y, base.z }, (Vector3){ base.x + tick, base.y, base.z }, color);
    DrawLine3D((Vector3){ base.x, base.y, base.z - tick }, (Vector3){ base.x, base.y, base.z + tick }, color);
}

static void draw_trayectory(const OrbitCamera* camera, const Simulation* simulation, unsigned index)
{
    const Trayectory* trayectory = &simulation->trayectories[index];
    const Body* body = &simulation->bodies[index];
    if(trayectory->count < 2){
        return;
    }
    Vector3 previous = to_render(camera, trayectory->points[0]);
    for(unsigned i = 1; i < trayectory->count; i++){
        Vector3 point = to_render(camera, trayectory->points[i]);
        // Older points fade out
        float age = (float)i / (float)trayectory->count;
        DrawLine3D(previous, point, Fade(body->color, TRAIL_FADE + (1.0f - TRAIL_FADE) * age));
        previous = point;
    }
    DrawLine3D(previous, to_render(camera, body->position), body->color);
}

// A body heavy enough to be a star lights the others instead of being lit
static bool is_star(const Body* body)
{
    return body->mass >= STAR_MASS;
}

// Direction from a body to the star that lights it the most, in render
// space. Returns false when there is no star in the simulation.
static bool light_direction(const Simulation* simulation, unsigned index, Vector3* out)
{
    const Body* body = &simulation->bodies[index];
    double best = 0.0;
    Vec3 direction = vec3(0.0, 0.0, 0.0);

    for(unsigned i = 0; i < simulation->count; i++){
        if(i == index || !is_star(&simulation->bodies[i])){
            continue;
        }
        Vec3 delta = vec3_sub(simulation->bodies[i].position, body->position);
        double distance2 = vec3_dot(delta, delta);
        if(distance2 <= 0.0){
            continue;
        }
        double brightness = simulation->bodies[i].mass / distance2;
        if(brightness > best){
            best = brightness;
            direction = vec3_scale(delta, 1.0 / sqrt(distance2));
        }
    }
    if(best == 0.0){
        return false;
    }
    *out = swap_axes(direction);
    return true;
}

// Sphere with the poles on the vertical axis. Every vertex is colored by
// the side it is on: the half facing the light keeps the color of the body
// and the other half stays black.
static void draw_sphere_lit(Vector3 center, float radius, int rings, int slices, Color color, Vector3 light)
{
    rlCheckRenderBatchLimit(rings * slices * 6);
    rlBegin(RL_TRIANGLES);
    for(int ring = 0; ring < rings; ring++){
        float phi0 = PI * (float)ring / (float)rings;
        float phi1 = PI * (float)(ring + 1) / (float)rings;
        for(int slice = 0; slice < slices; slice++){
            float theta0 = 2.0f * PI * (float)slice / (float)slices;
            float theta1 = 2.0f * PI * (float)(slice + 1) / (float)slices;

            Vector3 normals[4] = {
                { sinf(phi0) * cosf(theta0), cosf(phi0), sinf(phi0) * sinf(theta0) },
                { sinf(phi0) * cosf(theta1), cosf(phi0), sinf(phi0) * sinf(theta1) },
                { sinf(phi1) * cosf(theta1), cosf(phi1), sinf(phi1) * sinf(theta1) },
                { sinf(phi1) * cosf(theta0), cosf(phi1), sinf(phi1) * sinf(theta0) }
            };
            // Two triangles, wound counterclockwise seen from outside
            static const int order[6] = { 0, 2, 3, 0, 1, 2 };
            for(int i = 0; i < 6; i++){
                Vector3 normal = normals[order[i]];
                Color vertex = Vector3DotProduct(normal, light) > 0.0f ? color : (Color){ 0, 0, 0, 255 };
                rlColor4ub(vertex.r, vertex.g, vertex.b, vertex.a);
                rlVertex3f(center.x + normal.x * radius,
                           center.y + normal.y * radius,
                           center.z + normal.z * radius);
            }
        }
    }
    rlEnd();
}

// Fewer triangles for the bodies that only take a few pixels
static void sphere_detail(float pixels, int* rings, int* slices)
{
    if(pixels < 6.0f){
        *rings = 4;  *slices = 6;
    }
    else if(pixels < 24.0f){
        *rings = 8;  *slices = 12;
    }
    else{
        *rings = 16; *slices = 24;
    }
}

static void draw_bodies(const OrbitCamera* camera, const Simulation* simulation, const DisplayFlags* flags)
{
    bool any_star = false;
    if(flags->lighting){
        for(unsigned i = 0; i < simulation->count && !any_star; i++){
            any_star = is_star(&simulation->bodies[i]);
        }
    }

    for(unsigned i = 0; i < simulation->count; i++){
        const Body* body = &simulation->bodies[i];
        Vector3 position = to_render(camera, body->position);
        float radius = render_radius(flags, body);

        int rings = 0;
        int slices = 0;
        sphere_detail(screen_radius(camera, position, radius), &rings, &slices);

        Vector3 light = {0};
        bool lit = any_star && !is_star(body) && light_direction(simulation, i, &light);
        if(lit){
            draw_sphere_lit(position, radius, rings, slices, body->color, light);
        }
        else{
            DrawSphereEx(position, radius, rings, slices, body->color);
            // The default shader has no lighting, the wireframe gives the
            // spheres some volume
            DrawSphereWires(position, radius * 1.01f, rings / 2 + 2, slices / 2 + 2, Fade(BLACK, 0.35f));
        }

        if((int)i == flags->selected){
            DrawSphereWires(position, radius * 1.35f, 8, 12, Fade(RAYWHITE, 0.5f));
        }
        if(flags->height_lines){
            draw_height_line(camera, body);
        }
        if(flags->display_trayectory){
            draw_trayectory(camera, simulation, i);
        }
    }
}

void begin_frame(void)
{
    BeginDrawing();
    ClearBackground((Color){ 8, 8, 12, 255 });
}

void end_frame(void)
{
    EndDrawing();
}

void draw_scene(const OrbitCamera* camera, const Simulation* simulation, const DisplayFlags* flags, const GhostBody* ghost)
{
    double extent = 0.0;
    for(unsigned i = 0; i < simulation->count; i++){
        double distance = vec3_distance(simulation->bodies[i].position, camera->focus) * RENDER_SCALE;
        if(distance > extent){
            extent = distance;
        }
    }

    begin_scene(camera, extent);
        if(flags->grid){
            draw_grid(camera);
        }
        draw_bodies(camera, simulation, flags);
        if(ghost != NULL && ghost->visible){
            Vector3 position = to_render(camera, ghost->position);
            float radius = (float)(ghost->radius * UNITS_PER_AU) * flags->body_scale;
            DrawSphereWires(position, radius, 8, 12, Fade(ghost->color, 0.8f));
        }
    end_scene();
}

static bool label_fits(const Rectangle* taken, int count, Rectangle label)
{
    for(int i = 0; i < count; i++){
        if(CheckCollisionRecs(taken[i], label)){
            return false;
        }
    }
    return true;
}

// Names, and the markers that keep distant bodies visible, are drawn in 2D
// on top of the scene
static void draw_markers(const OrbitCamera* camera, const Simulation* simulation, const DisplayFlags* flags)
{
    // Names of bodies that end up on top of each other are dropped, the
    // selected one always keeps its label
    Rectangle taken[MAX_LABELS];
    int taken_count = 0;

    for(unsigned i = 0; i < simulation->count; i++){
        const Body* body = &simulation->bodies[i];
        Vector3 position = to_render(camera, body->position);
        if(!in_front(camera, position)){
            continue;
        }
        Vector2 screen = GetWorldToScreen(position, camera->camera);
        float radius = screen_radius(camera, position, render_radius(flags, body));

        if(radius < MIN_BODY_PIXELS){
            DrawCircleV(screen, MIN_BODY_PIXELS, body->color);
            radius = MIN_BODY_PIXELS;
        }
        if(!flags->names){
            continue;
        }

        int width = MeasureText(body->name, TEXT_SIZE - 4);
        Rectangle label = {
            screen.x - width / 2.0f,
            screen.y - radius - TEXT_SIZE,
            width + LABEL_SPACING,
            TEXT_SIZE
        };
        bool selected = (int)i == flags->selected;
        if(!selected && !label_fits(taken, taken_count, label)){
            continue;
        }
        if(taken_count < MAX_LABELS){
            taken[taken_count++] = label;
        }
        DrawText(body->name, (int)label.x, (int)label.y, TEXT_SIZE - 4,
                 selected ? RAYWHITE : (Color){ 200, 200, 210, 255 });
    }
}

static const char* format_time(double seconds)
{
    unsigned long long total = (unsigned long long)fabs(seconds);
    return TextFormat("%s%lluy %03llud %02lluh %02llum %02llus",
        seconds < 0 ? "-" : "",
        total / (365ULL * 24 * 60 * 60),
       (total / (24ULL * 60 * 60)) % 365,
       (total / (60ULL * 60)) % 24,
       (total / 60ULL) % 60,
        total % 60ULL);
}

static void draw_help(void)
{
    static const char* lines[] = {
        "Mouse left drag    Orbit camera",
        "Mouse right drag   Pan camera",
        "Mouse wheel        Zoom",
        "Mouse left click   Select body",
        "W A S D / Q E      Orbit / zoom",
        "F                  Follow selected body",
        "TAB / SHIFT+TAB    Select next / previous body",
        "R                  Frame the whole system",
        "SPACE              Pause",
        "RIGHT / LEFT       Simulation speed",
        "C                  Body creator",
        "O                  Save simulation",
        "T / K              Trayectories / height lines",
        "N / G / L          Names / grid / lighting",
        "I                  Debug info",
        "B / V              Bigger / smaller bodies",
        "M                  Back to the main menu",
        "H                  Toggle this help",
        "F11                Fullscreen",
        "ESC                Quit",
    };
    int count = sizeof(lines) / sizeof(lines[0]);
    float height = count * (TEXT_SIZE - 2) + 50.0f;
    Rectangle area = { TEXT_X, GetScreenHeight() - height - 20.0f, 420.0f, height };
    ui_panel(area, "CONTROLS");
    for(int i = 0; i < count; i++){
        DrawText(lines[i], (int)area.x + 10, (int)(area.y + 36 + i * (TEXT_SIZE - 2)), TEXT_SIZE - 6, ui_color_dim());
    }
}

static void draw_selection(const OrbitCamera* camera, const Simulation* simulation, const DisplayFlags* flags)
{
    if(flags->selected < 0 || (unsigned)flags->selected >= simulation->count){
        return;
    }
    const Body* body = &simulation->bodies[flags->selected];
    Rectangle area = { GetScreenWidth() - 340.0f, 20.0f, 320.0f, 170.0f };
    ui_panel(area, body->name);

    int x = (int)area.x + 10;
    int y = (int)area.y + 38;
    Color text = ui_color_dim();
    DrawText(TextFormat("mass     %.4g kg%s", body->mass, is_star(body) ? "  (star)" : ""), x, y, TEXT_SIZE - 4, text);
    DrawText(TextFormat("speed    %.4g km/s", vec3_length(body->velocity) / KM), x, y + TEXT_OFFSET, TEXT_SIZE - 4, text);
    DrawText(TextFormat("height   %.5g AU", body->position.z / AU), x, y + 2 * TEXT_OFFSET, TEXT_SIZE - 4, text);
    DrawText(TextFormat("pos %.4g, %.4g, %.4g AU",
        body->position.x / AU, body->position.y / AU, body->position.z / AU), x, y + 3 * TEXT_OFFSET, TEXT_SIZE - 6, text);
    DrawText(camera->follow == flags->selected ? "camera following" : "F to follow",
        x, y + 4 * TEXT_OFFSET, TEXT_SIZE - 4, camera->follow == flags->selected ? GREEN : (Color){ 140, 140, 150, 255 });
}

void draw_hud(const OrbitCamera* camera, const Simulation* simulation, const DisplayFlags* flags, const SimStats* stats)
{
    draw_markers(camera, simulation, flags);

    if(flags->debug){
        DrawText(TextFormat("FPS: %d  Bodies: %u  Threads: %u  Speed: x%.0lf%s",
            GetFPS(), simulation->count, stats->threads, flags->t_speed,
            flags->paused ? "  [PAUSED]" : ""), (int)TEXT_X, TEXT_Y, TEXT_SIZE, LIGHTGRAY);
        DrawText(TextFormat("t: %s", format_time(simulation->time)),
            (int)TEXT_X, TEXT_Y + TEXT_OFFSET, TEXT_SIZE, LIGHTGRAY);

        double drift = 0.0;
        if(stats->energy0 != 0.0){
            drift = fabs((stats->energy - stats->energy0) / stats->energy0) * 100.0;
        }
        DrawText(TextFormat("energy drift: %.6f %%   view: %.4g AU",
            drift, camera->distance / UNITS_PER_AU), (int)TEXT_X, TEXT_Y + 2 * TEXT_OFFSET, TEXT_SIZE, LIGHTGRAY);
    }

    draw_selection(camera, simulation, flags);

    if(flags->help){
        draw_help();
    }
}

Rectangle toolbar_area(void)
{
    return (Rectangle){ 0.0f, 0.0f, TOOLBAR_WIDTH, (float)GetScreenHeight() };
}

// Every key of the simulation has a button here. The toolbar owns the
// display flags and the camera, and hands back the actions that need the
// application to react.
UiAction draw_toolbar(OrbitCamera* camera, const Simulation* simulation, DisplayFlags* flags)
{
    Rectangle area = toolbar_area();
    UiAction action = ACTION_NONE;

    DrawRectangleRec(area, (Color){ 12, 12, 18, 245 });
    DrawLine((int)area.width, 0, (int)area.width, GetScreenHeight(), ui_color_line());
    DrawText("BodySim", 12, 12, UI_TITLE, ui_color_text());

    float x = 8.0f;
    float y = 44.0f;
    float full = area.width - 16.0f;
    float half = (full - 4.0f) * 0.5f;
    float row = 24.0f;

    Rectangle wide  = { x, y, full, row };
    Rectangle left  = { x, y, half, row };
    Rectangle right = { x + half + 4.0f, y, half, row };

    #define UI_PLACE  wide.y = left.y = right.y = y
    #define UI_NEXT   y += row + 4.0f; UI_PLACE
    #define UI_GROUP(title) ui_separator((Rectangle){ x, y, full, 14.0f }, title); y += 18.0f; UI_PLACE

    UI_GROUP("TIME");
    if(ui_button(wide, flags->paused ? "Play" : "Pause", flags->paused)){
        flags->paused = !flags->paused;
    }
    UI_NEXT;
    if(ui_button(left, "Slower", false)){
        speed_down(flags);
    }
    if(ui_button(right, "Faster", false)){
        speed_up(flags);
    }
    y += row + 4.0f;
    DrawText(TextFormat("speed x%.0lf", flags->t_speed), (int)x + 2, (int)y, UI_SMALL, ui_color_dim());
    y += 18.0f;
    UI_PLACE;

    UI_GROUP("VIEW");
    if(ui_button(left, "Trails", flags->display_trayectory)){
        flags->display_trayectory = !flags->display_trayectory;
    }
    if(ui_button(right, "Heights", flags->height_lines)){
        flags->height_lines = !flags->height_lines;
    }
    UI_NEXT;
    if(ui_button(left, "Names", flags->names)){
        flags->names = !flags->names;
    }
    if(ui_button(right, "Grid", flags->grid)){
        flags->grid = !flags->grid;
    }
    UI_NEXT;
    if(ui_button(left, "Light", flags->lighting)){
        flags->lighting = !flags->lighting;
    }
    if(ui_button(right, "Info", flags->debug)){
        flags->debug = !flags->debug;
    }
    UI_NEXT;
    if(ui_button(left, "Size -", false)){
        scale_bodies(flags, 1.0f / 1.25f);
    }
    if(ui_button(right, "Size +", false)){
        scale_bodies(flags, 1.25f);
    }
    y += row + 4.0f;
    UI_PLACE;

    UI_GROUP("CAMERA");
    if(ui_button(left, "Frame", false)){
        frame_simulation(camera, simulation);
    }
    if(ui_button_ex(right, "Follow", camera->follow >= 0 && camera->follow == flags->selected, flags->selected >= 0)){
        if(camera->follow == flags->selected){
            camera->follow = -1;
        }
        else{
            follow_body(camera, simulation, flags->selected);
        }
    }
    UI_NEXT;
    if(ui_button_ex(left, "Prev", false, simulation->count > 0)){
        cycle_selection(simulation, camera, flags, -1);
    }
    if(ui_button_ex(right, "Next", false, simulation->count > 0)){
        cycle_selection(simulation, camera, flags, 1);
    }
    y += row + 4.0f;
    UI_PLACE;

    UI_GROUP("SIMULATION");
    if(ui_button(wide, "New body", false)){
        action = ACTION_CREATOR;
    }
    UI_NEXT;
    if(ui_button(wide, "Save as...", false)){
        action = ACTION_SAVE;
    }
    UI_NEXT;
    if(ui_button(wide, "Main menu", false)){
        action = ACTION_MENU;
    }
    y += row + 4.0f;
    UI_PLACE;

    UI_GROUP("WINDOW");
    if(ui_button(left, "Help", flags->help)){
        flags->help = !flags->help;
    }
    if(ui_button(right, "Full", IsWindowFullscreen())){
        ToggleFullscreen();
    }
    UI_NEXT;
    if(ui_button(wide, "Quit", false)){
        action = ACTION_QUIT;
    }

    #undef UI_PLACE
    #undef UI_NEXT
    #undef UI_GROUP

    return action;
}
