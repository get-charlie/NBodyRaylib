#include "gui.h"

Vector3 to_render(const OrbitCamera* camera, Vec3 position)
{
    Vec3 offset = vec3_scale(vec3_sub(position, camera->focus), RENDER_SCALE);
    return (Vector3){ (float)offset.x, (float)offset.y, (float)offset.z };
}

static Vector3 camera_offset(const OrbitCamera* camera)
{
    double cp = cos(camera->pitch);
    return (Vector3){
        (float)(camera->distance * cp * cos(camera->yaw)),
        (float)(camera->distance * cp * sin(camera->yaw)),
        (float)(camera->distance * sin(camera->pitch))
    };
}

// The focus sits on the render origin, so the raylib camera only has to
// describe the orbit around it.
static void sync_camera(OrbitCamera* camera)
{
    camera->camera.target     = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera->camera.position   = camera_offset(camera);
    camera->camera.up         = (Vector3){ 0.0f, 0.0f, 1.0f };   // z is up
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
    Vec3 move = vec3_add(
        vec3_scale(vec3(right.x, right.y, right.z), -delta.x * pixel),
        vec3_scale(vec3(up.x, up.y, up.z), delta.y * pixel)
    );
    camera->focus  = vec3_add(camera->focus, vec3_scale(move, 1.0 / RENDER_SCALE));
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

    // Keyboard fallback for the same controls, off while the creator takes
    // the keyboard
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

// Grid on the z = 0 plane, snapped to world coordinates so it does not
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
    double oy = -fmod(focus.y, step);
    float z = (float)(-focus.z);
    Color line = { 40, 40, 48, 255 };

    for(int i = -GRID_LINES; i <= GRID_LINES; i++){
        float x = (float)(ox + i * step);
        float y = (float)(oy + i * step);
        DrawLine3D((Vector3){ x, (float)(oy - length), z }, (Vector3){ x, (float)(oy + length), z }, line);
        DrawLine3D((Vector3){ (float)(ox - length), y, z }, (Vector3){ (float)(ox + length), y, z }, line);
    }

    // Axes of the system, drawn only while the origin is close enough for
    // the float coordinates to stay accurate
    if(fabs(focus.x) < length && fabs(focus.y) < length && fabs(focus.z) < length){
        Vector3 origin = { (float)-focus.x, (float)-focus.y, (float)-focus.z };
        float l = (float)length;
        DrawLine3D((Vector3){ origin.x - l, origin.y, origin.z }, (Vector3){ origin.x + l, origin.y, origin.z }, (Color){ 180, 60, 60, 255 });
        DrawLine3D((Vector3){ origin.x, origin.y - l, origin.z }, (Vector3){ origin.x, origin.y + l, origin.z }, (Color){ 60, 160, 60, 255 });
        DrawLine3D((Vector3){ origin.x, origin.y, origin.z - l }, (Vector3){ origin.x, origin.y, origin.z + l }, (Color){ 60, 100, 200, 255 });
    }
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

static void draw_bodies(const OrbitCamera* camera, const Simulation* simulation, const DisplayFlags* flags)
{
    for(unsigned i = 0; i < simulation->count; i++){
        const Body* body = &simulation->bodies[i];
        Vector3 position = to_render(camera, body->position);
        float radius = render_radius(flags, body);

        DrawSphereEx(position, radius, 12, 16, body->color);
        // The default shader has no lighting, the wireframe gives the
        // spheres some volume
        DrawSphereWires(position, radius * 1.01f, 8, 10, Fade(BLACK, 0.35f));

        if((int)i == flags->selected){
            DrawSphereWires(position, radius * 1.35f, 8, 12, Fade(RAYWHITE, 0.5f));
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

// Names, and the markers that keep distant bodies visible, are drawn in 2D
// on top of the scene
static bool label_fits(const Rectangle* taken, int count, Rectangle label)
{
    for(int i = 0; i < count; i++){
        if(CheckCollisionRecs(taken[i], label)){
            return false;
        }
    }
    return true;
}

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

void draw_panel(Rectangle area, const char* title)
{
    DrawRectangleRec(area, Fade((Color){ 15, 15, 22, 255 }, PANEL_ALPHA));
    DrawRectangleLinesEx(area, 1.0f, (Color){ 70, 70, 90, 255 });
    if(title != NULL){
        DrawText(title, (int)area.x + 10, (int)area.y + 8, TEXT_SIZE, RAYWHITE);
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
        "T / N / G          Trayectories / names / grid",
        "I                  Debug info",
        "B / V              Bigger / smaller bodies",
        "H                  Toggle this help",
        "F11                Fullscreen",
        "ESC                Quit",
    };
    int count = sizeof(lines) / sizeof(lines[0]);
    float height = count * (TEXT_SIZE - 2) + 50.0f;
    Rectangle area = { 20.0f, GetScreenHeight() - height - 20.0f, 420.0f, height };
    draw_panel(area, "CONTROLS");
    for(int i = 0; i < count; i++){
        DrawText(lines[i], (int)area.x + 10, (int)(area.y + 36 + i * (TEXT_SIZE - 2)), TEXT_SIZE - 6, (Color){ 190, 190, 200, 255 });
    }
}

static void draw_selection(const OrbitCamera* camera, const Simulation* simulation, const DisplayFlags* flags)
{
    if(flags->selected < 0 || (unsigned)flags->selected >= simulation->count){
        return;
    }
    const Body* body = &simulation->bodies[flags->selected];
    Rectangle area = { GetScreenWidth() - 340.0f, 20.0f, 320.0f, 170.0f };
    draw_panel(area, body->name);

    int x = (int)area.x + 10;
    int y = (int)area.y + 38;
    Color text = { 190, 190, 200, 255 };
    DrawText(TextFormat("mass     %.4g kg", body->mass), x, y, TEXT_SIZE - 4, text);
    DrawText(TextFormat("speed    %.4g km/s", vec3_length(body->velocity) / KM), x, y + TEXT_OFFSET, TEXT_SIZE - 4, text);
    DrawText(TextFormat("distance %.5g AU", vec3_length(body->position) / AU), x, y + 2 * TEXT_OFFSET, TEXT_SIZE - 4, text);
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
            flags->paused ? "  [PAUSED]" : ""), TEXT_X, TEXT_Y, TEXT_SIZE, LIGHTGRAY);
        DrawText(TextFormat("t: %s", format_time(simulation->time)),
            TEXT_X, TEXT_Y + TEXT_OFFSET, TEXT_SIZE, LIGHTGRAY);

        double drift = 0.0;
        if(stats->energy0 != 0.0){
            drift = fabs((stats->energy - stats->energy0) / stats->energy0) * 100.0;
        }
        DrawText(TextFormat("energy drift: %.6f %%   view: %.4g AU",
            drift, camera->distance / UNITS_PER_AU), TEXT_X, TEXT_Y + 2 * TEXT_OFFSET, TEXT_SIZE, LIGHTGRAY);
    }

    draw_selection(camera, simulation, flags);

    if(flags->help){
        draw_help();
    }
    else{
        DrawText("H: controls", TEXT_X, GetScreenHeight() - TEXT_OFFSET - 10, TEXT_SIZE - 4, (Color){ 140, 140, 150, 255 });
    }
}
