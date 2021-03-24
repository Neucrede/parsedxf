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

int dxf_init(struct dxf* const dxf, size_t size_reserved)
{
    if (hashtable_create(&(dxf->header), 37, 0, 0, (pfn_hash_t)str_hash, (pfn_keycmp_t)str_cmp) != 0) {
        return -1;
    }
    
    if ((dxf->pool = crapool_create(size_reserved, NULL)) == NULL) {
        return -1;
    }
    
    return 0;
}

int dxf_add_layer(struct dxf* const dxf, const char *name)
{
    struct dxf_layer *layer;
    char *layer_name;
    size_t len = strlen(name);
        
    if (len == 0) {
        return -1;
    }

    if ((layer_name = crapool_alloc(dxf->pool, len + 2)) == NULL) {
        return -1;
    }

    if ((layer = crapool_alloc(dxf->pool, sizeof(struct dxf_layer))) == NULL) {
        return -1;
    }

    memcpy(layer_name, name, len + 1);
    layer->name = layer_name;
    layer->next = dxf->layers;
    dxf->layers = layer;
    
    return 0;
}
