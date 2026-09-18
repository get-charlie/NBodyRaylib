#include "raylib.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "physics.h"
#include "simulation.h"
#include "threadpool.h"
#include "gui.h"
#include "creator.h"
#include "loader.h"

const int screenWidth = 1200;
const int screenHeight = 750;

#define MAX_SIM_SPEED   10000000.0
#define MAX_FRAME_TIME  0.1         // s, keeps a stall from jumping the simulation
#define ENERGY_PERIOD   30          // frames between energy measurements
#define MIN_BODY_SCALE  0.02f
#define MAX_BODY_SCALE  500.0f

typedef struct {
    const char* file;
    unsigned    threads;    // 0 selects one thread per processor
} Options;

static Options parse_args(int argc, char** argv)
{
    Options options = { .file = NULL, .threads = 0 };

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-t") == 0 && i + 1 < argc){
            options.threads = (unsigned)strtoul(argv[++i], NULL, 10);
        }
        else if(options.file == NULL){
            options.file = argv[i];
        }
        else{
            options.file = NULL;
            break;
        }
    }

    if(options.file == NULL || !strstr(options.file, ".json")){
        printf("Use: %s <simulation.json> [-t threads]\n", argv[0]);
        printf("  -t threads   worker threads of the simulation (default: one per core)\n");
        exit(0);
    }
    return options;
}

static void cycle_selection(Simulation* simulation, OrbitCamera* camera, DisplayFlags* flags, int step)
{
    if(simulation->count == 0){
        return;
    }
    int count = (int)simulation->count;
    int next = flags->selected < 0 ? 0 : (flags->selected + count + step) % count;
    flags->selected = next;
    follow_body(camera, simulation, next);
}

static void handle_input(Simulation* simulation, OrbitCamera* camera, DisplayFlags* flags, Creator* creator, bool* quit)
{
    if(IsKeyPressed(KEY_F11)){
        ToggleFullscreen();
    }
    if(IsKeyPressed(KEY_ESCAPE)){
        *quit = true;
    }

    // Time controls
    if(IsKeyPressed(KEY_RIGHT)){
        if(flags->t_speed > 0.0 && flags->t_speed < MAX_SIM_SPEED){
            flags->t_speed *= 10.0;
        }
        else if(flags->t_speed < 1.0){
            flags->t_speed = 1.0;
        }
    }
    if(IsKeyPressed(KEY_LEFT)){
        if(flags->t_speed > 1.0){
            flags->t_speed /= 10.0;
        }
        else{
            flags->t_speed = 0.0;
        }
    }
    if(IsKeyPressed(KEY_SPACE)){
        flags->paused = !flags->paused;
    }

    // Display
    if(IsKeyPressed(KEY_T)){
        flags->display_trayectory = !flags->display_trayectory;
        if(flags->display_trayectory){
            clear_trayectories(simulation);
        }
    }
    if(IsKeyPressed(KEY_I)){
        flags->debug = !flags->debug;
    }
    if(IsKeyPressed(KEY_N)){
        flags->names = !flags->names;
    }
    if(IsKeyPressed(KEY_G)){
        flags->grid = !flags->grid;
    }
    if(IsKeyPressed(KEY_H)){
        flags->help = !flags->help;
    }
    if(IsKeyPressed(KEY_B)){
        flags->body_scale = Clamp(flags->body_scale * 1.25f, MIN_BODY_SCALE, MAX_BODY_SCALE);
    }
    if(IsKeyPressed(KEY_V)){
        flags->body_scale = Clamp(flags->body_scale / 1.25f, MIN_BODY_SCALE, MAX_BODY_SCALE);
    }

    // Camera targets
    if(IsKeyPressed(KEY_R)){
        frame_simulation(camera, simulation);
    }
    if(IsKeyPressed(KEY_F)){
        if(camera->follow >= 0 && camera->follow == flags->selected){
            camera->follow = -1;
        }
        else{
            follow_body(camera, simulation, flags->selected);
        }
    }
    if(IsKeyPressed(KEY_TAB)){
        bool back = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        cycle_selection(simulation, camera, flags, back ? -1 : 1);
    }

    // Body creator
    if(IsKeyPressed(KEY_C)){
        creator_open(creator, simulation, camera);
    }
}

int main(int argc, char** argv)
{
    Options options = parse_args(argc, argv);

    Simulation* simulation = malloc(sizeof(Simulation));
    if(simulation == NULL){
        fprintf(stderr, "Could not allocate simulation.\n");
        exit(1);
    }

    if(load_simulation(simulation, options.file)){
        fprintf(stderr, "Error loading the file.\n");
        free(simulation);
        exit(1);
    }

    ThreadPool* pool = threadpool_create(options.threads);
    if(pool == NULL){
        fprintf(stderr, "Warning: could not create the thread pool, running single threaded.\n");
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "NBodyRaylib");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);   // esc is handled with the rest of the input

    OrbitCamera camera;
    init_camera(&camera, simulation);

    DisplayFlags flags = {0};
    flags.t_speed = 1.0;
    flags.debug = true;
    flags.names = true;
    flags.grid = true;
    flags.display_trayectory = true;
    flags.body_scale = 1.0f;
    flags.selected = -1;

    Creator creator;
    creator_init(&creator);

    SimStats stats = { .threads = threadpool_size(pool), .energy = 0.0, .energy0 = 0.0 };
    stats.energy0 = simulation_energy(simulation);
    stats.energy = stats.energy0;

    bool quit = false;
    int frame = 0;
    unsigned bodies = simulation->count;

    while(!WindowShouldClose() && !quit){

        bool typing = creator_update(&creator, simulation, &flags);
        if(!typing){
            handle_input(simulation, &camera, &flags, &creator, &quit);
        }

        bool over_panel = creator.active && CheckCollisionPointRec(GetMousePosition(), creator_area());
        update_camera(&camera, simulation, !over_panel, !typing);

        // A short click that did not orbit the camera selects a body
        if(!over_panel && IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && camera.drag < CLICK_SLOP){
            flags.selected = pick_body(&camera, simulation, GetMousePosition());
        }

        double frame_time = fmin(GetFrameTime(), MAX_FRAME_TIME);
        double elapsed = flags.paused ? 0.0 : frame_time * flags.t_speed;

        update_simulation(simulation, pool, elapsed);

        if(flags.display_trayectory && elapsed != 0.0){
            update_trayectories(simulation);
        }
        // Adding a body changes the energy of the system, the reference of
        // the drift has to follow it
        if(simulation->count != bodies){
            bodies = simulation->count;
            stats.energy0 = simulation_energy(simulation);
        }
        if(flags.debug && frame % ENERGY_PERIOD == 0){
            stats.energy = simulation_energy(simulation);
        }

        GhostBody ghost = creator_ghost(&creator, simulation);
        begin_frame();
            draw_scene(&camera, simulation, &flags, &ghost);
            draw_hud(&camera, simulation, &flags, &stats);
            creator_draw(&creator);
        end_frame();

        frame++;
    }

    threadpool_destroy(pool);
    free(simulation);
    CloseWindow();
    return 0;
}
