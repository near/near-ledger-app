#ifndef __UI_MENUS_NANOS_H__
#define __UI_MENUS_NANOS_H__

#include "os.h"
#include "cx.h"

//    type          userid  x  y  w    h str rad fill       fg        bg      fid iid  txt 
#define EMPTY_BG() \
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0}, NULL}

//    type     userid  x  y   w  h str rad fill fg     bg      fid  iid    txt
#define LEFT_ICON(icon) \
    {{BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, icon}, NULL}
#define RIGHT_ICON(icon) \
    {{BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, icon}, NULL}

//    type       userid  x  y   w    h  str rad fill    fg        bg
//    fid                                                            iid  txt
#define INFO_CELL(id, title, info)  \
    {{BAGL_LABELINE, id, 0, 12, 128, 12, 0, 0, 0, 0xFFFFFF, 0x000000, \
      BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER, 0}, title}, \
    {{BAGL_LABELINE, id, 23, 26, 82, 12, BAGL_STROKE_FLAG_ONESHOT | 10, 0, 0, 0xFFFFFF, 0x000000, \
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 26}, info}

const bagl_element_t ui_address_nanos[] = {
    EMPTY_BG(),

    LEFT_ICON(BAGL_GLYPH_ICON_CROSS),
    RIGHT_ICON(BAGL_GLYPH_ICON_CHECK),

    INFO_CELL(0x01, "Confirm", "public key")
};

const bagl_element_t ui_verify_transfer_nanos[] = {
    EMPTY_BG(),

    LEFT_ICON(BAGL_GLYPH_ICON_CROSS),
    RIGHT_ICON(BAGL_GLYPH_ICON_CHECK),

    INFO_CELL(0x01, "Confirm", "transfer"),
    INFO_CELL(0x02, "Amount", ui_context.line1),
    INFO_CELL(0x03, "To", ui_context.line2),
    INFO_CELL(0x04, "From", ui_context.line3)
};

const bagl_element_t ui_verify_function_call_nanos[] = {
    EMPTY_BG(),

    LEFT_ICON(BAGL_GLYPH_ICON_CROSS),
    RIGHT_ICON(BAGL_GLYPH_ICON_CHECK),

    INFO_CELL(0x01, "Confirm", ui_context.line1),
    INFO_CELL(0x02, "Deposit", ui_context.line5),
    INFO_CELL(0x03, "To", ui_context.line2),
    INFO_CELL(0x04, "From", ui_context.line3),
    INFO_CELL(0x05, "Args", ui_context.line4)
};

const bagl_element_t ui_verify_transaction_nanos[] = {
    EMPTY_BG(),

    LEFT_ICON(BAGL_GLYPH_ICON_CROSS),
    RIGHT_ICON(BAGL_GLYPH_ICON_CHECK),

    INFO_CELL(0x01, "Confirm", ui_context.line1),
    INFO_CELL(0x02, "To", ui_context.line2),
    INFO_CELL(0x03, "From", ui_context.line3)
};

const ux_menu_entry_t menu_about[];

const ux_menu_entry_t menu_main[] = {
    {NULL, NULL, 0, &C_icon_waves, "Use wallet to", "view accounts", 33, 12},
    {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
    {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
    UX_MENU_END};

const ux_menu_entry_t menu_about[] = {
    {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
    {menu_main, NULL, 1, &C_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END};

#endif