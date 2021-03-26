#include <stdlib.h>
#include <string.h>
#include "dxf.h"
#include "dbgprint.h"

static unsigned int str_hash(const char **psz);
static int str_cmp(const char **psz1, const char **psz2);
static int init_entity(struct dxf_entity* const entity);

static unsigned int str_hash(const char **psz) 
{
    unsigned int hash = 0;
    const char *sz = *psz;
    
    while (*sz != '\0') {
        hash = *(sz++) + (hash << 5) - 1;
    }
    
    return hash;
}

static int str_cmp(const char **psz1, const char **psz2) 
{
    return strcmp(*psz1, *psz2);
}

static int init_entity(struct dxf_entity* const entity)
{
    struct dxf_point *point = (struct dxf_point*)entity;
    struct dxf_line *line = (struct dxf_line*)entity;
    struct dxf_circle *circle = (struct dxf_circle*)entity;
    struct dxf_lwpolyline *lwpolyline = (struct dxf_lwpolyline*)entity;

    switch (entity->type) {
        case DXF_POINT:
            point->x = point->y = point->z = 0.0;
            return 0;
        case DXF_LINE:
            line->x1 = line->x2 = line->y1 = line->y2 =
                line->z1 = line->z2 = 0.0;
            return 0;
        case DXF_CIRCLE:
            circle->x = circle->y = circle->z = circle->r = 0.0;
            return 0;
        case DXF_LWPOLYLINE:
            lwpolyline->closed = 0;
            lwpolyline->number_of_vertices = 0;
            lwpolyline->vertices = NULL;
            lwpolyline->tail_vertex = NULL;
            return 0;
        default:
            return -1;
    }
}

int dxf_init(struct dxf* const dxf, size_t pool_size)
{
    if (hashtable_create(&(dxf->header), 37, 0, 0, (pfn_hash_t)str_hash, (pfn_keycmp_t)str_cmp) != 0) {
        dbgprint("\ndxf: Failed to create header. \n");
        return -1;
    }
    
    if ((dxf->pool = crapool_create(pool_size, NULL)) == NULL) {
        dbgprint("\ndxf: Memory pool creation failed. \n");
        return -1;
    }

    dxf->layers = NULL;
    dxf->last_accessed_layer = NULL;

    if (dxf_add_layer(dxf, "0") == NULL) {
        dbgprint("\ndxf: Failed to add default layer 0. \n");
        crapool_destroy(dxf->pool);
        return -1;
    }
    
    dbgprint("\ndxf: Initialized dxf struct @0x%x, pool_size=%u. \n",
            (unsigned int)dxf, pool_size);
    
    return 0;
}

struct dxf_layer* dxf_add_layer(struct dxf* const dxf, const char *name)
{
    struct dxf_layer *layer;
    char *layer_name;
    size_t len = strlen(name);
        
    if (len == 0) {
        dbgprint("\ndxf: Layer name is empty. \n");
        return NULL;
    }

    if ((layer_name = dxf_alloc_string(dxf, len)) == NULL) {
        dbgprint("\ndxf: Failed to allocate pool space for storing layer name. \n");
        return NULL;
    }

    if ((layer = (struct dxf_layer*)crapool_alloc(dxf->pool, sizeof(struct dxf_layer))) == NULL) {
        dbgprint("\ndxf: Failed to allocate pool space for storing layer struct. \n");
        return NULL;
    }
    
    memset(&(layer->entities), 0, DXF_ENTITY_TYPES_COUNT * sizeof(struct dxf_entity*));
    memcpy(layer_name, name, len + 1);
    layer->name = layer_name;
    layer->next = dxf->layers;
    dxf->layers = layer;
    dxf->last_accessed_layer = layer;
    
    dbgprint("\ndxf: Added layer @0x%x, name=%s, entities=@0x%x, next=@0x%x \n",
            (unsigned int)layer, layer->name, (unsigned int)(layer->entities), 
            (unsigned int)(layer->next));
    
    return layer;
}

struct dxf_layer* dxf_get_layer(struct dxf* const dxf, const char *name)
{
    struct dxf_layer *layer;

    if (strcmp(dxf->last_accessed_layer->name, name) == 0) {
        layer = dxf->last_accessed_layer;
        dbgprint("\ndxf: Layer found (fast fetch) @0x%x, name=%s, entities=@0x%x, next=@0x%x \n",
            (unsigned int)layer, layer->name, 
            (unsigned int)(layer->entities), (unsigned int)(layer->next));
        return layer;
    }

    for (layer = dxf->layers; layer != NULL; layer = layer->next) {
        if (strcmp(layer->name, name) == 0) {
            dxf->last_accessed_layer = layer;
            dbgprint("\ndxf: Layer found @0x%x, name=%s, entities=@0x%x, next=@0x%x \n",
                    (unsigned int)layer, layer->name, 
                    (unsigned int)(layer->entities), (unsigned int)(layer->next));
            return layer;
        }
    }

    dbgprint("\ndxf: Layer %s not found. \n", name);
    return NULL;
}

int dxf_add_entity(struct dxf* const dxf, const char* layer_name,
                    struct dxf_entity* entity)
{
    struct dxf_layer *layer;
    const char *lay_name = layer_name;
    int type = entity->type;
    
    if ((type < DXF_ENTITY_TYPE_START) || (type > DXF_ENTITY_TYPE_END)) {
        dbgprint("\ndxf: Bad entity type %d. \n", type);
        return -1;
    }
    
    if (lay_name == NULL) {
        lay_name = "0";
    }

    if ((layer = dxf_get_layer(dxf, lay_name)) == NULL) {
        if ((layer = dxf_add_layer(dxf, lay_name)) == NULL) {
            dbgprint("\ndxf: Failed to allocate pool space for storing layer struct. \n");
            return -1;
        }
    }

    entity->layer = layer;
    entity->next = layer->entities[type];
    layer->entities[type] = entity;
    
    dbgprint("\ndxf: Added entity @0x%x (type=%d) to layer @0x%x (name=%s). \n",
                (unsigned int)entity, type, (unsigned int)layer, layer->name);

    return 0;
}

void* dxf_alloc_binary(struct dxf* const dxf, size_t size)
{
    return crapool_alloc(dxf->pool, size);
}

char* dxf_alloc_string(struct dxf* const dxf, size_t len)
{
    return crapool_calloc(dxf->pool, 1, len + 2);
}

struct dxf_entity* dxf_alloc_entity(struct dxf* const dxf, int entity_type)
{
    size_t size;
    struct dxf_entity *entity = NULL;
    
    switch (entity_type) {
        case DXF_POINT:
            size = sizeof(struct dxf_point);
            break;
        case DXF_LINE:
            size = sizeof(struct dxf_line);
            break;
        case DXF_CIRCLE:
            size = sizeof(struct dxf_circle);
            break;
        case DXF_LWPOLYLINE:
            size = sizeof(struct dxf_lwpolyline);
            break;
        default:
            dbgprint("\ndxf: Could not allocate space for entity type %d. \n", entity_type);
            return NULL;
    }
    
    if ((entity = (struct dxf_entity*)crapool_alloc(dxf->pool, size)) != NULL) {
        *((int*)(&(entity->type))) = entity_type;
        *((size_t*)(&(entity->size))) = size;
        entity->block_ref = NULL;
        init_entity(entity);
    }
    
    dbgprint("\ndxf: Allocated space for new entity @0x%x, type=%d, size=%u. \n",
                (unsigned int)entity, entity_type, entity->size);
    return entity;
}
