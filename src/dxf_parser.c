#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dxf_parser.h"
#include "dbgprint.h"

static int skip_until_lexer_tag(struct dxf_lexer_desc* const lexer_desc, int tag_expected);
static int parse_point(struct dxf_parser_desc* const parser_desc);

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
    
    dbgprint("dxf_parser: Point entity ");
    
    while (dxf_lexer_get_token(lexer_desc) == 0) {
        switch (token->tag) {
            case DXF_ENTITY_TYPE:
                dxf_lexer_unget_token(lexer_desc);
                dbgprint("\ndxf_parser: End of point entity. \n");
                return 0;
            case DXF_LAYER_NAME:
                dbgprint("layer=%s ", token->value.str);
                break;
            case DXF_X:
                dbgprint("x=%f ", token->value.f);
                break;
            case DXF_Y:
                dbgprint("y=%f ", token->value.f);
                break;
            case DXF_Z:
                dbgprint("z=%f ", token->value.f);
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
            else {
                dbgprint("dxf_parser: Skipping entity type %s \n", lexer_desc->token.value.str);
            }
        }
    }
    
    return 0;
}
