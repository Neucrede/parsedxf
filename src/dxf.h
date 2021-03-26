#ifndef __DXF_H__
#define __DXF_H__

#include "hashtab.h"
#include "crapool.h"

#define DXF_ENTITY_TYPE_START 0
#define DXF_POINT 0
#define DXF_LINE 1
#define DXF_ARC 2
#define DXF_CIRCLE 3
#define DXF_ELLIPSE 4
#define DXF_VERTEX 5
#define DXF_POLYLINE 6
#define DXF_LWPOLYLINE_VERTEX 7
#define DXF_LWPOLYLINE 8
#define DXF_SPLINE 9
#define DXF_DIMENSION 10
#define DXF_HATCH 11
#define DXF_INSERT 12
#define DXF_TEXTSTRING 13
#define DXF_MTEXT 14
#define DXF_SOLID 15
#define DXF_ENTITY_TYPE_END 15
#define DXF_ENTITY_TYPES_COUNT (DXF_ENTITY_TYPE_END + 1)

struct dxf_layer;
struct dxf_entity;
struct dxf_block;

struct dxf {
    struct hashtable header;
    struct dxf_layer *layers;
    struct dxf_layer *last_accessed_layer;
    struct crapool_desc *pool;
};

struct dxf_layer {
    char *name;
    struct dxf_entity *entities[DXF_ENTITY_TYPES_COUNT];
    struct dxf_layer *next;
};

struct dxf_entity {
    const size_t size;
    const int type;
    struct dxf_layer *layer;
    struct dxf_block *block_ref;
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

struct dxf_circle {
    struct dxf_entity header;
    float x;
    float y;
    float z;
    float r;
};

#ifdef __cplusplus
extern "C" {
#endif
    
int dxf_init(struct dxf* const dxf, size_t pool_size);
struct dxf_layer* dxf_add_layer(struct dxf* const dxf, const char *name);
struct dxf_layer* dxf_get_layer(struct dxf* const dxf, const char *name);
int dxf_add_entity(struct dxf* const dxf, const char* layer_name,
                    struct dxf_entity* entity);
void* dxf_alloc_binary(struct dxf* const dxf, size_t size);
char* dxf_alloc_string(struct dxf* const dxf, size_t len);
struct dxf_entity* dxf_alloc_entity(struct dxf* const dxf, int entity_type);

#ifdef __cplusplus
}
#endif

#endif /* __DXF_H__ */
