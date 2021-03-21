#include <stdio.h>
#include "dxf_lexer.h"

int main() {
    struct dxf_lexer_desc desc;

    dxf_lexer_init();
    dxf_lexer_open_desc(&desc, "./aa.dxf");

    printf(desc.buf);

    dxf_lexer_close_desc(&desc);

    return 0;
}

