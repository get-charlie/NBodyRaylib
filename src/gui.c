#include "gui.h"
#include "physics.h"

void update_camera_pos(Camera2D* camera)
{
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f/camera->zoom);
        camera->target = Vector2Add(camera->target, delta);
    }
}

void update_zoom(Camera2D* camera)
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

static float roundNearest10(float num)
{
    if (num <= 1) {
        return 1;
    }
    float exponent = floor(log10(num));
    return pow(10, exponent);
}

#define MARKER_SCALE 20
static void draw_grid(Camera2D camera, Simulation simulation)
{
    float step = 0.0;
    for(int i = 1; i <= GRID_LINES; i++){
        DrawLine(step, -GRID_LEN, step, GRID_LEN, GRAY);
        DrawLine(-GRID_LEN, step, GRID_LEN, step, GRAY);
        DrawLine(-step, -GRID_LEN, -step, GRID_LEN, GRAY);
        DrawLine(-GRID_LEN, -step, GRID_LEN, -step, GRAY);
        step += GRID_STEP * roundNearest10(1.0/camera.zoom);
    }
    step = 0.0;
    for(int i = 1; i <= 100; i++){
        float zm = roundNearest10(1.0/(camera.zoom*50));
        step += simulation.scale * zm;
        DrawText(TextFormat(
            "%.0f AU", i * zm
        ), step, 0, (1/camera.zoom) * MARKER_SCALE, WHITE);
        DrawText(TextFormat(
            "%.0f AU", i * zm
        ), 0, step, (1/camera.zoom) * MARKER_SCALE, WHITE);
        DrawText(TextFormat(
            "%.0f AU", i * zm
        ), -step, 0, (1/camera.zoom) * MARKER_SCALE, WHITE);
        DrawText(TextFormat(
            "%.0f AU", i * zm
        ), 0, -step, (1/camera.zoom) * MARKER_SCALE, WHITE);
    }
    // Draw asix lines
    DrawLine(0, -GRID_LEN, 0, GRID_LEN, RED);
    DrawLine(-GRID_LEN, 0, GRID_LEN, 0, BLUE);
}

static void draw_trayectories(Body body, DisplayFlags flags)
{
    if(flags.displayTrayectory && body.trayectory.count > 1){
        for(unsigned j = 1; j < body.trayectory.count; j++){
            DrawLineV(body.trayectory.points[j - 1], body.trayectory.points[j], body.color);
        }
        DrawLineV(body.trayectory.points[body.trayectory.count - 1], body.position, body.color);
        DrawLineV(body.trayectory.points[body.trayectory.count - 1], body.position, body.color);
    }
}

static void draw_name_mass(Camera2D camera, Body body, DisplayFlags flags)
{
    if(flags.names){
        DrawText(TextFormat(
            "%s m=%.3lf EM", 
            body.name, 
            body.mass / EARTH_MASS
        ), body.position.x, body.position.y - (1/camera.zoom) * 15, (1/camera.zoom) * 20, WHITE);
    }
}

static void draw_bodies(Camera2D camera, Simulation simulation, DisplayFlags flags)
{
    for(unsigned i = 0; i < simulation.count; i++){
        Body body = simulation.bodies[i];
        DrawCircle(body.position.x, body.position.y, body.radius, body.color);
        draw_trayectories(body, flags);
        draw_name_mass(camera, body, flags);
    }
}

static void draw_debug(Camera2D camera, Simulation simulation, DisplayFlags flags, double simtime)
{
    if(flags.debug){
        // Display debug info
        DrawText(TextFormat(
            "FPS: %d Bodies: %d tSpeed: %.0lf x: %f AU y: %f AU z: %f", 
            GetFPS(), 
            simulation.count, 
            flags.tSpeed, 
            camera.target.x / simulation.scale, 
            camera.target.y / simulation.scale, 
            camera.zoom
        ), TEXT_X, TEXT_Y, TEXT_SIZE, LIGHTGRAY);
        // Display time
        DrawText(TextFormat(
            "t: %03uy %03ud %02uh %02um %02us", 
             (unsigned)simtime / (365 * 24 * 60 * 60),  // seconds to years
            ((unsigned)simtime / (24 * 60 * 60)) % 365, // seconds to days      0...364
            ((unsigned)simtime / (60 * 60)) % 24,       // seconds to hours     0...23
            ((unsigned)simtime / 60) % 60,              // seconds to minutes   0...59
             (unsigned)simtime % 60                     // secods               0...59
        ), TEXT_X, TEXT_Y + TEXT_OFFSET, TEXT_SIZE, LIGHTGRAY);
    }
}

void draw(Camera2D camera, Simulation simulation, DisplayFlags flags, double simtime)
{
    BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);
            draw_grid(camera, simulation);
            draw_bodies(camera, simulation, flags);
        EndMode2D();
        draw_debug(camera, simulation, flags, simtime);
    EndDrawing();
}




















