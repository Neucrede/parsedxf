#ifndef __DXF_H__
#define __DXF_H__

#include "hashtab.h"
#include "crapool.h"

struct dxf_layer;
struct dxf_entity;

struct dxf {
    struct hashtable header;
    struct dxf_layer *layers;
    struct dxf_layer *last_accessed_layer;
    struct crapool_desc *pool;
};

struct dxf_layer {
    char *name;
    struct dxf_entity *entities;
    struct dxf_layer *next;
};

struct dxf_entity {
    int type;
    struct dxf_layer *layer;
    struct dxf_entity *next;
};

struct dxf_point {
    struct dxf_entity header;
    float x;
    float y;
    float z;
};

struct dxf_vertex;
struct dxf_vertex {
    struct dxf_entity header;
    float x;
    float y;
    float z;
    struct dxf_vertex *next;
};

struct dxf_line {
    struct dxf_entity header;
    float x1;
    float y1;
    float z1;
    float x2;
    float y2;
    float z2;
};

#ifdef __cplusplus
extern "C" {
#endif
    
int dxf_init(struct dxf* const dxf, size_t pool_size);
struct dxf_layer* dxf_add_layer(struct dxf* const dxf, const char *name);
const struct dxf_layer* dxf_get_layer(struct dxf* const dxf, const char *name);
int dxf_add_entity(struct dxf* const dxf, const char* layer_name,
                    struct dxf_entity* entity);
void* dxf_pool_alloc(struct dxf* const dxf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* __DXF_H__ */
