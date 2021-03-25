#include <stdio.h>
#include <string.h>
#include "dxf_lexer.h"

int main() {
    struct dxf_lexer_desc desc;

    dxf_lexer_init();
    dxf_lexer_init_desc(&desc);
    dxf_lexer_open_desc(&desc, "./aa.dxf", NULL);

    printf("%d\n", strlen(desc.buf));
    
    while (1) {
        if (dxf_lexer_get_token(&desc) != 0) {
            break;
        }
    }

    dxf_lexer_close_desc(&desc, 1);

    return 0;
}
