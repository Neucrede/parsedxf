#ifndef __DXF_PARSER_H__
#define __DXF_PARSER_H__

#include "dxf.h"
#include "dxf_lexer.h"

struct dxf_parser_desc {
    struct dxf_lexer_desc* lexer_desc;
    struct dxf* dxf;
};

#ifdef __cplusplus
extern "C" {
#endif

int dxf_parser_init();
int dxf_parser_init_desc(struct dxf_parser_desc* const parser_desc,
                        struct dxf_lexer_desc* const lexer_desc,
                        struct dxf* const dxf);
int dxf_parser_parse(struct dxf_parser_desc* const parser_desc);
    
#ifdef __cplusplus
}
#endif

#endif /* __DXF_PARSER_H__ */
