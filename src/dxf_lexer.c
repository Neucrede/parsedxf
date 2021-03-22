#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "dxf_lexer.h"
#include "memmap.h"
#include "dbgprint.h"

#define GROUP_CODE_TAG_XLAT_TAB_LEN 128

struct xlat_tab_entry;
struct xlat_tab_entry {
    struct xlat_tab_entry *next;
    int group_code;
    const struct dxf_group_code_desc *desc;
};

struct xlat_tab {
    int len;
    struct xlat_tab_entry *table;
} group_code_desc_xlat_tab;

static int xlat_tab_init();
static int xlat_tab_put(int grp_code, const struct dxf_group_code_desc *desc);
static const struct dxf_group_code_desc *xlat_tab_get(int grp_code);
static int skip_blanks(struct dxf_lexer_desc* const desc);
static int get_line(struct dxf_lexer_desc* const desc);
static int next_line(struct dxf_lexer_desc* const desc);
static int scan_integer(struct dxf_lexer_desc* const desc, int *pi);
static int scan_float(struct dxf_lexer_desc* const desc, float *pf);
static int scan_string(struct dxf_lexer_desc* const desc, char **buf);
static int scan_binary(struct dxf_lexer_desc* const desc, void **buf);

const struct dxf_group_code_desc dxf_invalid_desc = 
    { DXF_INVALID_TAG, "Invalid", -1, -1, (pfn_scanner_t)scan_string };
    
const struct dxf_group_code_desc dxf_group_code_descs[] = {
    { DXF_ENTITY_TYPE, "Entity type", 0, 0, (pfn_scanner_t)scan_string },
    { DXF_ENTITY_PRIMARY_TEXT, "Primary text value for an entity", 1, 1, (pfn_scanner_t)scan_string },
    { DXF_BLOCK_NAME, "Block name", 2, 2, (pfn_scanner_t)scan_string },
    { DXF_OTHER_NAME, "Other name values", 3, 4, (pfn_scanner_t)scan_string },
    { DXF_ENTITY_HANDLE, "Entity handle", 5, 5, (pfn_scanner_t)scan_string },
    { DXF_LINE_TYPE, "Line type name", 6, 6, (pfn_scanner_t)scan_string },
    { DXF_TEXT_STYLE, "Text style name", 7, 7, (pfn_scanner_t)scan_string },
    { DXF_LAYER_NAME, "Layer name", 8, 8, (pfn_scanner_t)scan_string },
    { DXF_VARIABLE_NAME, "Variable name", 9, 9, (pfn_scanner_t)scan_string },
    { DXF_X, "X value of a primary point", 10, 18, (pfn_scanner_t)scan_float },
    { DXF_Y, "Y value of a primary point", 20, 28, (pfn_scanner_t)scan_float },
    { DXF_Z, "Z value of a primary point", 30, 37, (pfn_scanner_t)scan_float },
    { DXF_ELEVATION, "Elevation of an entity", 38, 38, (pfn_scanner_t)scan_float },
    { DXF_THICKNESS, "Thickness", 39, 39, (pfn_scanner_t)scan_integer },
    { DXF_FLOAT, "Float", 40, 48, (pfn_scanner_t)scan_float },
    { DXF_REPEATED_FLOAT, "Repeated float", 49, 49, (pfn_scanner_t)scan_float },
    { DXF_ANGLE, "Angle value", 50, 58, (pfn_scanner_t)scan_float },
    { DXF_VISIBILITY, "Visibility of an entity", 60, 60, (pfn_scanner_t)scan_integer },
    { DXF_COLOR_NUMBER, "Color number", 62, 62, (pfn_scanner_t)scan_integer },
    { DXF_ENTITIES_FOLLOW, "Entities follow flag", 66, 66, (pfn_scanner_t)scan_integer },
    { DXF_SPACE, "Model or paper space", 67, 67, (pfn_scanner_t)scan_integer },
    { DXF_INTEGER, "Integer 16", 70, 78, (pfn_scanner_t)scan_integer },
    { DXF_INTEGER32, "Integer 32", 90, 99, (pfn_scanner_t)scan_integer },
    { DXF_SUBCLASS_DATA_MARKER, "Subclass data marker", 100, 100, (pfn_scanner_t)scan_string },
    { DXF_CONTROL_STRING, "Control string", 102, 102, (pfn_scanner_t)scan_string },
    { DXF_DIMVAR_SYMBOL_TABLE_ENTRY_OBJECT_HANDLE, "DIMVAR symbol table entry object handle", 
        105, 105, (pfn_scanner_t)scan_string },
    { DXF_EXTRUSION_DIRECTION_X, "X value of extrusion direction", 210, 210, (pfn_scanner_t)scan_float },
    { DXF_EXTRUSION_DIRECTION_Y, "Y value of extrusion direction", 220, 220, (pfn_scanner_t)scan_float },
    { DXF_EXTRUSION_DIRECTION_Z, "Z value of extrusion direction", 230, 230, (pfn_scanner_t)scan_float },
    { DXF_INTEGER8, "Integer 8", 280, 289, (pfn_scanner_t)scan_integer },
    { DXF_TEXT, "Text string", 300, 309, (pfn_scanner_t)scan_string },
    { DXF_BINARY_CHUNK, "Binary chunk", 310, 319, (pfn_scanner_t)scan_binary },
    { DXF_OBJECT_HANDLE, "Object handle", 320, 329, (pfn_scanner_t)scan_string },
    { DXF_SOFT_POINTER_HANDLE, "Soft pointer handle", 330, 339, (pfn_scanner_t)scan_string },
    { DXF_HARD_POINTER_HANDLE, "Hard pointer handle", 340, 349, (pfn_scanner_t)scan_string },
    { DXF_SOFT_OWNER_HANDLE, "Soft owner handle", 350, 359, (pfn_scanner_t)scan_string },
    { DXF_HARD_OWNER_HANDLE, "Hard owner handle", 360, 369, (pfn_scanner_t)scan_string },
    { DXF_COMMENT, "Comment", 999, 999, (pfn_scanner_t)scan_string },
    { DXF_ASCII_STRING, "ASCII string", 1000, 1000, (pfn_scanner_t)scan_string },
    { DXF_EXT_DATA_APP_NAME, "Ext data application name", 1001, 1001, (pfn_scanner_t)scan_string },
    { DXF_EXT_DATA_CONTROL_STRING, "Ext data control string", 1002, 1002, (pfn_scanner_t)scan_string },
    { DXF_EXT_DATA_LAYER_NAME, "Ext data layer name", 1003, 1003, (pfn_scanner_t)scan_string },
    { DXF_EXT_DATA_BINARY_CHUNK, "Ext data binary chunk", 1004, 1004, (pfn_scanner_t)scan_binary },
    { DXF_EXT_DATA_ENTITY_HANDLE, "Ext data entity handle", 1005, 1005, (pfn_scanner_t)scan_string },
    { DXF_EXT_DATA_POINT_X, "Ext data X value of a point", 1010, 1010, (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_POINT_Y, "Ext data Y value of a point", 1020, 1020, (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_POINT_Z, "Ext data Z value of a point", 1030, 1030, (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_WCS_POSITION_X, "Ext data X value of a WCS position", 1011, 1011, 
        (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_WCS_POSITION_Y, "Ext data Y value of a WCS position", 1021, 1021,
        (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_WCS_POSITION_Z, "Ext data Z value of a WCS position", 1031, 1031, 
        (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_WCS_DISPLACEMENT_X, "Ext data X value of a WCS displacement", 1012, 1012, 
        (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_WCS_DISPLACEMENT_Y, "Ext data Y value of a WCS displacement", 1022, 1022, 
        (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_WCS_DISPLACEMENT_Z, "Ext data Z value of a WCS displacement", 1032, 1032, 
        (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_WCS_DIRECTION_X, "Ext data X value of a WCS direction", 1013, 1013, 
        (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_WCS_DIRECTION_Y, "Ext data Y value of a WCS direction", 1023, 1023, 
        (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_WCS_DIRECTION_Z, "Ext data Z value of a WCS direction", 1033, 1033, 
        (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_FLOAT, "Ext data floating point value", 1040, 1040, (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_DISTANCE, "Ext data distance", 1041, 1041, (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_SCALE_FACTOR, "Ext data scale factor", 1042, 1042, (pfn_scanner_t)scan_float },
    { DXF_EXT_DATA_INTEGER16, "Ext data integer 16", 1070, 1070, (pfn_scanner_t)scan_integer },
    { DXF_EXT_DATA_INTEGER32, "Ext data integer 32", 1071, 1071, (pfn_scanner_t)scan_integer },
    { DXF_INVALID_TAG, NULL, -1, -1, (pfn_scanner_t)scan_string }
};

const struct dxf_token dxf_invalid_token = { DXF_INVALID_TAG, NULL };

static int xlat_tab_init()
{
    struct xlat_tab *tab = &group_code_desc_xlat_tab;
    int len = GROUP_CODE_TAG_XLAT_TAB_LEN; 
    int slot;
    const struct dxf_group_code_desc *desc;
    int grp_code;

    tab->len = len;
    if ((tab->table = (struct xlat_tab_entry*)malloc(len * sizeof(struct xlat_tab_entry))) == NULL) {
        return -1;
    }

    for (slot = 0; slot < len; ++slot) {
        tab->table[slot].desc = &dxf_invalid_desc;
        tab->table[slot].next = NULL;
    }

    for (desc = &dxf_group_code_descs[0]; desc->tag != DXF_INVALID_TAG; ++desc) {
        for (grp_code = desc->range_start; grp_code <= desc->range_end; ++grp_code) {
            if (xlat_tab_put(grp_code, desc) != 0) {
                return -1;
            }
        }
    }
    
#ifdef DEBUG
    for (grp_code = 0; grp_code < 2000; ++grp_code) {
        if ((desc = xlat_tab_get(grp_code)) != &dxf_invalid_desc) {
            printf("grp_code %d: tag=%d, name=%s, start=%d, end=%d, scanner=@0x%x \n", 
                    grp_code, desc->tag, desc->name,
                    desc->range_start, desc->range_end, desc->scanner);
        }
    }
#endif
    
    return 0;
}

static int xlat_tab_put(int grp_code, const struct dxf_group_code_desc *desc)
{
    struct xlat_tab *tab = &group_code_desc_xlat_tab;
    int slot = grp_code % tab->len;
    struct xlat_tab_entry *first_entry = &tab->table[slot];
    struct xlat_tab_entry *entry = first_entry;
    struct xlat_tab_entry *new_entry;

    do {
        if (entry->group_code == grp_code) {
            entry->desc = desc;
            return 0;
        }
        else {
            entry = entry->next;
        }
    } while (entry != NULL);

    if (entry == first_entry) {
        new_entry = first_entry;
    }
    else if ((new_entry = (struct xlat_tab_entry*)malloc(sizeof(struct xlat_tab_entry))) == NULL) {
        return -1;
    }

    new_entry->group_code = grp_code;
    new_entry->desc = desc;
    new_entry->next = first_entry->next;
    if (new_entry != first_entry) {
        first_entry->next = new_entry;
    }

    return 0;
}

static const struct dxf_group_code_desc *xlat_tab_get(int grp_code)
{
    struct xlat_tab *tab = &group_code_desc_xlat_tab;
    int slot = grp_code % tab->len;
    struct xlat_tab_entry *entry = &tab->table[slot];

    do {
        if (entry->group_code == grp_code) {
            return entry->desc;
        }
        else {
            entry = entry->next;
        }
    } while (entry != NULL);

    return &dxf_invalid_desc;
}

static int skip_blanks(struct dxf_lexer_desc* const desc)
{
    char *end = desc->end;
    char ch = *(desc->cur);
    
    while ((desc->cur < end) && ((ch == ' ') || (ch == '\t'))) {
        ch = *(++(desc->cur));  
    }
    
    return desc->cur < end ? 0 : -1;
}

static int get_line(struct dxf_lexer_desc* const desc)
{
    int i;
    char *end;
    char ch;
    
/*  if (skip_blanks(desc) != 0) {
        return -1;
    } */
    
    end = desc->end;
    ch = *(desc->cur);
    i = 0;
    while ((desc->cur < end) && (i < DXF_LEXER_MAX_LINE_LENGTH) &&
            (ch != '\r') && (ch != '\n')) 
    {
        desc->line_buf[i++] = ch;
        ch = *(++(desc->cur));
    }
    desc->line_buf[i] = '\0';
    dbgprint("dxf_lexer: Current line is \n%s \n", desc->line_buf);
    
    /* Skip remaining text in this line if any. */
    next_line(desc);
    
    return i + 1;
}

static int next_line(struct dxf_lexer_desc* const desc)
{
    char *end = desc->end;
    char ch = *(desc->cur);
    
    while ((desc->cur < end) && (ch != '\r') && (ch != '\n')) {
        ch = *(++(desc->cur));
    }
    
    if ((desc->cur < end) && (ch == '\r')) {
        ch = *(++(desc->cur));
        if (ch != '\n') {
            return -1;
        }
        ++(desc->cur);
    }
    
    return desc->cur < end ? 0 : -1;
}

static int scan_integer(struct dxf_lexer_desc* const desc, int *pi)
{
    int i;
    
    if (get_line(desc) == -1) {
        return -1;
    }
    
    i = (int)strtol(desc->line_buf, NULL, 0);
    desc->token.value.i = i;
    
    if (pi != NULL) {
        *pi = i;
    }
    
    return 0;
}

static int scan_float(struct dxf_lexer_desc* const desc, float *pf)
{
    float f;
    
    if (get_line(desc) == -1) {
        return -1;
    }
    
    f = strtof(desc->line_buf, NULL);
    desc->token.value.f = f;
    
    if (pf != NULL) {
        *pf = f;
    }
    
    return 0;
}

static int scan_string(struct dxf_lexer_desc* const desc, char **buf)
{
    int len;
    
    if ((len = get_line(desc)) == -1) {
        return -1;
    }
    
    if ((desc->token.value.str = (char*)crapool_alloc(desc->pool, len)) == NULL) {
        return -1;
    }
    strcpy(desc->token.value.str, desc->line_buf);
    
    if (buf != NULL) {
        strcpy(*buf, desc->line_buf);
    }
    
    return 0;
}

static int scan_binary(struct dxf_lexer_desc* const desc, void **buf)
{
    if (get_line(desc) == -1) {
        return -1;
    }

    return 0;
}

int dxf_lexer_init()
{
    if (xlat_tab_init() != 0) {
        return -1;
    }

    return 0;
}

int dxf_lexer_init_desc(struct dxf_lexer_desc* const desc)
{
    desc->buf = NULL;
    desc->cur = NULL;
    desc->end = NULL;
    desc->fd = (memmap_fd_t)0;
    desc->err = EBADF;
    desc->pool = NULL;
    
    memcpy(&(desc->token), &dxf_invalid_token, sizeof(struct dxf_token));

    return 0;
}

int dxf_lexer_open_desc(struct dxf_lexer_desc* const desc, const char *filename, 
                        struct crapool_desc* const pool)
{
    memmap_fd_t fd;
    size_t file_len;
    
    fd = memmap_open(filename, O_RDONLY, 0);
    file_len = memmap_get_file_size(fd);
    desc->buf = (char*)memmap_map(NULL, file_len, MEMMAP_READ, MEMMAP_SHARED, fd, 0);
    
    if (desc->buf != NULL) {
        desc->fd = fd;
        desc->cur = desc->buf;
        desc->end = (char*)(desc->buf + file_len - 1);
    }
    else {
        return -1;
    }
    
    if (pool != NULL) {
        desc->pool = pool;
    }
    else if ((desc->pool = crapool_create(DXF_LEXER_DESC_INITIAL_POOL_SIZE, NULL)) == NULL) {
        return -1;
    }

    desc->err = errno;
    memcpy(&(desc->token), &dxf_invalid_token, sizeof(struct dxf_token));

    return 0;
}

int dxf_lexer_close_desc(struct dxf_lexer_desc* const desc, int destroy_pool)
{
    size_t file_len;

    file_len = memmap_get_file_size(desc->fd);
    memmap_unmap(desc->buf, file_len);
    memmap_close(desc->fd);
    
    if (destroy_pool != 0) {
        crapool_destroy(desc->pool);
        desc->pool = NULL;
    }

    desc->err = errno;

    return 0;
}

int dxf_lexer_get_token(struct dxf_lexer_desc* const desc)
{
    int grp_code;
    const struct dxf_group_code_desc* grp_code_desc;
    
    if (scan_integer(desc, &grp_code) != 0) {
        return -1;
    }

    grp_code_desc = xlat_tab_get(grp_code);
    
    desc->token.tag = grp_code_desc->tag;
    if (grp_code_desc->scanner == (pfn_scanner_t)scan_string ||
        grp_code_desc->scanner == (pfn_scanner_t)scan_binary)
    {
        
    }
    grp_code_desc->scanner(desc, &(desc->token.value));
    
    dbgprint("dxf_lexer: Token tag=%d, ", grp_code_desc->tag);
    if (grp_code_desc->scanner == (pfn_scanner_t)scan_string) {
        dbgprint("value=%s \n", desc->token.value.str);
    } else if (grp_code_desc->scanner == (pfn_scanner_t)scan_integer) {
        dbgprint("value=%i \n", desc->token.value.i);
    } else if (grp_code_desc->scanner == (pfn_scanner_t)scan_float) {
        dbgprint("value=%f \n", desc->token.value.f);
    } else {
        dbgprint("\n");
    }
    
    return 0;
}
