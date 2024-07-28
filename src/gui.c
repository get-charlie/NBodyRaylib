#include "gui.h"

void update_camera_pos(Camera2D *camera){
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f/camera->zoom);
        camera->target = Vector2Add(camera->target, delta);
    }
}

void update_zoom(Camera2D *camera){
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

static float roundNearest10(float num) {
    if (num <= 1) {
        return 1;
    }
    float exponent = floor(log10(num));
    return pow(10, exponent);
}

void draw(Camera2D camera, Simulation simulation, DisplayFlags flags){
    BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);
            float step = 0.0;
            float mul = (1.0 / camera.zoom);
            mul = roundNearest10(mul);
            // Draw grid
            for(int i = 0; i < GRID_LINES; i++){
                DrawLine(step, -GRID_LEN, step, GRID_LEN, GRAY);
                step += GRID_STEP * mul;
            }
            step = 0.0;
            for(int i = 0; i < GRID_LINES; i++){
                DrawLine(step, -GRID_LEN, step, GRID_LEN, GRAY);
                step -= GRID_STEP * mul;
            }
            step = 0.0;
            for(int i = 0; i < GRID_LINES; i++){
                DrawLine(-GRID_LEN, step, GRID_LEN, step, GRAY);
                step += GRID_STEP * mul;
            }
            step = 0.0;
            for(int i = 0; i < GRID_LINES; i++){
                DrawLine(-GRID_LEN, step, GRID_LEN, step, GRAY);
                step -= GRID_STEP * mul;
            }
            // Draw asix lines
            DrawLine(0, -GRID_LEN, 0, GRID_LEN, RED);
            DrawLine(-GRID_LEN, 0, GRID_LEN, 0, BLUE);
            // Draw bodies
            for(unsigned i = 0; i < simulation.count; i++){
                SimulationBody body = simulation.bodies[i];
                // Draw trayectories
                if (body.trayectory.count > 1) {
                    for(unsigned j = 1; j < body.trayectory.count; j++){
                        DrawLineV(body.trayectory.points[j-1], body.trayectory.points[j], body.color);
                    }
                    DrawLineV(body.trayectory.points[body.trayectory.count-1], body.position, body.color);
                }
                DrawLineV(body.trayectory.points[body.trayectory.count-1], body.position, body.color);
                DrawCircle(body.position.x, body.position.y, body.radius, body.color);
                DrawText(TextFormat("%s", body.name), body.position.x, body.position.y - (1/camera.zoom) * 15, (1/camera.zoom) * 25, WHITE);
            }
        EndMode2D();
        DrawText(TextFormat("FPS: %d Bodies: %d tSpeed: %.0lf", GetFPS(), simulation.count, flags.tSpeed), 30, 30, 40, LIGHTGRAY);
    EndDrawing();
}












