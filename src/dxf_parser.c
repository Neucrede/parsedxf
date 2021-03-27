#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dxf_parser.h"
#include "dbgprint.h"

static int skip_until_lexer_tag(struct dxf_lexer_desc* const lexer_desc, int tag_expected);
static int parse_point(struct dxf_parser_desc* const parser_desc);
static int parse_line(struct dxf_parser_desc* const parser_desc);
static int parse_circle(struct dxf_parser_desc* const parser_desc);
static int parse_lwpolyline(struct dxf_parser_desc* const parser_desc);

static int skip_until_lexer_tag(struct dxf_lexer_desc* const lexer_desc, int tag_expected)
{
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        if (lexer_desc->token.tag == tag_expected) {
            return 0;
        }
    }
    
    return -1;
}

static int parse_point(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    struct dxf* const dxf = parser_desc->dxf;
    struct dxf_point* point;
    
    dbgprint("\ndxf_parser: Point entity ");
    
    if ((point = (struct dxf_point*)dxf_alloc_entity(dxf, DXF_POINT)) == NULL) {
        return -1;
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            case DXF_ENTITY_TYPE:
                dxf_lexer_unget_token(lexer_desc);
                dbgprint("\ndxf_parser: End of point entity. \n");
                return 0;
            case DXF_LAYER_NAME:
                dbgprint("\nlayer=%s ", token->value.str);
                dxf_add_entity(dxf, token->value.str, (struct dxf_entity*)point);
                break;
            case DXF_X:
                dbgprint("\nx=%f ", token->value.f);
                point->x = token->value.f;
                break;
            case DXF_Y:
                dbgprint("\ny=%f ", token->value.f);
                point->y = token->value.f;
                break;
            case DXF_Z:
                dbgprint("\nz=%f ", token->value.f);
                point->z = token->value.f;
                break;
            default:
                break;
        }
    }
    
    return 0;
}

static int parse_line(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    struct dxf* const dxf = parser_desc->dxf;
    struct dxf_line* line;
    
    dbgprint("\ndxf_parser: Line entity ");
    
    if ((line = (struct dxf_line*)dxf_alloc_entity(dxf, DXF_LINE)) == NULL) {
        return -1;
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            case DXF_ENTITY_TYPE:
                dxf_lexer_unget_token(lexer_desc);
                dbgprint("\ndxf_parser: End of line entity. \n");
                return 0;
            case DXF_LAYER_NAME:
                dbgprint("\nlayer=%s ", token->value.str);
                dxf_add_entity(dxf, token->value.str, (struct dxf_entity*)line);
                break;
            case DXF_X:
                if (token->group_code == 10) {
                    dbgprint("\nx1=%f ", token->value.f);
                    line->x1 = token->value.f;
                }
                else {
                    dbgprint("\nx2=%f ", token->value.f);
                    line->x2 = token->value.f;
                }
                break;
            case DXF_Y:
                if (token->group_code == 20) {
                    dbgprint("\ny1=%f ", token->value.f);
                    line->y1 = token->value.f;
                }
                else {
                    dbgprint("\ny2=%f ", token->value.f);
                    line->y2 = token->value.f;
                }
                break;
            case DXF_Z:
                if (token->group_code == 30) {
                    dbgprint("\nz1=%f ", token->value.f);
                    line->z1 = token->value.f;
                }
                else {
                    dbgprint("\nz2=%f ", token->value.f);
                    line->z2 = token->value.f;
                }
                break;
            default:
                break;
        }
    }
    
    return 0;
}

static int parse_circle(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    struct dxf_token* const token = &(lexer_desc->token);
    struct dxf* const dxf = parser_desc->dxf;
    struct dxf_circle* circle;
    int number_of_coord_values_read = 0;
    
    dbgprint("\ndxf_parser: Circle entity ");
    
    if ((circle = (struct dxf_circle*)dxf_alloc_entity(dxf, DXF_CIRCLE)) == NULL) {
        return -1;
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            case DXF_ENTITY_TYPE:
                dxf_lexer_unget_token(lexer_desc);
                dbgprint("\ndxf_parser: End of circle entity. \n");
                return 0;
            case DXF_LAYER_NAME:
                dbgprint("\nlayer=%s ", token->value.str);
                dxf_add_entity(dxf, token->value.str, (struct dxf_entity*)circle);
                break;
            case DXF_X:
                dbgprint("\nx=%f ", token->value.f);
                circle->x = token->value.f;
                ++number_of_coord_values_read;
                break;
            case DXF_Y:
                dbgprint("\ny=%f ", token->value.f);
                circle->y = token->value.f;
                ++number_of_coord_values_read;
                break;
            case DXF_Z:
                dbgprint("\nz=%f ", token->value.f);
                circle->z = token->value.f;
                ++number_of_coord_values_read;
                break;
            case DXF_FLOAT:
                /* Z coordinate value could be omitted. */
                if (number_of_coord_values_read >= 2) {
                    dbgprint("\nr=%f ", token->value.f);
                    circle->r = token->value.f;
                }
                break;
            default:
                break;
        }
    }
    
    return 0;
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
    
    dbgprint("\ndxf_parser: LwPolyline entity ");
    
    if ((lwpolyline = (struct dxf_lwpolyline*)dxf_alloc_entity(dxf, DXF_LWPOLYLINE)) == NULL) {
        return -1;
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            case DXF_ENTITY_TYPE:
                dxf_lexer_unget_token(lexer_desc);
                dbgprint("\ndxf_parser: End of lwpolyline entity. \n");
                if (number_of_vertices_read < number_of_vertices) {
                    dbgprint("\ndxf_parser: WARNING Actual number of vertices read " \
                                "is less than the number that it claims to have. \n");
                    lwpolyline->number_of_vertices = number_of_vertices_read;
                }
                return 0;
            case DXF_LAYER_NAME:
                dbgprint("\nlayer=%s ", token->value.str);
                dxf_add_entity(dxf, token->value.str, (struct dxf_entity*)lwpolyline);
                break;
            case DXF_INTEGER:
                if (token->group_code == 70) {
                    dbgprint("\nflag=%d ", token->value.i);
                    lwpolyline->flag = token->value.i;
                }
                break;
            case DXF_INTEGER32:
                if (token->group_code == 90) {
                    dbgprint("\nnumber_of_vertices=%d ", token->value.i);
                    number_of_vertices = token->value.i;
                }
                break;
            case DXF_X:
                if (number_of_vertices_read >= number_of_vertices) {
                    dbgprint("\nUnexpected token, skipping...");
                    break;
                }
                dbgprint("\nx=%f ", token->value.f);
                if ((vertex = dxf_alloc_binary(dxf, sizeof(struct dxf_lwpolyline_vertex))) == NULL) {
                    return -1;
                }
                dbgprint("\ndxf_parser: Allocated space for lwpolyline vertex @0x%x \n", 
                        (unsigned int)vertex);
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
                if ((number_of_vertices_read >= number_of_vertices) || (vertex == NULL)) {
                    dbgprint("\nUnexpected token, skipping...");
                    break;
                }
                dbgprint("\ny=%f ", token->value.f);
                vertex->y = token->value.f;
                break;
            case DXF_Z:
                if ((number_of_vertices_read >= number_of_vertices) || (vertex == NULL)) {
                    dbgprint("\nUnexpected token, skipping...");
                    break;
                }
                dbgprint("\nz=%f ", token->value.f);
                vertex->z = token->value.f;
                break;
            case DXF_FLOAT:
                if ((number_of_vertices_read >= number_of_vertices) || (vertex == NULL)) {
                    dbgprint("\nUnexpected or unwanted token, skipping...");
                    break;
                }
                if (token->group_code == 42) {
                    dbgprint("\nbulge=%f ", token->value.f);
                    vertex->bulge = token->value.f;
                }
                break;
            default:
                break;
        }
    }
    
    return 0;
}

int dxf_parser_init()
{
    return 0;
}

int dxf_parser_init_desc(struct dxf_parser_desc* const parser_desc,
                        struct dxf_lexer_desc* const lexer_desc,
                        struct dxf* const dxf)
{
    parser_desc->lexer_desc = lexer_desc;
    parser_desc->dxf = dxf;
    
    return 0;
}

int dxf_parser_parse(struct dxf_parser_desc* const parser_desc)
{
    struct dxf_lexer_desc* const lexer_desc = parser_desc->lexer_desc;
    
    while (skip_until_lexer_tag(lexer_desc, DXF_BLOCK_NAME) == 0) {
        if (strcmp(lexer_desc->token.value.str, "ENTITIES") == 0) {
            break;
        }
    }
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        if (lexer_desc->token.tag == DXF_ENTITY_TYPE) {
            if (strcmp(lexer_desc->token.value.str, "ENDSEC") == 0) {
                break;
            }
            else if (strcmp(lexer_desc->token.value.str, "POINT") == 0) {
                parse_point(parser_desc);
            }
            else if (strcmp(lexer_desc->token.value.str, "LINE") == 0) {
                parse_line(parser_desc);
            }
            else if (strcmp(lexer_desc->token.value.str, "CIRCLE") == 0) {
                parse_circle(parser_desc);
            }
            else if (strcmp(lexer_desc->token.value.str, "LWPOLYLINE") == 0) {
                parse_lwpolyline(parser_desc);
            }
            else {
                dbgprint("\ndxf_parser: Skipping entity type %s \n", lexer_desc->token.value.str);
            }
        }
    }
    
    return 0;
}
