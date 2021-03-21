#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "dxf_lexer.h"
#include "memmap.h"

#define GROUP_CODE_TAG_XLAT_TAB_LEN 128

const struct dxf_group_code_desc dxf_invalid_desc = { DXF_INVALID_TAG, "Invalid", -1, -1 };
const struct dxf_group_code_desc dxf_group_code_descs[] = {
    { DXF_ENTITY_TYPE, "Entity type", 0, 0 },
    { DXF_ENTITY_PRIMARY_TEXT, "Primary text value for an entity", 1, 1 },
    { DXF_BLOCK_NAME, "Block name", 2, 2 },
    { DXF_OTHER_NAME, "Other name values", 3, 4 },
    { DXF_ENTITY_HANDLE, "Entity handle", 5, 5 },
    { DXF_LINE_TYPE, "Line type name", 6, 6 },
    { DXF_TEXT_STYLE, "Text style name", 7, 7 },
    { DXF_LAYER_NAME, "Layer name", 8, 8 },
    { DXF_VARIABLE_NAME, "Variable name", 9, 9 },
    { DXF_X, "X value of a primary point", 10, 18 },
    { DXF_Y, "Y value of a primary point", 20, 28 },
    { DXF_Z, "Z value of a primary point", 30, 37 },
    { DXF_ELEVATION, "Elevation of an entity", 38, 38 },
    { DXF_THICKNESS, "Thickness", 39, 39 },
    { DXF_FLOAT, "Float", 40, 48 },
    { DXF_REPEATED_FLOAT, "Repeated float", 49, 49 },
    { DXF_ANGLE, "Angle value", 50, 58 },
    { DXF_VISIBILITY, "Visibility of an entity", 60, 60 },
    { DXF_COLOR_NUMBER, "Color number", 62, 62 },
    { DXF_ENTITIES_FOLLOW, "Entities follow flag", 66, 66 },
    { DXF_SPACE, "Model or paper space", 67, 67 },
    { DXF_INTEGER, "Integer 16", 70, 78 },
    { DXF_INTEGER32, "Integer 32", 90, 99 },
    { DXF_SUBCLASS_DATA_MARKER, "Subclass data marker", 100, 100 },
    { DXF_CONTROL_STRING, "Control string", 102, 102 },
    { DXF_DIMVAR_SYMBOL_TABLE_ENTRY_OBJECT_HANDLE, "DIMVAR symbol table entry object handle", 105, 105 },
    { DXF_EXTRUSION_DIRECTION_X, "X value of extrusion direction", 210, 210 },
    { DXF_EXTRUSION_DIRECTION_Y, "Y value of extrusion direction", 220, 220 },
    { DXF_EXTRUSION_DIRECTION_Z, "Z value of extrusion direction", 230, 230 },
    { DXF_INTEGER8, "Integer 8", 280, 289 },
    { DXF_TEXT, "Text string", 300, 309 },
    { DXF_BINARY_CHUNK, "Binary chunk", 310, 319 },
    { DXF_OBJECT_HANDLE, "Object handle", 320, 329 },
    { DXF_SOFT_POINTER_HANDLE, "Soft pointer handle", 330, 339 },
    { DXF_HARD_POINTER_HANDLE, "Hard pointer handle", 340, 349 },
    { DXF_SOFT_OWNER_HANDLE, "Soft owner handle", 350, 359 },
    { DXF_HARD_OWNER_HANDLE, "Hard owner handle", 360, 369 },
    { DXF_COMMENT, "Comment", 999, 999 },
    { DXF_ASCII_STRING, "ASCII string", 1000, 1000 },
    { DXF_EXT_DATA_APP_NAME, "Ext data application name", 1001, 1001 },
    { DXF_EXT_DATA_CONTROL_STRING, "Ext data control string", 1002, 1002 },
    { DXF_EXT_DATA_LAYER_NAME, "Ext data layer name", 1003, 1003 },
    { DXF_EXT_DATA_BINARY_CHUNK, "Ext data binary chunk", 1004, 1004 },
    { DXF_EXT_DATA_ENTITY_HANDLE, "Ext data entity handle", 1005, 1005 },
    { DXF_EXT_DATA_POINT_X, "Ext data X value of a point", 1010, 1010 },
    { DXF_EXT_DATA_POINT_Y, "Ext data Y value of a point", 1020, 1020 },
    { DXF_EXT_DATA_POINT_Z, "Ext data Z value of a point", 1030, 1030 },
    { DXF_EXT_DATA_WCS_POSITION_X, "Ext data X value of a WCS position", 1011, 1011 },
    { DXF_EXT_DATA_WCS_POSITION_Y, "Ext data Y value of a WCS position", 1021, 1021 },
    { DXF_EXT_DATA_WCS_POSITION_Z, "Ext data Z value of a WCS position", 1031, 1031 },
    { DXF_EXT_DATA_WCS_DISPLACEMENT_X, "Ext data X value of a WCS displacement", 1012, 1012 },
    { DXF_EXT_DATA_WCS_DISPLACEMENT_Y, "Ext data Y value of a WCS displacement", 1022, 1022 },
    { DXF_EXT_DATA_WCS_DISPLACEMENT_Z, "Ext data Z value of a WCS displacement", 1032, 1032 },
    { DXF_EXT_DATA_WCS_DIRECTION_X, "Ext data X value of a WCS direction", 1013, 1013 },
    { DXF_EXT_DATA_WCS_DIRECTION_Y, "Ext data Y value of a WCS direction", 1023, 1023 },
    { DXF_EXT_DATA_WCS_DIRECTION_Z, "Ext data Z value of a WCS direction", 1033, 1033 },
    { DXF_EXT_DATA_FLOAT, "Ext data floating point value", 1040, 1040 },
    { DXF_EXT_DATA_DISTANCE, "Ext data distance", 1041, 1041 },
    { DXF_EXT_DATA_SCALE_FACTOR, "Ext data scale factor", 1042, 1042 },
    { DXF_EXT_DATA_INTEGER16, "Ext data integer 16", 1070, 1070 },
    { DXF_EXT_DATA_INTEGER32, "Ext data integer 32", 1071, 1071 },
    { DXF_INVALID_TAG, NULL, -1, -1 }
};

const struct dxf_token dxf_invalid_token = { DXF_INVALID_TAG, NULL };

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

int dxf_lexer_init()
{
    if (xlat_tab_init() != 0) {
        return -1;
    }

#ifdef DEBUG
    const struct dxf_group_code_desc *desc;
    int grp_code;

    for (grp_code = 0; grp_code < 2000; ++grp_code) {
        if ((desc = xlat_tab_get(grp_code)) != &dxf_invalid_desc) {
            printf("grp_code %d: tag=%d, name=%s, start=%d, end=%d \n", grp_code, desc->tag, desc->name,
                    desc->range_start, desc->range_end);
        }
    }
#endif

    return 0;
}


int dxf_lexer_init_desc(struct dxf_lexer_desc* const desc)
{
    desc->buf = NULL;
    desc->cur = NULL;
    desc->end = NULL;
    desc->fd = (memmap_fd_t)0;
    desc->err = EBADF;
    memcpy(&(desc->token), &dxf_invalid_token, sizeof(struct dxf_token));

    return 0;
}

int dxf_lexer_open_desc(struct dxf_lexer_desc* const desc, const char *filename)
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
        desc->cur = NULL;
        desc->end = NULL;
    }

    desc->err = errno;
    memcpy(&(desc->token), &dxf_invalid_token, sizeof(struct dxf_token));

    return desc->buf != NULL ? 0 : -1;
}

int dxf_lexer_close_desc(struct dxf_lexer_desc* const desc)
{
    size_t file_len;
    int result;

    file_len = memmap_get_file_size(desc->fd);
    memmap_unmap(desc->buf, file_len);
    result = memmap_close(desc->fd);

    desc->err = errno;

    return result;
}

int dxf_lexer_get_token(struct dxf_lexer_desc* const desc)
{
}

