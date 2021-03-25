#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dxf_parser.h"
#include "dbgprint.h"

static int skip_until_lexer_tag(struct dxf_lexer_desc* const lexer_desc, int tag_expected);
static int parse_point(struct dxf_parser_desc* const parser_desc);
static int parse_line(struct dxf_parser_desc* const parser_desc);

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
            else {
                dbgprint("\ndxf_parser: Skipping entity type %s \n", lexer_desc->token.value.str);
            }
        }
    }
    
    return 0;
}
