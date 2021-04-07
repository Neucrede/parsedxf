#ifndef __DXF_LEXER_H__
#define __DXF_LEXER_H__

#include <sys/types.h>
#include "memmap.h"
#include "crapool.h"

#define DXF_LEXER_DESC_INITIAL_POOL_SIZE 4096
#define DXF_LEXER_LINE_BUFFER_SIZE 256
#define DXF_LEXER_MAX_LINE_LENGTH (DXF_LEXER_LINE_BUFFER_SIZE - 1)

/* Lexer tags */
#define DXF_INVALID_TAG 0
#define DXF_ENTITY_TYPE 1
#define DXF_ENTITY_PRIMARY_TEXT 2
#define DXF_BLOCK_NAME 3
#define DXF_OTHER_NAME 4
#define DXF_ENTITY_HANDLE 5
#define DXF_LINE_TYPE 6
#define DXF_TEXT_STYLE 7
#define DXF_LAYER_NAME 8
#define DXF_VARIABLE_NAME 9
#define DXF_X 10
#define DXF_Y 11
#define DXF_Z 12
#define DXF_ELEVATION 13
#define DXF_THICKNESS 14
#define DXF_FLOAT 15
#define DXF_REPEATED_FLOAT 16
#define DXF_ANGLE 17
#define DXF_VISIBILITY 18
#define DXF_COLOR_NUMBER 19
#define DXF_ENTITIES_FOLLOW 20
#define DXF_SPACE 21
#define DXF_INTEGER 22
#define DXF_INTEGER32 23
#define DXF_SUBCLASS_DATA_MARKER 24
#define DXF_CONTROL_STRING 25
#define DXF_DIMVAR_SYMBOL_TABLE_ENTRY_OBJECT_HANDLE 26
#define DXF_EXTRUSION_DIRECTION_X 27
#define DXF_EXTRUSION_DIRECTION_Y 28
#define DXF_EXTRUSION_DIRECTION_Z 29
#define DXF_INTEGER8 30
#define DXF_TEXT 31
#define DXF_BINARY_CHUNK 32
#define DXF_OBJECT_HANDLE 33
#define DXF_SOFT_POINTER_HANDLE 34
#define DXF_HARD_POINTER_HANDLE 35
#define DXF_SOFT_OWNER_HANDLE 36
#define DXF_HARD_OWNER_HANDLE 37
#define DXF_COMMENT 38
#define DXF_ASCII_STRING 39
#define DXF_EXT_DATA_APP_NAME 40
#define DXF_EXT_DATA_CONTROL_STRING 41
#define DXF_EXT_DATA_LAYER_NAME 42
#define DXF_EXT_DATA_BINARY_CHUNK 43
#define DXF_EXT_DATA_ENTITY_HANDLE 44
#define DXF_EXT_DATA_POINT_X 45
#define DXF_EXT_DATA_POINT_Y 46
#define DXF_EXT_DATA_POINT_Z 47
#define DXF_EXT_DATA_WCS_POSITION_X 48
#define DXF_EXT_DATA_WCS_POSITION_Y 49
#define DXF_EXT_DATA_WCS_POSITION_Z 50
#define DXF_EXT_DATA_WCS_DISPLACEMENT_X 51
#define DXF_EXT_DATA_WCS_DISPLACEMENT_Y 52
#define DXF_EXT_DATA_WCS_DISPLACEMENT_Z 53
#define DXF_EXT_DATA_WCS_DIRECTION_X 54
#define DXF_EXT_DATA_WCS_DIRECTION_Y 55
#define DXF_EXT_DATA_WCS_DIRECTION_Z 56
#define DXF_EXT_DATA_FLOAT 57
#define DXF_EXT_DATA_DISTANCE 58
#define DXF_EXT_DATA_SCALE_FACTOR 59
#define DXF_EXT_DATA_INTEGER16 60
#define DXF_EXT_DATA_INTEGER32 61

struct dxf_lexer_desc;
typedef int (*pfn_scanner_t)(struct dxf_lexer_desc* const, void*);

struct dxf_group_code_desc {
    int tag;
    char *name;
    unsigned int range_start;
    unsigned int range_end;
    pfn_scanner_t scanner;
};

struct dxf_token {
    int tag;
    unsigned int group_code;
    union {
        char *str;
        int i;
        double f;
        void *bin;
    } value;
};

struct dxf_lexer_desc {
    const char *buf;
    const char *cur;
    const char *prev;
    const char *end;
    memmap_fd_t fd;
    memmap_fd_t fd2;
    char line_buf[DXF_LEXER_LINE_BUFFER_SIZE];
    struct crapool_desc *pool;
    struct dxf_token token;
};

extern const struct dxf_group_code_desc dxf_invalid_desc;
extern const struct dxf_group_code_desc dxf_group_code_descs[];
extern const struct dxf_token dxf_invalid_token;

#ifdef __cplusplus
extern "C" {
#endif

int dxf_lexer_init();
int dxf_lexer_init_desc(struct dxf_lexer_desc* const desc, const char *buf,
                        size_t buf_len, struct crapool_desc* const pool);
int dxf_lexer_clear_desc(struct dxf_lexer_desc* const desc);
int dxf_lexer_open_desc(struct dxf_lexer_desc* const desc, const char *filename, 
                        struct crapool_desc* const pool);
int dxf_lexer_close_desc(struct dxf_lexer_desc* const desc, int destroy_pool);
int dxf_lexer_get_token(struct dxf_lexer_desc* const desc);
int dxf_lexer_unget_token(struct dxf_lexer_desc* const desc);
int dxf_lexer_skip_to(struct dxf_lexer_desc* const lexer_desc, int tag_expected);

#ifdef __cplusplus
}
#endif

#endif /* __DXF_LEXER_H__ */
