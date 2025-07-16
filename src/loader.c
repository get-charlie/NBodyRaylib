#include "loader.h"

static char* read_file(const char* path)
{
    FILE* file = fopen(path, "rb");
    if(!file){
        fprintf(stderr, "Error: Could not open file %s\n", path);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* jsondata = malloc(length + 1);
    if(!jsondata){
        perror("Error: Malloc failed\n");
        fclose(file);
        return NULL;
    }

    fread(jsondata, 1, length, file);
    jsondata[length] = '\0';

    fclose(file);
    return jsondata;
}

static cJSON* parse_json(const char * jsondata)
{
    cJSON* json = cJSON_Parse(jsondata);
    if(!json){
        const char* error_ptr = cJSON_GetErrorPtr();
        if(error_ptr){
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        return NULL;
    }
    return json;
}

// returns 1 if an error is found
int load_simulation(Simulation* simulation, const char* path)
{
    *simulation = (Simulation){0};
    char* jsondata = read_file(path);
    if(!jsondata){
        fprintf(stderr, "Error: could not read json file\n");
        return 1;
    }

    cJSON* json = parse_json(jsondata);
    if(!json){
        fprintf(stderr, "Error: Could not parse json\n");
        return 1;
    }
    
    cJSON* scale = cJSON_GetObjectItemCaseSensitive(json, "scale");
    if(!cJSON_IsNumber(scale)){
        fprintf(stderr, "Error: time must me a number\n");
        return 1;
    }
    simulation->scale = scale->valuedouble;

    cJSON* bodiesjson =  cJSON_GetObjectItemCaseSensitive(json, "bodies");
    if(!cJSON_IsArray(bodiesjson)){
        fprintf(stderr, "Error: file is formated wrong\n");
        return 1;
    }
    
    cJSON* body = NULL;
    cJSON_ArrayForEach(body, bodiesjson){

        if(!cJSON_IsObject(body)){
            fprintf(stderr, "Error: invalid data in body object\n");
            return 1;
        }

        cJSON* name = cJSON_GetObjectItemCaseSensitive(body, "name");
        cJSON* color = cJSON_GetObjectItemCaseSensitive(body, "color");
        cJSON* mass = cJSON_GetObjectItemCaseSensitive(body, "mass");
        cJSON* radius = cJSON_GetObjectItemCaseSensitive(body, "radius");
        cJSON* position = cJSON_GetObjectItemCaseSensitive(body, "position");
        cJSON* velocity = cJSON_GetObjectItemCaseSensitive(body, "velocity");

        if(!cJSON_IsString(name) || !cJSON_IsObject(color) || !cJSON_IsNumber(mass) || !cJSON_IsNumber(radius) || !cJSON_IsObject(position) || !cJSON_IsObject(velocity)){
            fprintf(stderr, "Error: invalid data in body object\n");
            return 1;
        }

        cJSON* r = cJSON_GetObjectItemCaseSensitive(color, "r");
        cJSON* g = cJSON_GetObjectItemCaseSensitive(color, "g");
        cJSON* b = cJSON_GetObjectItemCaseSensitive(color, "b");

        if (!cJSON_IsNumber(r) || !cJSON_IsNumber(g) || !cJSON_IsNumber(b)){
            fprintf(stderr, "Error: Invalid color data in body object\n");
            cJSON_Delete(json);
            return 1;
        }

        cJSON* x = cJSON_GetObjectItemCaseSensitive(position, "x");
        cJSON* y = cJSON_GetObjectItemCaseSensitive(position, "y");

        if(!cJSON_IsNumber(x) || !cJSON_IsNumber(y)){
            fprintf(stderr, "Error: Invalid position data in body object\n");
            cJSON_Delete(json);
            return 1;
        }

        cJSON* vx = cJSON_GetObjectItemCaseSensitive(velocity, "x");
        cJSON* vy = cJSON_GetObjectItemCaseSensitive(velocity, "y");

        if(!cJSON_IsNumber(vx) || !cJSON_IsNumber(vy)){
            fprintf(stderr, "Error: Invalid velocity data in body object\n");
            cJSON_Delete(json);
            return 1;
        }

        Color col = {r->valueint, g->valueint, b->valueint, 255};
        Body simbody = new_body(
            name->valuestring,  col,
            mass->valuedouble,  radius->valuedouble,
            x->valuedouble,     y->valuedouble,
            vx->valuedouble,    vy->valuedouble,
            simulation->scale
        );
        add_simulation_body(simulation, simbody);
    }
    cJSON_Delete(json);
    return 0;
}

