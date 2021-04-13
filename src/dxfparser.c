#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dxfparser.h"
#include "hashtab.h"
#include "dbgprint.h"

static int initialized = 0;

static struct hashtable parsers;
typedef int(*pfn_parser_t)(struct dxf_parser_desc* const);

static const char *str_point = "POINT";
static const char *str_line = "LINE";
static const char *str_circle = "CIRCLE";
static const char *str_lwpolyline = "LWPOLYLINE";
static const char *str_arc = "ARC";
static const char *str_insert = "INSERT";
static const char *str_ellipse = "ELLIPSE";
static const char *str_hatch = "HATCH";
static const char *str_mtext = "MTEXT";
static const char *str_text = "TEXT";
static const char *str_solid = "SOLID";
static const char *str_spline = "SPLINE";
static const char *str_block = "BLOCK";
static const char *str_section = "SECTION";
static const char *str_header = "HEADER";
static const char *str_classes = "CLASSES";
static const char *str_tables = "TABLES";
static const char *str_blocks = "BLOCKS";
static const char *str_entities = "ENTITIES";
static const char *str_objects = "OBJECTS";
static const char *str_thumbnailimage = "THUMBNAILIMAGE";
static const char *str_endblk = "ENDBLK";
static const char *str_endsec = "ENDSEC";
static const char *str_endtab = "ENDTAB";
static const char *str_seqend = "SEQEND";
static const char *str_eof = "EOF";

static unsigned int str_hash(const char **psz);
static int str_cmp(const char **psz1, const char **psz2);
static int register_parser(const char **object_name, pfn_parser_t parser);
static int dummy_parser_hook(struct dxf_entity* entity);
static int parse_endxxx(struct dxf_parser_desc* const parser_desc);
static int parse_point(struct dxf_parser_desc* const parser_desc);
static int parse_line(struct dxf_parser_desc* const parser_desc);
static int parse_circle(struct dxf_parser_desc* const parser_desc);
static int parse_lwpolyline(struct dxf_parser_desc* const parser_desc);
static int parse_arc(struct dxf_parser_desc* const parser_desc);
static int parse_insert(struct dxf_parser_desc* const parser_desc);
static int parse_block(struct dxf_parser_desc* const parser_desc);
static int parse_blocks(struct dxf_parser_desc* const parser_desc);
static int parse_entities(struct dxf_parser_desc* const parser_desc);

#define DXF_ENTITY_PARSER_ACTION_ON_ENTITY_TYPE(parser_desc, lexer_desc, token, entity, entity_type) \
    case DXF_ENTITY_TYPE: \
        dxf_lexer_unget_token(lexer_desc); \
        dbgprint("dxfparser: End of " #entity " entity. \n"); \
        if (parser_desc->target_layer != NULL) { \
                dxf_add_entity(dxf, parser_desc->target_layer->name, (struct dxf_entity*)entity, DXF_ADD_ENTITY_TO_LAYER); \
        } \
        if (parser_desc->target_block != NULL) { \
                dxf_add_entity(dxf, parser_desc->target_block->name, (struct dxf_entity*)entity, DXF_ADD_ENTITY_TO_BLOCK); \
        } \
        parser_desc->entity_post_parse_hooks[entity_type]((struct dxf_entity*)entity); \
        return 0; \

#define DXF_ENTITY_PARSER_ACTION_ON_LAYER_NAME(parser_desc, lexer_desc, token, entity, entity_type) \
    case DXF_LAYER_NAME: \
        if (parser_desc->target_layer == NULL) { \
            dbgprint("layer=%s \n", token->value.str); \
            dxf_add_entity(dxf, token->value.str, (struct dxf_entity*)entity, DXF_ADD_ENTITY_TO_LAYER); \
        } \
        break; \

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

static int register_parser(const char **object_name, pfn_parser_t parser)
{
    return hashtable_put(&parsers, 
                        (void*)object_name, sizeof(char*),
                        &parser, sizeof(pfn_parser_t));
}

static int dummy_parser_hook(struct dxf_entity* entity)
{
    (void)entity;
    return 0;
}

static int parse_endxxx(struct dxf_parser_desc* const parser_desc)
{
    (void)parser_desc;
    
    /* Returning 1 indicates that we have met an end-of-object tag. */
    return ((((( 1 )))));
}

static int parse_point(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    struct dxf* const dxf = parser_desc->dxf;
    struct dxf_point* point;
    
    dbgprint("dxfparser: Point entity \n");
    
    if ((point = (struct dxf_point*)dxf_alloc_entity(dxf, DXF_POINT)) == NULL) {
        return -1;
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            DXF_ENTITY_PARSER_ACTION_ON_ENTITY_TYPE(parser_desc, lexer_desc, token, point, DXF_POINT);
            DXF_ENTITY_PARSER_ACTION_ON_LAYER_NAME(parser_desc, lexer_desc, token, point, DXF_POINT);
            case DXF_X:
                dbgprint("x=%f \n", token->value.f);
                point->x = token->value.f;
                break;
            case DXF_Y:
                dbgprint("y=%f \n", token->value.f);
                point->y = token->value.f;
                break;
            case DXF_Z:
                dbgprint("z=%f \n", token->value.f);
                point->z = token->value.f;
                break;
            default:
                break;
        }
    }
    
    return -1;
}

static int parse_line(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    struct dxf* const dxf = parser_desc->dxf;
    struct dxf_line* line;
    
    dbgprint("dxfparser: Line entity \n");
    
    if ((line = (struct dxf_line*)dxf_alloc_entity(dxf, DXF_LINE)) == NULL) {
        return -1;
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            DXF_ENTITY_PARSER_ACTION_ON_ENTITY_TYPE(parser_desc, lexer_desc, token, line, DXF_LINE);
            DXF_ENTITY_PARSER_ACTION_ON_LAYER_NAME(parser_desc, lexer_desc, token, line, DXF_LINE);
            case DXF_X:
                if (token->group_code == 10) {
                    dbgprint("x1=%f \n", token->value.f);
                    line->x1 = token->value.f;
                }
                else {
                    dbgprint("x2=%f \n", token->value.f);
                    line->x2 = token->value.f;
                }
                break;
            case DXF_Y:
                if (token->group_code == 20) {
                    dbgprint("y1=%f \n", token->value.f);
                    line->y1 = token->value.f;
                }
                else {
                    dbgprint("y2=%f \n", token->value.f);
                    line->y2 = token->value.f;
                }
                break;
            case DXF_Z:
                if (token->group_code == 30) {
                    dbgprint("z1=%f \n", token->value.f);
                    line->z1 = token->value.f;
                }
                else {
                    dbgprint("z2=%f \n", token->value.f);
                    line->z2 = token->value.f;
                }
                break;
            default:
                break;
        }
    }

    return -1;
}

static int parse_circle(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    struct dxf* const dxf = parser_desc->dxf;
    struct dxf_circle* circle;
    
    dbgprint("dxfparser: Circle entity \n");
    
    if ((circle = (struct dxf_circle*)dxf_alloc_entity(dxf, DXF_CIRCLE)) == NULL) {
        return -1;
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            DXF_ENTITY_PARSER_ACTION_ON_ENTITY_TYPE(parser_desc, lexer_desc, token, circle, DXF_CIRCLE);
            DXF_ENTITY_PARSER_ACTION_ON_LAYER_NAME(parser_desc, lexer_desc, token, circle, DXF_CIRCLE);
            case DXF_X:
                dbgprint("x=%f \n", token->value.f);
                circle->x = token->value.f;
                break;
            case DXF_Y:
                dbgprint("y=%f \n", token->value.f);
                circle->y = token->value.f;
                break;
            case DXF_Z:
                dbgprint("z=%f \n", token->value.f);
                circle->z = token->value.f;
                break;
            case DXF_FLOAT:
                if (token->group_code == 40) {
                    dbgprint("r=%f \n", token->value.f);
                    circle->r = token->value.f;
                }
                break;
            default:
                break;
        }
    }
    
    return -1;
}

static int parse_lwpolyline(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    struct dxf* const dxf = parser_desc->dxf;

    struct dxf_lwpolyline* lwpolyline;
    struct dxf_lwpolyline_vertex *vertex = NULL;
    size_t number_of_vertices = 0;
    size_t number_of_vertices_read = 0;
    
    dbgprint("dxfparser: LwPolyline entity \n");
    
    if ((lwpolyline = (struct dxf_lwpolyline*)dxf_alloc_entity(dxf, DXF_LWPOLYLINE)) == NULL) {
        return -1;
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            DXF_ENTITY_PARSER_ACTION_ON_ENTITY_TYPE(parser_desc, lexer_desc, token, lwpolyline, DXF_LWPOLYLINE);
            DXF_ENTITY_PARSER_ACTION_ON_LAYER_NAME(parser_desc, lexer_desc, token, lwpolyline, DXF_LWPOLYLINE);
            case DXF_INTEGER:
                if (token->group_code == 70) {
                    dbgprint("flag=%d \n", token->value.i);
                    lwpolyline->flag = token->value.i;
                }
                break;
            case DXF_INTEGER32:
                if (token->group_code == 90) {
                    dbgprint("number_of_vertices=%d \n", token->value.i);
                    number_of_vertices = token->value.i;
                }
                break;
            case DXF_X:
                if (number_of_vertices_read > number_of_vertices) {
                    dbgprint("Unexpected token, skipping... \n");
                    break;
                }
                dbgprint("x=%f \n", token->value.f);
                if ((vertex = dxf_alloc_binary(dxf, sizeof(struct dxf_lwpolyline_vertex))) == NULL) {
                    errprint("dxfparser: parse_lwpolyline(): Failed to allocate space for vertices. \n");
                    return -1;
                }
                dbgprint("dxfparser: parse_lwpolyline(): Allocated space for lwpolyline vertex @0x%lx \n", 
                        (unsigned long)vertex);
                vertex->next = NULL;
                vertex->x = token->value.f;
                vertex->y = vertex->z = vertex->bulge = 0.0;
                if (lwpolyline->tail_vertex != NULL) {
                    lwpolyline->tail_vertex->next = vertex;
                    lwpolyline->tail_vertex = vertex;
                }
                else {
                    lwpolyline->tail_vertex = lwpolyline->vertices = vertex;
                }
                ++number_of_vertices_read;
                break;
            case DXF_Y:
                if ((number_of_vertices_read > number_of_vertices) || (vertex == NULL)) {
                    dbgprint("Unexpected token, skipping... \n");
                    break;
                }
                dbgprint("y=%f \n", token->value.f);
                vertex->y = token->value.f;
                break;
            case DXF_Z:
                if ((number_of_vertices_read > number_of_vertices) || (vertex == NULL)) {
                    dbgprint("Unexpected token, skipping... \n");
                    break;
                }
                dbgprint("z=%f \n", token->value.f);
                vertex->z = token->value.f;
                break;
            case DXF_FLOAT:
                if ((number_of_vertices_read > number_of_vertices) || (vertex == NULL)) {
                    dbgprint("Unexpected or unwanted token, skipping... \n");
                    break;
                }
                if (token->group_code == 42) {
                    dbgprint("bulge=%f \n", token->value.f);
                    vertex->bulge = token->value.f;
                }
                break;
            default:
                break;
        }
    }
    
    return -1;
}

static int parse_arc(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    struct dxf* const dxf = parser_desc->dxf;
    struct dxf_arc* arc;
    
    dbgprint("dxfparser: Arc entity \n");
    
    if ((arc = (struct dxf_arc*)dxf_alloc_entity(dxf, DXF_ARC)) == NULL) {
        return -1;
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            DXF_ENTITY_PARSER_ACTION_ON_ENTITY_TYPE(parser_desc, lexer_desc, token, arc, DXF_ARC);
            DXF_ENTITY_PARSER_ACTION_ON_LAYER_NAME(parser_desc, lexer_desc, token, arc, DXF_ARC);
            case DXF_X:
                dbgprint("x=%f \n", token->value.f);
                arc->x = token->value.f;
                break;
            case DXF_Y:
                dbgprint("y=%f \n", token->value.f);
                arc->y = token->value.f;
                break;
            case DXF_Z:
                dbgprint("z=%f \n", token->value.f);
                arc->z = token->value.f;
                break;
            case DXF_FLOAT:
                if (token->group_code == 40) {
                    dbgprint("r=%f \n", token->value.f);
                    arc->r = token->value.f;
                }
                break;
            case DXF_ANGLE:
                if (token->group_code == 50) {
                    dbgprint("angle_start=%f \n", token->value.f);
                    arc->angle_start = token->value.f;
                }
                else if (token->group_code == 51) {
                    dbgprint("angle_end=%f \n", token->value.f);
                    arc->angle_end = token->value.f;
                }
                break;
            default:
                break;
        }
    }
    
    return -1;
}

static int parse_insert(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    struct dxf* const dxf = parser_desc->dxf;

    struct dxf_insert *insert;
    struct dxf_layer *layer_of_block = NULL;

    dbgprint("dxfparser: Insert entity \n");

    if ((insert = (struct dxf_insert*)dxf_alloc_entity(dxf, DXF_INSERT)) == NULL) {
        return -1;
    }

    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            DXF_ENTITY_PARSER_ACTION_ON_ENTITY_TYPE(parser_desc, lexer_desc, token, insert, DXF_INSERT);
            case DXF_BLOCK_NAME:
                dbgprint("blockname=%s \n", token->value.str);
                if ((insert->header.block = dxf_get_block(dxf, token->value.str)) != NULL) {
                    layer_of_block = insert->header.block->parent;
                    if (layer_of_block != NULL) {
                        dxf_add_entity(dxf, layer_of_block->name,
                            (struct dxf_entity*)insert, DXF_ADD_ENTITY_TO_LAYER);
                    }
                    else {
                        errprint("dxf_parser: WARNING: Block %s did not attached to a layer. \n", token->value.str);
                    }
                }
                else {
                    errprint("dxf_parser: Block %s was not found. \n", token->value.str);
                }
                break;
            case DXF_X:
                dbgprint("x=%f \n", token->value.f);
                insert->x = token->value.f;
                break;
            case DXF_Y:
                dbgprint("y=%f \n", token->value.f);
                insert->y = token->value.f;
                break;
            case DXF_Z:
                dbgprint("z=%f \n", token->value.f);
                insert->z = token->value.f;
                break;
            case DXF_FLOAT:
                switch (token->group_code) {
                    case 41:
                        dbgprint("x_scale=%f \n", token->value.f);
                        insert->x_scale = token->value.f;
                        break;
                    case 42:
                        dbgprint("y_scale=%f \n", token->value.f);
                        insert->y_scale = token->value.f;
                        break;
                    case 43:
                        dbgprint("z_scale=%f \n", token->value.f);
                        insert->z_scale = token->value.f;
                        break;
                    case 44:
                        dbgprint("column_spacing=%f \n", token->value.f);
                        insert->column_spacing = token->value.f;
                        break;
                    case 45:
                        dbgprint("row_spacing=%f \n", token->value.f);
                        insert->row_spacing = token->value.f;
                        break;
                    default:
                        break;
                }
                break;
            case DXF_INTEGER:
                switch (token->group_code) {
                    case 70:
                        dbgprint("column_count=%d \n", token->value.i);
                        insert->column_count = token->value.i;
                        break;
                    case 71:
                        dbgprint("row_count=%d \n", token->value.i);
                        insert->row_count = token->value.i;
                        break;
                    default:
                        break;
                }
                break;
            case DXF_ANGLE:
                dbgprint("angle=%f \n", token->value.f);
                insert->angle = token->value.f;
                break;
            default:
                break;
        }
    }

    return 0;
}

static int parse_block(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    struct dxf* const dxf = parser_desc->dxf;
    const pfn_parser_t *pfn_parser;
    int parser_return_value = -1;

    parser_desc->target_layer = NULL;
    parser_desc->target_block = NULL;

    dbgprint("dxfparser: Block \n");

    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            case DXF_ENTITY_TYPE:
                if ((pfn_parser = hashtable_get(&parsers, &(token->value.str))) != NULL) {
                    if (*pfn_parser != NULL) {
                        parser_return_value = (*pfn_parser)(parser_desc);
                    }
                }
                else {
                    errprint("dxf_parser: Skipping entity type %s \n", token->value.str);
                    continue;
                }

                switch (parser_return_value) {
                    case 0:
                        break;
                    case 1:
                        if (strcmp(token->value.str, str_endblk) == 0) {
                            parser_desc->target_layer = NULL;
                            parser_desc->target_block = NULL;
                            dbgprint("dxfparser: End of block. \n");
                            return 0;
                        }
                        break;
                    default:
                        errprint("dxf_parser: Stopped after an error occured when parsing an entity. \n");
                        parser_desc->target_layer = NULL;
                        parser_desc->target_block = NULL;
                        return -1;
                }
                break;
            case DXF_LAYER_NAME:
                dbgprint("layer=%s \n", token->value.str);
                if ((parser_desc->target_layer = dxf_get_layer(dxf, token->value.str)) == NULL) {
                    if (dxf_add_layer(dxf, token->value.str) == NULL) {
                        return -1;
                    }
                    parser_desc->target_layer = dxf_get_layer(dxf, token->value.str);
                }
                break;
            case DXF_BLOCK_NAME:
                dbgprint("block=%s \n", token->value.str);
                if ((parser_desc->target_block = dxf_get_block(dxf, token->value.str)) == NULL) {
                    if (dxf_add_block(dxf, token->value.str, parser_desc->target_layer) == NULL) {
                        return -1;
                    }
                    parser_desc->target_block = dxf_get_block(dxf, token->value.str);
                }
                break;
            case DXF_INTEGER:
                if (token->group_code == 70) {
                    if (parser_desc->target_block != NULL) {
                        dbgprint("flag=%d \n", token->value.i);
                        parser_desc->target_block->flag = token->value.i;
                    }
                }
                break;
            case DXF_X:
                dbgprint("x=%f \n", token->value.f);
                if (parser_desc->target_block != NULL) {
                    parser_desc->target_block->x = token->value.f;
                }
                break;
            case DXF_Y:
                dbgprint("y=%f \n", token->value.f);
                if (parser_desc->target_block != NULL) {
                    parser_desc->target_block->y = token->value.f;
                }
                break;
            case DXF_Z:
                dbgprint("z=%f \n", token->value.f);
                if (parser_desc->target_block != NULL) {
                    parser_desc->target_block->z = token->value.f;
                }
                break;
            default:
                break;
        }
    }

    return 0;
}

static int parse_blocks(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    const pfn_parser_t *pfn_parser;
    int parser_return_value = -1;

    dbgprint("dxfparser: Parsing BLOCKS section. \n");

    parser_desc->target_layer = NULL;
    parser_desc->target_block = NULL;

    while (dxf_lexer_get_token(lexer_desc) == 0) {
        if (token->tag == DXF_ENTITY_TYPE) {
            if ((pfn_parser = hashtable_get(&parsers, &(token->value.str))) != NULL) {
                if (*pfn_parser != NULL) {
                    parser_return_value = (*pfn_parser)(parser_desc);
                }
            }
            else {
                errprint("dxf_parser: Skipping object type %s \n", token->value.str);
                continue;
            }

            switch (parser_return_value) {
                case 0:
                    break;
                case 1:
                    if (strcmp(token->value.str, str_endsec) == 0) {
                        parser_desc->target_layer = NULL;
                        parser_desc->target_block = NULL;
                        dbgprint("dxfparser: End of BLOCKS section. \n");
                        return 0;
                    }
                    break;
                default:
                    errprint("dxf_parser: Stopped after an error occured when parsing an object. \n");
                    parser_desc->target_layer = NULL;
                    parser_desc->target_block = NULL;
                    return -1;
            }
        }
    }

    return 0;
}

static int parse_entities(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    const pfn_parser_t *pfn_parser;
    int parser_return_value = -1;

    dbgprint("dxfparser: Parsing ENTITIES section. \n");

    parser_desc->target_layer = NULL;
    parser_desc->target_block = NULL;

    while (dxf_lexer_get_token(lexer_desc) == 0) {
        if (token->tag == DXF_ENTITY_TYPE) {
            if ((pfn_parser = hashtable_get(&parsers, &(token->value.str))) != NULL) {
                if (*pfn_parser != NULL) {
                    parser_return_value = (*pfn_parser)(parser_desc);
                }
            }
            else {
                errprint("dxf_parser: Skipping entity type %s \n", token->value.str);
                continue;
            }

            switch (parser_return_value) {
                case 0:
                    break;
                case 1:
                    if (strcmp(token->value.str, str_endsec) == 0) {
                        dbgprint("dxfparser: End of ENTITIES section. \n");
                        return 0;
                    }
                    break;
                default:
                    errprint("dxfparser: Stopped after an error occured when parsing an entity. \n");
                    return -1;
            }
        }
    }

    return 0;
}

int dxf_parser_init()
{
    if (initialized == 1) {
        return 0;
    }
    
    if (hashtable_create(&parsers, 0, 0, 0, HASHTABLE_COPY_VALUE,
        HASHTABLE_COPY_VALUE, (pfn_hash_t)str_hash, (pfn_keycmp_t)str_cmp, NULL) != 0)
    {
        errprint("dxf_parser: hashtable_init() failed. \n");
        return -1;
    }

    register_parser(&str_entities, parse_entities);
    register_parser(&str_point, parse_point);
    register_parser(&str_line, parse_line);
    register_parser(&str_circle, parse_circle);
    register_parser(&str_lwpolyline, parse_lwpolyline);
    register_parser(&str_arc, parse_arc);
    register_parser(&str_insert, parse_insert);
    
    register_parser(&str_blocks, parse_blocks);
    register_parser(&str_block, parse_block);
    
    register_parser(&str_endblk, parse_endxxx);
    register_parser(&str_endsec, parse_endxxx);
    register_parser(&str_endtab, parse_endxxx);
    register_parser(&str_seqend, parse_endxxx);
    register_parser(&str_eof, parse_endxxx);

    initialized = 1;
    return 0;
}

int dxf_parser_init_desc(struct dxf_parser_desc* const parser_desc,
                        struct dxf_lexer_desc* const lexer_desc,
                        struct dxf* const dxf)
{
    int i;

    parser_desc->lexer_desc = lexer_desc;
    parser_desc->dxf = dxf;
    parser_desc->target_block = NULL;
    parser_desc->target_layer = NULL;

    for (i = 0; i < DXF_ENTITY_TYPES_COUNT; ++i) {
        parser_desc->entity_post_parse_hooks[i] = dummy_parser_hook;
    }
    
    return 0;
}

int dxf_parser_set_entity_post_parse_hook(struct dxf_parser_desc* const parser_desc,
                                            int entity_type,
                                            pfn_entity_post_parse_hook_t hook)
{
    if ((entity_type < DXF_ENTITY_TYPE_START) || (entity_type > DXF_ENTITY_TYPE_END)) {
        return -1;
    }

    parser_desc->entity_post_parse_hooks[entity_type] = hook;
    return 0;
}

int dxf_parser_parse(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    const pfn_parser_t *pfn_parser;
    int parser_return_value = -1;

    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
        case DXF_BLOCK_NAME:
        case DXF_ENTITY_TYPE:
            if ((pfn_parser = hashtable_get(&parsers, &(token->value.str))) != NULL) {
                if (*pfn_parser != NULL) {
                    parser_return_value = (*pfn_parser)(parser_desc);
                }
            }
            else {
                continue;
            }

            switch (parser_return_value) {
                case 0:
                    break;
                case 1:
                    if (strcmp(token->value.str, str_eof) == 0) {
                        dbgprint("dxfparser: Reached EOF. \n");
                        return 0;
                    }
                    break;
                default:
                    errprint("dxfparser: dxf_parser_parse(): Parser stopped on error. \n");
                    return -1;
            }
        default:
            break;
        }
    }

    return 0;
}
