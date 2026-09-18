#include "raylib.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "physics.h"
#include "simulation.h"
#include "threadpool.h"
#include "gui.h"
#include "creator.h"
#include "saver.h"
#include "menu.h"
#include "loader.h"

const int screenWidth = 1200;
const int screenHeight = 750;

#define MAX_FRAME_TIME  0.1         // s, keeps a stall from jumping the simulation
#define ENERGY_PERIOD   30          // frames between energy measurements
#define DEFAULT_SCALE   250000.0

typedef enum {
    APP_MENU = 0,
    APP_SIM
} AppState;

static void handle_input(Simulation* simulation, OrbitCamera* camera, DisplayFlags* flags, UiAction* action)
{
    if(IsKeyPressed(KEY_F11)){
        ToggleFullscreen();
    }
    if(IsKeyPressed(KEY_ESCAPE)){
        *action = ACTION_QUIT;
    }

    // Time controls
    if(IsKeyPressed(KEY_RIGHT)){
        speed_up(flags);
    }
    if(IsKeyPressed(KEY_LEFT)){
        speed_down(flags);
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
    if(IsKeyPressed(KEY_K)){
        flags->height_lines = !flags->height_lines;
    }
    if(IsKeyPressed(KEY_L)){
        flags->lighting = !flags->lighting;
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
        scale_bodies(flags, 1.25f);
    }
    if(IsKeyPressed(KEY_V)){
        scale_bodies(flags, 1.0f / 1.25f);
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

    // Panels
    if(IsKeyPressed(KEY_C)){
        *action = ACTION_CREATOR;
    }
    if(IsKeyPressed(KEY_O)){
        *action = ACTION_SAVE;
    }
    if(IsKeyPressed(KEY_M)){
        *action = ACTION_MENU;
    }
}

static DisplayFlags default_flags(void)
{
    DisplayFlags flags = {0};
    flags.t_speed = 1.0;
    flags.debug = true;
    flags.names = true;
    flags.grid = true;
    flags.display_trayectory = true;
    flags.height_lines = true;
    flags.lighting = true;
    flags.body_scale = 1.0f;
    flags.selected = -1;
    return flags;
}

int main(int argc, char** argv)
{
    Simulation* simulation = malloc(sizeof(Simulation));
    if(simulation == NULL){
        fprintf(stderr, "Could not allocate simulation.\n");
        exit(1);
    }
    *simulation = (Simulation){0};
    simulation->scale = DEFAULT_SCALE;

    // A file on the command line opens straight into the simulation, with
    // no arguments the program starts on the main menu
    bool loaded = false;
    if(argc > 1){
        if(load_simulation(simulation, argv[1])){
            fprintf(stderr, "Error loading the file.\n");
            free(simulation);
            exit(1);
        }
        loaded = true;
    }

    // The pool holds one worker per processor, how many of them a step
    // actually uses depends on the number of bodies
    ThreadPool* pool = threadpool_create(0);
    if(pool == NULL){
        fprintf(stderr, "Warning: could not create the thread pool, running single threaded.\n");
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "NBodyRaylib");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);   // esc is handled with the rest of the input

    OrbitCamera camera;
    init_camera(&camera, simulation);

    DisplayFlags flags = default_flags();
    SimStats stats = {0};
    stats.energy0 = simulation_energy(simulation);
    stats.energy = stats.energy0;

    Creator creator;
    creator_init(&creator);
    Saver saver;
    saver_init(&saver);
    Menu menu;
    menu_init(&menu);
    if(argc > 1){
        snprintf(menu.source, PATH_LEN, "%s", GetFileName(argv[1]));
    }

    AppState state = loaded ? APP_SIM : APP_MENU;
    if(state == APP_MENU){
        menu_open(&menu, false);
    }

    bool quit = false;
    int frame = 0;
    unsigned bodies = simulation->count;

    while(!WindowShouldClose() && !quit){

        if(state == APP_MENU){
            begin_frame();
            MenuResult result = menu_run(&menu, simulation);
            end_frame();

            if(result == MENU_START){
                init_camera(&camera, simulation);
                flags = default_flags();
                creator_close(&creator);
                saver_close(&saver);
                saver_init(&saver);
                snprintf(saver.name, PATH_LEN, "%s", menu.source);
                bodies = simulation->count;
                stats.energy0 = simulation_energy(simulation);
                stats.energy = stats.energy0;
                state = APP_SIM;
            }
            else if(result == MENU_RESUME){
                state = APP_SIM;
            }
            else if(result == MENU_QUIT){
                quit = true;
            }
            continue;
        }

        UiAction action = ACTION_NONE;

        // Forms take the keyboard while they are open
        bool typing = saver_update(&saver, simulation);
        if(!saver.active){
            typing = creator_update(&creator, simulation, &flags) || typing;
        }
        if(!typing){
            handle_input(simulation, &camera, &flags, &action);
        }

        bool over_ui = ui_mouse_in(toolbar_area())
                    || (creator.active && ui_mouse_in(creator_area()))
                    || (saver.active && ui_mouse_in(saver_area()));

        update_camera(&camera, simulation, !over_ui, !typing);

        // A short click that did not orbit the camera selects a body
        if(!over_ui && IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && camera.drag < CLICK_SLOP){
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
        stats.threads = threads_for_bodies(simulation->count);

        GhostBody ghost = creator_ghost(&creator, simulation);
        begin_frame();
            draw_scene(&camera, simulation, &flags, &ghost);
            draw_hud(&camera, simulation, &flags, &stats);
            creator_draw(&creator, simulation, &flags);
            saver_draw(&saver, simulation);
            UiAction clicked = draw_toolbar(&camera, simulation, &flags);
        end_frame();

        if(clicked != ACTION_NONE){
            action = clicked;
        }

        switch(action){
            case ACTION_CREATOR:
                saver_close(&saver);
                creator_open(&creator, simulation, &camera);
                break;
            case ACTION_SAVE:
                creator_close(&creator);
                saver_open(&saver, saver.name);
                break;
            case ACTION_MENU:
                creator_close(&creator);
                saver_close(&saver);
                menu_open(&menu, true);
                state = APP_MENU;
                break;
            case ACTION_QUIT:
                quit = true;
                break;
            default:
                break;
        }

        frame++;
    }

    threadpool_destroy(pool);
    free(simulation);
    CloseWindow();
    return 0;
}
