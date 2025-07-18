#include "gui.h"
#include "physics.h"

static void update_camera_pos(Camera2D* camera)
{
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f/camera->zoom);
        camera->target = Vector2Add(camera->target, delta);
    }
}

static void update_zoom(Camera2D* camera)
{
    float wheel = GetMouseWheelMove();
    if (wheel != 0){
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *camera);
        camera->offset = GetMousePosition();
        camera->target = mouseWorldPos;
        float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
        if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
        camera->zoom = Clamp(camera->zoom*scaleFactor, MIN_ZOOM, MAX_ZOOM);
    }
}

void update_camera(Camera2D* camera)
{
    update_camera_pos(camera);
    update_zoom(camera);
}

static float roundNearest10(float num)
{
    if (num <= 1) {
        return 1;
    }
    float exponent = floor(log10(num));
    return pow(10, exponent);
}

static void draw_grid(Camera2D camera)
{
    float step = 0.0;
    for(int i = 1; i <= GRID_LINES; i++){
        DrawLine(step, -GRID_LEN, step, GRID_LEN, GRAY);
        DrawLine(-GRID_LEN, step, GRID_LEN, step, GRAY);
        DrawLine(-step, -GRID_LEN, -step, GRID_LEN, GRAY);
        DrawLine(-GRID_LEN, -step, GRID_LEN, -step, GRAY);
        step += GRID_STEP * roundNearest10(1.0/camera.zoom);
    }
    // Draw asix lines
    DrawLine(0, -GRID_LEN, 0, GRID_LEN, RED);
    DrawLine(-GRID_LEN, 0, GRID_LEN, 0, BLUE);
}

static void draw_trayectories(Body body, DisplayFlags flags)
{
    if(flags.display_trayectory && body.trayectory.count > 1){
        for(unsigned j = 1; j < body.trayectory.count; j++){
            DrawLineV(body.trayectory.points[j - 1], body.trayectory.points[j], body.color);
        }
        DrawLineV(body.trayectory.points[body.trayectory.count - 1], body.position, body.color);
        DrawLineV(body.trayectory.points[body.trayectory.count - 1], body.position, body.color);
    }
}

static void draw_name(Camera2D camera, Body body, DisplayFlags flags)
{
    if(flags.names){
        DrawText(TextFormat( "%s", body.name), 
            body.position.x, body.position.y - (1/camera.zoom) * 15, (1/camera.zoom) * 20, WHITE);
    }
}

static void draw_bodies(Camera2D camera, Simulation simulation, DisplayFlags flags)
{
    for(unsigned i = 0; i < simulation.count; i++){
        Body body = simulation.bodies[i];
        DrawCircle(body.position.x, body.position.y, body.radius, body.color);
        draw_trayectories(body, flags);
        draw_name(camera, body, flags);
    }
}

static void draw_debug(Simulation simulation, DisplayFlags flags, double simtime)
{
    if(flags.debug){
        // Display debug info
        DrawText(TextFormat(
            "FPS: %d Bodies: %d Time Speed: %.0lf", 
            GetFPS(), 
            simulation.count, 
            flags.t_speed 
        ), TEXT_X, TEXT_Y, TEXT_SIZE, LIGHTGRAY);
        // Display time
        DrawText(TextFormat(
            "t: %03uy %03ud %02uh %02um %02us", 
             (unsigned)simtime / (365 * 24 * 60 * 60),  // seconds to years
            ((unsigned)simtime / (24 * 60 * 60)) % 365, // seconds to days      0...364
            ((unsigned)simtime / (60 * 60)) % 24,       // seconds to hours     0...23
            ((unsigned)simtime / 60) % 60,              // seconds to minutes   0...59
             (unsigned)simtime % 60                     // seconds              0...59
        ), TEXT_X, TEXT_Y + TEXT_OFFSET, TEXT_SIZE, LIGHTGRAY);
    }
}

void draw(Camera2D camera, Simulation simulation, DisplayFlags flags, double simtime)
{
    BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);
            draw_grid(camera);
            draw_bodies(camera, simulation, flags);
        EndMode2D();
        draw_debug(simulation, flags, simtime);
    EndDrawing();
}




















