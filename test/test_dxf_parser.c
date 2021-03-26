#include <stdio.h>
#include "dxf.h"
#include "dxf_lexer.h"
#include "dxf_parser.h"

int main(int argc, char *argv[])
{
    struct dxf_lexer_desc lexer_desc;
    struct dxf_parser_desc parser_desc;
    struct dxf dxf;
    
    if (argc < 2) {
        return 1;
    }

    dxf_lexer_init();
    dxf_lexer_init_desc(&lexer_desc);
    if (dxf_lexer_open_desc(&lexer_desc, argv[1], NULL) == -1) {
        printf("Failed to open %s for mapping. \n", argv[1]);
        return 1;
    }
    
    if (dxf_init(&dxf, 0) != 0) {
        printf("Failed to initialize dxf struct.\n");
        return 1;
    }
    
    dxf_parser_init();
    dxf_parser_init_desc(&parser_desc, &lexer_desc, &dxf);
    dxf_parser_parse(&parser_desc);
    
    dxf_lexer_close_desc(&lexer_desc, 0);
    
    return 0;
}
