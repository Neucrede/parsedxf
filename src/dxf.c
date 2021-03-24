#include <stdlib.h>
#include <string.h>
#include "dxf.h"
#include "dbgprint.h"

static unsigned int str_hash(const char **psz);
static int str_cmp(const char **psz1, const char **psz2);

static unsigned int str_hash(const char **psz) {
    unsigned int hash = 0;
    const char *sz = *psz;
    
    while (*sz != '\0') {
        hash = *(sz++) + (hash << 5) - 1;
    }
    
    return hash;
}

static int str_cmp(const char **psz1, const char **psz2) {
    return strcmp(*psz1, *psz2);
}

int dxf_init(struct dxf* const dxf, size_t pool_size)
{
    if (hashtable_create(&(dxf->header), 37, 0, 0, (pfn_hash_t)str_hash, (pfn_keycmp_t)str_cmp) != 0) {
        return -1;
    }
    
    if ((dxf->pool = crapool_create(pool_size, NULL)) == NULL) {
        crapool_destroy(dxf->pool);
        return -1;
    }

    dxf->layers = NULL;
    dxf->last_accessed_layer = NULL;

    if (dxf_add_layer(dxf, "0") == NULL) {
        crapool_destroy(dxf->pool);
        return -1;
    }
    
    return 0;
}

struct dxf_layer* dxf_add_layer(struct dxf* const dxf, const char *name)
{
    struct dxf_layer *layer;
    char *layer_name;
    size_t len = strlen(name);
        
    if (len == 0) {
        return NULL;
    }

    if ((layer_name = crapool_alloc(dxf->pool, len + 2)) == NULL) {
        return NULL;
    }

    if ((layer = crapool_alloc(dxf->pool, sizeof(struct dxf_layer))) == NULL) {
        return NULL;
    }

    memcpy(layer_name, name, len + 1);
    layer->name = layer_name;
    layer->next = dxf->layers;
    dxf->layers = layer;
    dxf->last_accessed_layer = layer;
    return layer;
}

const struct dxf_layer* dxf_get_layer(struct dxf* const dxf, const char *name)
{
    struct dxf_layer *layer;

    if (strcmp(dxf->last_accessed_layer->name, name) == 0) {
        return dxf->last_accessed_layer;
    }

    for (layer = dxf->layers; layer != NULL; layer = layer->next) {
        if (strcmp(layer->name, name) == 0) {
            dxf->last_accessed_layer = layer;
            return layer;
        }
    }

    return NULL;
}

int dxf_add_entity(struct dxf* const dxf, const char* layer_name,
                    struct dxf_entity* entity)
{
    struct dxf_layer *layer;

    if ((layer = (struct dxf_layer*)dxf_get_layer(dxf, layer_name)) == NULL) {
        if ((layer = dxf_add_layer(dxf, layer_name)) == NULL) {
            return -1;
        }
    }

    entity->layer = layer;
    entity->next = layer->entities;
    layer->entities = entity;

    return 0;
}

void* dxf_pool_alloc(struct dxf* const dxf, size_t size)
{
    return crapool_alloc(dxf->pool, size);
}

