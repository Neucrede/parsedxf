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
    struct dxf_arc *arc = (struct dxf_arc*)entity;
    struct dxf_insert *insert = (struct dxf_insert*)entity;

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
            lwpolyline->flag = DXF_LWPOLYLINE_FLAG_DEFAULT;
            lwpolyline->number_of_vertices = 0;
            lwpolyline->vertices = NULL;
            lwpolyline->tail_vertex = NULL;
            return 0;
        case DXF_ARC:
            arc->x = arc->y = arc->z = arc->r = 0.0;
            arc->angle_start = arc->angle_end = 0.0;
            return 0;
        case DXF_INSERT:
            insert->x = insert->y = insert->z = insert->angle = 0.0;
            insert->x_scale = insert->y_scale = insert->z_scale = 1.0;
            insert->column_count = insert->row_count = 1;
            insert->column_spacing = insert->row_spacing = 0.0; 
            return 0;
        default:
            return -1;
    }
}

int dxf_init(struct dxf* const dxf, size_t pool_size)
{
    if (hashtable_create(&(dxf->header), 37, 0, 0, HASHTABLE_COPY_VALUE,
        HASHTABLE_COPY_VALUE, (pfn_hash_t)str_hash, (pfn_keycmp_t)str_cmp, NULL) != 0) 
    {
        errprint("dxf: dxf_init(): Failed to create header. \n");
        return -1;
    }
    
    if ((dxf->pool = crapool_create(pool_size, NULL)) == NULL) {
        errprint("dxf: dxf_init(): Memory pool creation failed. \n");
        return -1;
    }

    dxf->layers = NULL;
    dxf->last_accessed_layer = NULL;
    dxf->blocks = NULL;
    dxf->last_accessed_block = NULL;

    if (dxf_add_layer(dxf, "0") == NULL) {
        errprint("dxf: dxf_init(): Failed to add default layer 0. \n");
        crapool_destroy(dxf->pool);
        return -1;
    }
    
    dbgprint("dxf: dxf_init(): Initialized dxf struct @0x%lx, pool_size=%zu. \n",
            (unsigned long)dxf, pool_size);
    
    return 0;
}

int dxf_free(struct dxf* const dxf)
{
    if (dxf->pool != NULL) {
        crapool_destroy(dxf->pool);
    }

    hashtable_destroy(&(dxf->header));
    
    dxf->pool = NULL;
    dxf->layers = NULL;
    dxf->last_accessed_layer = NULL;
    dxf->blocks = NULL;
    dxf->last_accessed_block = NULL;
    return 0;
}

struct dxf_container* dxf_add_container(struct dxf* const dxf, const char *name, 
                                        struct dxf_layer* parent_layer, int type)
{
    struct dxf_container *container;
    struct dxf_container *head_old;
    char *container_name;
    size_t len = strlen(name);
        
    if (len == 0) {
        errprint("dxf: Container name is empty. \n");
        return NULL;
    }

    if ((type != DXF_BLOCK) && (type != DXF_LAYER)) {
        errprint("dxf: dxf_add_container(): Bad container type %d. \n", type);
    }

    if ((container_name = dxf_alloc_string(dxf, len)) == NULL) {
        errprint("dxf: dxf_add_container(): Failed to allocate pool space for " \
                "storing container name. \n");
        return NULL;
    }

    if ((container = (struct dxf_container*)crapool_alloc(dxf->pool, sizeof(struct dxf_container))) == NULL) {
        errprint("dxf: dxf_add_container(): Failed to allocate pool space for " \
                "storing container struct. \n");
        return NULL;
    }
    
    memset(&(container->entities), 0, DXF_ENTITY_TYPES_COUNT * sizeof(struct dxf_entity*));
    memcpy(container_name, name, len + 1);
    container->type = type;
    *((char**)(&(container->name))) = container_name;
    container->flag = 0;
    container->x = container->y = container->z = 0.0;

    switch (type) {
        case DXF_LAYER:
            head_old = dxf->layers;
            container->next = head_old;
            dxf->layers = container;
            dxf->last_accessed_layer = container;
            container->parent = NULL;
            break;
        case DXF_BLOCK:
            head_old = dxf->blocks;
            container->next = head_old;
            dxf->blocks = container;
            dxf->last_accessed_block = container;
            container->parent = (struct dxf_container*)parent_layer;
            break;
        default:
            errprint("dxf: dxf_add_container(): Bad container type %d. Control flow was messed up. \n", type);
            return NULL;
    }
    
    dbgprint("dxf: dxf_add_container(): Added container @0x%lx, name=%s, entities=@0x%lx, next=@0x%lx, type=%d \n",
            (unsigned long)container, container->name, (unsigned long)(container->entities), 
            (unsigned long)(container->next), type);
    
    return container;
}

struct dxf_container* dxf_get_container(struct dxf* const dxf, const char *name, int type)
{
    struct dxf_container *container;
    struct dxf_container *head;
    struct dxf_container **specific_last_accessed_container;

    switch (type) {
        case DXF_LAYER:
            specific_last_accessed_container = &(dxf->last_accessed_layer);
            head = dxf->layers;
            break;
        case DXF_BLOCK:
            specific_last_accessed_container = &(dxf->last_accessed_block);
            head = dxf->blocks;
            break;
        default:
            errprint("dxf: Bad container type.");
            return NULL;
    }

    if (*specific_last_accessed_container == NULL) {
        return NULL;
    }

    if (strcmp((*specific_last_accessed_container)->name, name) == 0) {
        container = *specific_last_accessed_container;
        dbgprint("dxf: Container found (fast fetch) @0x%lx, name=%s, entities=@0x%lx, next=@0x%lx \n",
            (unsigned long)container, container->name, 
            (unsigned long)(container->entities), (unsigned long)(container->next));
        return container;
    }

    for (container = head; container != NULL; container = container->next) {
        if (strcmp(container->name, name) == 0) {
            *specific_last_accessed_container = container;
            dbgprint("dxf: dxf_get_container(): Container found @0x%lx, " \
                    "name=%s, entities=@0x%lx, next=@0x%lx \n",
                    (unsigned long)container, container->name, 
                    (unsigned long)(container->entities), (unsigned long)(container->next));
            return container;
        }
    }

    dbgprint("dxf: dxf_get_container(): Container %s not found. \n", name);
    return NULL;
}

int dxf_add_entity(struct dxf* const dxf, const char* container_name,
                    struct dxf_entity* entity, int behaviour)
{
    struct dxf_container *container;
    int entity_type = entity->type;
    int container_type;
    
    if ((entity_type < DXF_ENTITY_TYPE_START) || (entity_type > DXF_ENTITY_TYPE_END)) {
        errprint("dxf: dxf_add_entity(): Bad entity type %d. \n", entity_type);
        return -1;
    }

    if (container_name == NULL) {
        if (behaviour == DXF_ADD_ENTITY_TO_LAYER) {
            container_name = "0";
        }
        else {
            errprint("dxf: dxf_add_entity(): Anonymous block. \n");
            return -1;
        }
    }

    switch (behaviour) {
        case DXF_ADD_ENTITY_TO_LAYER:
            container_type = DXF_LAYER;
            break;
        case DXF_ADD_ENTITY_TO_BLOCK:
            container_type = DXF_BLOCK;
            break;
        default:
            errprint("dxf: dxf_add_entity(): Unrecognized behaviour %d. \n", behaviour);
            return -1;
    }

    if ((container = dxf_get_container(dxf, container_name, container_type)) == NULL) {
        if ((container = dxf_add_container(dxf, container_name, NULL, container_type)) == NULL) {
            errprint("dxf: dxf_add_entity(): Failed to allocate pool space for storing container struct. \n");
            return -1;
        }
    }

    switch (behaviour) {
        case DXF_ADD_ENTITY_TO_LAYER:
            entity->layer = container;
            break;
        case DXF_ADD_ENTITY_TO_BLOCK:
            entity->block = container;
            break;
        default:
            errprint("dxf: dxf_add_entity(): Unrecognized behaviour %d. \n", behaviour);
            return -1;
    }

    entity->next = container->entities[entity_type];
    container->entities[entity_type] = entity;
    
    dbgprint("dxf: dxf_add_entity(): Added entity @0x%lx (type=%d) to " \
                "container @0x%lx (name=%s, type=%d). \n",
                (unsigned long)entity, entity_type, 
                (unsigned long)container, container->name, container->type);

    return 0;
}

void* dxf_alloc_binary(struct dxf* const dxf, size_t size)
{
    void *buf = crapool_alloc(dxf->pool, size);

    if (buf == NULL) {
        errprint("dxf: dxf_alloc_binary(): Allocation failed. size=%zu \n", size);
    }

    return buf;
}

char* dxf_alloc_string(struct dxf* const dxf, size_t len)
{
    char *str = crapool_calloc(dxf->pool, 1, len + 2);

    if (str == NULL) {
        errprint("dxf: dxf_alloc_string(): String buffer allocation failed. size=%zu \n", len);
    }

    return str;
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
        case DXF_ARC:
            size = sizeof(struct dxf_arc);
            break;
        case DXF_INSERT:
            size = sizeof(struct dxf_insert);
            break;
        default:
            errprint("dxf: dxf_alloc_entity(): Could not allocate space " \
                    "for entity type %d. \n", entity_type);
            return NULL;
    }
    
    if ((entity = (struct dxf_entity*)crapool_alloc(dxf->pool, size)) != NULL) {
        *((int*)(&(entity->type))) = entity_type;
        *((size_t*)(&(entity->size))) = size;
        entity->layer = NULL;
        entity->block = NULL;
        entity->next = NULL;
        entity->user_data = NULL;
        init_entity(entity);
    }
    
    dbgprint("dxf: dxf_alloc_entity(): Allocated space for new entity @0x%lx, " \
                "type=%d, size=%zu. \n",
                (unsigned long)entity, entity_type, entity->size);
    return entity;
}
