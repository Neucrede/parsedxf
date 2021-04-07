#ifndef __DXF_H__
#define __DXF_H__

#include "hashtab.h"
#include "crapool.h"

#define DXF_LAYER 0
#define DXF_BLOCK 1

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

#define DXF_LWPOLYLINE_FLAG_DEFAULT 0
#define DXF_LWPOLYLINE_FLAG_CLOSED 1
#define DXF_LWPOLYLINE_FLAG_PLINEGEN 128

#define DXF_ADD_ENTITY_TO_LAYER 0
#define DXF_ADD_ENTITY_TO_BLOCK 1

struct dxf_entity;
struct dxf_container;

struct dxf_container {
    int type;
    const char* const name;
    int flag;
    double x;
    double y;
    double z;
    struct dxf_entity *entities[DXF_ENTITY_TYPES_COUNT];
    struct dxf_container *parent;
    struct dxf_container *next;
};

#define dxf_layer dxf_container
#define dxf_block dxf_container

struct dxf {
    struct hashtable header;
    struct dxf_layer *layers;
    struct dxf_layer *last_accessed_layer;
    struct dxf_block *blocks;
    struct dxf_block *last_accessed_block;
    struct crapool_desc *pool;
};

struct dxf_entity {
    const size_t size;
    const int type;
    struct dxf_layer *layer;
    struct dxf_block *block;
    struct dxf_entity *next;
    void *user_data;
};

struct dxf_point {
    struct dxf_entity header;
    double x;
    double y;
    double z;
};

struct dxf_vertex;
struct dxf_vertex {
    struct dxf_entity header;
    double x;
    double y;
    double z;
    struct dxf_vertex *next;
};

struct dxf_line {
    struct dxf_entity header;
    double x1;
    double y1;
    double z1;
    double x2;
    double y2;
    double z2;
};

struct dxf_circle {
    struct dxf_entity header;
    double x;
    double y;
    double z;
    double r;
};

struct dxf_lwpolyline_vertex;
struct dxf_lwpolyline_vertex {
    double x;
    double y;
    double z;

    /* bulge = tan(theta / 4).
     * Theta is the included angle of the arc that goes *COUNTER
     * CLOCKWISE* from the starting point to the end point.
     */
    double bulge;

    struct dxf_lwpolyline_vertex *next;
};

struct dxf_lwpolyline {
    struct dxf_entity header;
    size_t number_of_vertices;
    int flag;
    struct dxf_lwpolyline_vertex *vertices;
    struct dxf_lwpolyline_vertex *tail_vertex;
};

struct dxf_arc {
    struct dxf_entity header;
    double x;
    double y;
    double z;
    double r;
    double angle_start;
    double angle_end;
};

struct dxf_insert {
    struct dxf_entity header;
    double x;
    double y;
    double z;
    double x_scale;
    double y_scale;
    double z_scale;
    double angle;
    int column_count;
    int row_count;
    double column_spacing;
    double row_spacing;
};

#ifdef __cplusplus
extern "C" {
#endif
    
int dxf_init(struct dxf* const dxf, size_t pool_size);
int dxf_free(struct dxf* const dxf);
struct dxf_container* dxf_add_container(struct dxf* const dxf, const char *name, 
                                        struct dxf_layer* parent_layer, int type);
struct dxf_container* dxf_get_container(struct dxf* const dxf, const char *name, int type);

int dxf_add_entity(struct dxf* const dxf, const char* container_name,
                    struct dxf_entity* entity, int behaviour);
void* dxf_alloc_binary(struct dxf* const dxf, size_t size);
char* dxf_alloc_string(struct dxf* const dxf, size_t len);
struct dxf_entity* dxf_alloc_entity(struct dxf* const dxf, int entity_type);

#define dxf_add_layer(dxf, name) dxf_add_container(dxf, name, NULL, DXF_LAYER)
#define dxf_add_block(dxf, name, layer) dxf_add_container(dxf, name, layer, DXF_BLOCK)
#define dxf_get_layer(dxf, name) dxf_get_container(dxf, name, DXF_LAYER)
#define dxf_get_block(dxf, name) dxf_get_container(dxf, name, DXF_BLOCK)

#ifdef __cplusplus
}
#endif

#endif /* __DXF_H__ */
