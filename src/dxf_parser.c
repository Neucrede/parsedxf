#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dxf_parser.h"
#include "hashtab.h"
#include "dbgprint.h"

static const char *str_point = "POINT";
static const char *str_line = "LINE";
static const char *str_circle = "CIRCLE";
static const char *str_lwpolyline = "LWPOLYLINE";
static const char *str_block = "BLOCK";
static const char *str_endblk = "ENDBLK";
static const char *str_endsec = "ENDSEC";
static const char *str_endtab = "ENDTAB";
static const char *str_seqend = "SEQEND";

typedef int(*pfn_parser_t)(struct dxf_parser_desc* const);
static struct hashtable parsers;

static unsigned int str_hash(const char **psz);
static int str_cmp(const char **psz1, const char **psz2);
static int register_parser(const char **entity_name, pfn_parser_t parser);
static int dummy_parser_hook(struct dxf_entity* entity);
static int parse_endxxx(struct dxf_parser_desc* const parser_desc);
static int parse_point(struct dxf_parser_desc* const parser_desc);
static int parse_line(struct dxf_parser_desc* const parser_desc);
static int parse_circle(struct dxf_parser_desc* const parser_desc);
static int parse_lwpolyline(struct dxf_parser_desc* const parser_desc);
static int parse_block(struct dxf_parser_desc* const parser_desc);
static int parse_blocks(struct dxf_parser_desc* const parser_desc);
static int parse_entities(struct dxf_parser_desc* const parser_desc);

#define DXF_PARSER_ENTITY_PARSER_COMMON_ACTIONS(parser_desc, lexer_desc, token, entity, entity_type) \
    case DXF_ENTITY_TYPE: \
        dxf_lexer_unget_token(lexer_desc); \
        dbgprint("dxf_parser: End of " #entity " entity. \n"); \
        if (parser_desc->target_layer != NULL) { \
            dxf_add_entity(dxf, parser_desc->target_layer->name, (struct dxf_entity*)entity, DXF_ADD_ENTITY_TO_LAYER); \
        } \
        if (parser_desc->target_block != NULL) { \
            dxf_add_entity(dxf, parser_desc->target_block->name, (struct dxf_entity*)entity, DXF_ADD_ENTITY_TO_BLOCK); \
        } \
        parser_desc->entity_after_parse_hooks[entity_type]((struct dxf_entity*)entity); \
        return 0; \
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

static int register_parser(const char **entity_name, pfn_parser_t parser)
{
    return hashtable_put(&parsers, (void*)entity_name, HASHTABLE_COPY_VALUE, sizeof(char*),
        &parser, HASHTABLE_COPY_VALUE, sizeof(pfn_parser_t));
}

static int dummy_parser_hook(struct dxf_entity* entity)
{
    (void)entity;
    return 0;
}

static int parse_endxxx(struct dxf_parser_desc* const parser_desc)
{
    (void)parser_desc;
    return ((((( 1 )))));
}

static int parse_point(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    struct dxf* const dxf = parser_desc->dxf;
    struct dxf_point* point;
    
    dbgprint("dxf_parser: Point entity \n");
    
    if ((point = (struct dxf_point*)dxf_alloc_entity(dxf, DXF_POINT)) == NULL) {
        return -1;
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            DXF_PARSER_ENTITY_PARSER_COMMON_ACTIONS(parser_desc, lexer_desc, token, point, DXF_POINT);
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
    
    dbgprint("dxf_parser: Line entity \n");
    
    if ((line = (struct dxf_line*)dxf_alloc_entity(dxf, DXF_LINE)) == NULL) {
        return -1;
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            DXF_PARSER_ENTITY_PARSER_COMMON_ACTIONS(parser_desc, lexer_desc, token, line, DXF_LINE);
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
    int number_of_coord_values_read = 0;
    
    dbgprint("dxf_parser: Circle entity \n");
    
    if ((circle = (struct dxf_circle*)dxf_alloc_entity(dxf, DXF_CIRCLE)) == NULL) {
        return -1;
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            DXF_PARSER_ENTITY_PARSER_COMMON_ACTIONS(parser_desc, lexer_desc, token, circle, DXF_CIRCLE);
            case DXF_X:
                dbgprint("x=%f \n", token->value.f);
                circle->x = token->value.f;
                ++number_of_coord_values_read;
                break;
            case DXF_Y:
                dbgprint("y=%f \n", token->value.f);
                circle->y = token->value.f;
                ++number_of_coord_values_read;
                break;
            case DXF_Z:
                dbgprint("z=%f \n", token->value.f);
                circle->z = token->value.f;
                ++number_of_coord_values_read;
                break;
            case DXF_FLOAT:
                /* Z coordinate value could be omitted. */
                if (number_of_coord_values_read >= 2) {
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
    
    dbgprint("dxf_parser: LwPolyline entity \n");
    
    if ((lwpolyline = (struct dxf_lwpolyline*)dxf_alloc_entity(dxf, DXF_LWPOLYLINE)) == NULL) {
        return -1;
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            DXF_PARSER_ENTITY_PARSER_COMMON_ACTIONS(parser_desc, lexer_desc, token, lwpolyline, DXF_LWPOLYLINE);
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
                    return -1;
                }
                dbgprint("dxf_parser: Allocated space for lwpolyline vertex @0x%lx \n", 
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

static int parse_block(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    struct dxf* const dxf = parser_desc->dxf;
    const pfn_parser_t *pfn_parser;
    int parser_return_value = -1;

    parser_desc->target_layer = NULL;
    parser_desc->target_block = NULL;

    dbgprint("dxf_parser: Block \n");

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
                            dbgprint("dxf_parser: End of block. \n");
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
                    if (dxf_add_block(dxf, token->value.str) == NULL) {
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
                        dbgprint("dxf_parser: End of BLOCKS section. \n");
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
                        dbgprint("dxf_parser: End of ENTITIES section. \n");
                        return 0;
                    }
                    break;
                default:
                    errprint("dxf_parser: Stopped after an error occured when parsing an entity. \n");
                    return -1;
            }
        }
    }

    return 0;
}

int dxf_parser_init()
{
    if (hashtable_create(&parsers, 0, 0, 0, (pfn_hash_t)str_hash, (pfn_keycmp_t)str_cmp) != 0) {
        errprint("dxf_parser: hashtable_init() failed. \n");
        return -1;
    }

    register_parser(&str_point, parse_point);
    register_parser(&str_line, parse_line);
    register_parser(&str_circle, parse_circle);
    register_parser(&str_lwpolyline, parse_lwpolyline);

    register_parser(&str_block, parse_block);
    register_parser(&str_endblk, parse_endxxx);
    register_parser(&str_endsec, parse_endxxx);
    register_parser(&str_endtab, parse_endxxx);
    register_parser(&str_seqend, parse_endxxx);

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
        parser_desc->entity_after_parse_hooks[i] = dummy_parser_hook;
    }
    
    return 0;
}

int dxf_parser_set_entity_after_parse_hook(struct dxf_parser_desc* const parser_desc,
                                            int entity_type,
                                            pfn_entity_after_parse_hook_t hook)
{
    if ((entity_type < DXF_ENTITY_TYPE_START) || (entity_type > DXF_ENTITY_TYPE_END)) {
        return -1;
    }

    parser_desc->entity_after_parse_hooks[entity_type] = hook;
    return 0;
}

int dxf_parser_parse(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;

    while (dxf_lexer_skip_to(lexer_desc, DXF_BLOCK_NAME) == 0) {
        if (strcmp(lexer_desc->token.value.str, "BLOCKS") == 0) {
            break;
        }
    }

    dbgprint("dxf_parser: Parsing BLOCKS section. \n");
    if (parse_blocks(parser_desc) != 0) {
        return -1;
    }

    while (dxf_lexer_skip_to(lexer_desc, DXF_BLOCK_NAME) == 0) {
        if (strcmp(lexer_desc->token.value.str, "ENTITIES") == 0) {
            break;
        }
    }

    dbgprint("dxf_parser: Parsing ENTITIES section. \n");
    if (parse_entities(parser_desc) != 0) {
        return -1;
    }
    
    return 0;
}
