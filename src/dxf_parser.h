#ifndef __DXF_PARSER_H__
#define __DXF_PARSER_H__

#include "dxf.h"
#include "dxf_lexer.h"

typedef int(*pfn_entity_post_parse_hook_t)(struct dxf_entity*);

struct dxf_parser_desc {
    struct dxf_lexer_desc* lexer_desc;
    struct dxf* dxf;
    struct dxf_block* target_block;
    struct dxf_layer* target_layer;
    pfn_entity_post_parse_hook_t entity_post_parse_hooks[DXF_ENTITY_TYPES_COUNT];
};

#ifdef __cplusplus
extern "C" {
#endif

int dxf_parser_init();
int dxf_parser_init_desc(struct dxf_parser_desc* const parser_desc,
                        struct dxf_lexer_desc* const lexer_desc,
                        struct dxf* const dxf);
int dxf_parser_set_entity_post_parse_hook(
                        struct dxf_parser_desc* const parser_desc,
                        int entity_type,
                        pfn_entity_post_parse_hook_t hook);
int dxf_parser_parse(struct dxf_parser_desc* const parser_desc);
    
#ifdef __cplusplus
}
#endif

#endif /* __DXF_PARSER_H__ */
