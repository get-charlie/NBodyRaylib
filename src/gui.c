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

void draw(Camera2D camera, Simulation simulation){
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
            DrawLine(0, -GRID_LEN, 0, GRID_LEN, WHITE);
            DrawLine(-GRID_LEN, 0, GRID_LEN, 0, WHITE);
            // Draw bodies
            for(unsigned i = 0; i < simulation.num_bodies; i++){
                SimulationBody body = simulation.bodies[i];
                DrawCircle(body.position.x, body.position.y, body.radius, body.color);
            }
            //DrawText("Sun", -50, -100, (1 / camera.zoom) * 40, LIGHTGRAY);
        EndMode2D();
    EndDrawing();
}












