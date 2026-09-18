#include "sim_io.h"

#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "physics.h"
#include "raylib.h"

static cJSON* vector_json(Vec3 v, double scale)
{
    cJSON* object = cJSON_CreateObject();
    if(object == NULL){
        return NULL;
    }
    cJSON_AddNumberToObject(object, "x", v.x / scale);
    cJSON_AddNumberToObject(object, "y", v.y / scale);
    cJSON_AddNumberToObject(object, "z", v.z / scale);
    return object;
}

static cJSON* body_json(const Body* body, double scale)
{
    cJSON* object = cJSON_CreateObject();
    if(object == NULL){
        return NULL;
    }
    cJSON_AddStringToObject(object, "name", body->name);

    cJSON* color = cJSON_CreateObject();
    cJSON_AddNumberToObject(color, "r", body->color.r);
    cJSON_AddNumberToObject(color, "g", body->color.g);
    cJSON_AddNumberToObject(color, "b", body->color.b);
    cJSON_AddItemToObject(object, "color", color);

    cJSON_AddNumberToObject(object, "mass", body->mass);
    // The file keeps radii in its own units, the simulation in AU
    cJSON_AddNumberToObject(object, "radius", body->radius * scale);
    cJSON_AddItemToObject(object, "position", vector_json(body->position, AU));
    cJSON_AddItemToObject(object, "velocity", vector_json(body->velocity, KM));
    return object;
}

int save_simulation(const Simulation* simulation, const char* path)
{
    cJSON* root = cJSON_CreateObject();
    if(root == NULL){
        return 1;
    }
    cJSON_AddNumberToObject(root, "scale", simulation->scale);

    cJSON* bodies = cJSON_AddArrayToObject(root, "bodies");
    if(bodies == NULL){
        cJSON_Delete(root);
        return 1;
    }
    for(unsigned i = 0; i < simulation->count; i++){
        cJSON* body = body_json(&simulation->bodies[i], simulation->scale);
        if(body == NULL){
            cJSON_Delete(root);
            return 1;
        }
        cJSON_AddItemToArray(bodies, body);
    }

    char* text = cJSON_Print(root);
    cJSON_Delete(root);
    if(text == NULL){
        return 1;
    }

    FILE* file = fopen(path, "wb");
    if(file == NULL){
        fprintf(stderr, "Error: could not write %s\n", path);
        cJSON_free(text);
        return 1;
    }
    size_t length = strlen(text);
    size_t written = fwrite(text, 1, length, file);
    fputc('\n', file);
    fclose(file);
    cJSON_free(text);

    return written == length ? 0 : 1;
}

static int scan_directory(const char* directory, char list[][PATH_LEN], int max, int count)
{
    if(!DirectoryExists(directory)){
        return count;
    }
    FilePathList files = LoadDirectoryFilesEx(directory, ".json", false);
    for(unsigned i = 0; i < files.count && count < max; i++){
        const char* path = files.paths[i];
        // Keep the paths relative and short, they are shown in the dialogs
        if(path[0] == '.' && (path[1] == '/' || path[1] == '\\')){
            path += 2;
        }
        snprintf(list[count], PATH_LEN, "%s", path);
        count++;
    }
    UnloadDirectoryFiles(files);
    return count;
}

int list_json_files(char list[][PATH_LEN], int max)
{
    int count = scan_directory(".", list, max, 0);
    count = scan_directory("examples", list, max, count);
    return count;
}
