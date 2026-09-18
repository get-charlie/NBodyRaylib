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

// The z component is optional so the 2D simulations written for the old
// version keep working, they just run on the z = 0 plane.
static int get_vector(const cJSON* object, Vec3* out)
{
    cJSON* x = cJSON_GetObjectItemCaseSensitive(object, "x");
    cJSON* y = cJSON_GetObjectItemCaseSensitive(object, "y");
    cJSON* z = cJSON_GetObjectItemCaseSensitive(object, "z");

    if(!cJSON_IsNumber(x) || !cJSON_IsNumber(y)){
        return 1;
    }
    if(z != NULL && !cJSON_IsNumber(z)){
        return 1;
    }

    out->x = x->valuedouble;
    out->y = y->valuedouble;
    out->z = cJSON_IsNumber(z) ? z->valuedouble : 0.0;
    return 0;
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
    free(jsondata);
    if(!json){
        fprintf(stderr, "Error: Could not parse json\n");
        return 1;
    }
    
    cJSON* scale = cJSON_GetObjectItemCaseSensitive(json, "scale");
    if(!cJSON_IsNumber(scale) || scale->valuedouble <= 0.0){
        cJSON_Delete(json);
        fprintf(stderr, "Error: scale must be a positive number\n");
        return 1;
    }
    simulation->scale = scale->valuedouble;

    cJSON* bodiesjson =  cJSON_GetObjectItemCaseSensitive(json, "bodies");
    if(!cJSON_IsArray(bodiesjson)){
        fprintf(stderr, "Error: file is formated wrong\n");
        cJSON_Delete(json);
        return 1;
    }
    
    cJSON* body = NULL;
    cJSON_ArrayForEach(body, bodiesjson){

        if(!cJSON_IsObject(body)){
            fprintf(stderr, "Error: invalid data in body object\n");
            cJSON_Delete(json);
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
            cJSON_Delete(json);
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

        Vec3 pos = {0};
        if(get_vector(position, &pos)){
            fprintf(stderr, "Error: Invalid position data in body object\n");
            cJSON_Delete(json);
            return 1;
        }

        Vec3 vel = {0};
        if(get_vector(velocity, &vel)){
            fprintf(stderr, "Error: Invalid velocity data in body object\n");
            cJSON_Delete(json);
            return 1;
        }

        Color col = {r->valueint, g->valueint, b->valueint, 255};
        Body simbody = new_body(
            name->valuestring,  col,
            mass->valuedouble,  radius->valuedouble,
            pos,                vel,
            simulation->scale
        );
        if(!add_simulation_body(simulation, simbody)){
            fprintf(stderr, "Error: too many bodies, the limit is %d\n", MAX_BODIES);
            cJSON_Delete(json);
            return 1;
        }
    }
    cJSON_Delete(json);
    return 0;
}
