#include "ui_menus_prepro.h"

#include "../ui.h"

const bagl_element_t * simple_scroll_prepro(const bagl_element_t *element) {
    // display or not according to step, and adjust delay
    if (element->component.userid > 0) {
        unsigned int display = (ux_step == element->component.userid - 1);
        if (display) {
            UX_CALLBACK_SET_INTERVAL(
                MAX(2000, bagl_label_roundtrip_duration_ms(element, 7)));
        }
        if (!display) {
             return NULL;
        }
    }
    return element;
}

const bagl_element_t * ui_address_prepro(const bagl_element_t *element) {
    return simple_scroll_prepro(element);
}

const bagl_element_t * ui_verify_transfer_prepro(const bagl_element_t *element) {
    return simple_scroll_prepro(element);
}

const bagl_element_t * ui_verify_function_call_prepro(const bagl_element_t *element) {
    return simple_scroll_prepro(element);
}

const bagl_element_t * ui_verify_transaction_prepro(const bagl_element_t *element) {
    return simple_scroll_prepro(element);
}