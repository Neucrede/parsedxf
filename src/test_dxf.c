#include <stdlib.h>
#include <stdio.h>
#include "dxf.h"

int main(int argc, char *argv[])
{
    struct dxf dxf;
    struct dxf_entity *entity;

    if (dxf_init(&dxf, 4096) != 0) {
        printf("Failed to init dxf. \n");
        return 1;
    }

    dxf_add_layer(&dxf, "Layer1");
    dxf_add_layer(&dxf, "Layer2");

    entity = (struct dxf_entity*)dxf_pool_alloc(&dxf, sizeof(struct dxf_point));
    entity->type = 0;
    ((struct dxf_point*)entity)->x = 1.0f;
    ((struct dxf_point*)entity)->y = 2.0f;
    ((struct dxf_point*)entity)->z = 3.0f;
    dxf_add_entity(&dxf, "Layer1", entity);

    entity = (struct dxf_entity*)dxf_pool_alloc(&dxf, sizeof(struct dxf_line));
    entity->type = 0;
    ((struct dxf_line*)entity)->x1 = 1.0f;
    ((struct dxf_line*)entity)->y1 = 2.0f;
    ((struct dxf_line*)entity)->z1 = 3.0f;
    ((struct dxf_line*)entity)->x2 = 4.0f;
    ((struct dxf_line*)entity)->y2 = 5.0f;
    ((struct dxf_line*)entity)->z2 = 6.0f;
    dxf_add_entity(&dxf, "Layer2", entity);

    return 0;
}
