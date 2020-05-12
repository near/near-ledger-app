
/*******************************************************************************
*   Burstcoin Wallet App for Nano Ledger S. Updated By Waves community.
*   Copyright (c) 2017-2018 Jake B.
*
*   Based on Sample code provided and (c) 2016 Ledger
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "ui.h"
#include <stdbool.h>
#include "../glyphs.h"
#include "../main.h"

#ifdef TARGET_NANOS
#include "nanos/ui_menus_nanos.h"
#include "nanos/ui_menus_buttons.h"
#include "nanos/ui_menus_prepro.h"
#endif

ux_state_t ux;

// UI currently displayed
enum UI_STATE ui_state;

int ux_step, ux_step_count;


void menu_address_init() {
    ux_step = 0;
    ux_step_count = 1;
    #if defined(TARGET_NANOS)
        UX_DISPLAY(ui_address_nanos, ui_address_prepro);
    #endif // #if TARGET_ID
}

// Idle state, sow the menu
void ui_idle() {
    ux_step = 0; ux_step_count = 0;
    ui_state = UI_IDLE;
    #if defined(TARGET_NANOS)
        UX_MENU_DISPLAY(0, menu_main, NULL);
    #endif // #if TARGET_ID
}

/*
 Adapted from https://en.wikipedia.org/wiki/Double_dabble#C_implementation
 Returns: length of resulting string or -1 for error
*/
int format_long_int_amount(size_t input_size, char *input, size_t output_size, char *output) {
    // NOTE: Have to copy to have word-aligned array (otherwise crashing on read)
    // Lots of time has been lost debugging this, make sure to avoid unaligned RAM access (as compiler in BOLOS SDK won't)
    uint16_t aligned_amount[8];
    os_memmove(aligned_amount, input, 16);
    // Convert size in bytes into words
    size_t n = input_size / 2;

    size_t nbits = 16 * n;       /* length of arr in bits */
    size_t nscratch = nbits / 3; /* length of scratch in bytes */
    if (nscratch >= output_size) {
        // Output buffer is too small
        output[0] = '\0';
        return -1;
    }

    char *scratch = output;

    size_t i, j, k;
    size_t smin = nscratch - 2; /* speed optimization */

    for (i = 0; i < n; ++i) {
        for (j = 0; j < 16; ++j) {
            /* This bit will be shifted in on the right. */
            int shifted_in = (aligned_amount[n - i - 1] & (1 << (15 - j))) ? 1 : 0;

            /* Add 3 everywhere that scratch[k] >= 5. */
            for (k = smin; k < nscratch; ++k) {
                scratch[k] += (scratch[k] >= 5) ? 3 : 0;
            }

            /* Shift scratch to the left by one position. */
            if (scratch[smin] >= 8) {
                smin -= 1;
            }
            for (k = smin; k < nscratch - 1; ++k) {
                scratch[k] <<= 1;
                scratch[k] &= 0xF;
                scratch[k] |= (scratch[k + 1] >= 8);
            }

            /* Shift in the new bit from arr. */
            scratch[nscratch - 1] <<= 1;
            scratch[nscratch - 1] &= 0xF;
            scratch[nscratch - 1] |= shifted_in;
        }
    }

    /* Remove leading zeros from the scratch space. */
    for (k = 0; k < nscratch - 1; ++k) {
        if (scratch[k] != 0) {
            break;
        }
    }
    nscratch -= k;
    os_memmove(scratch, scratch + k, nscratch + 1);

    /* Convert the scratch space from BCD digits to ASCII. */
    for (k = 0; k < nscratch; ++k) {
        scratch[k] += '0';
    }

    /* Resize and return */
    os_memmove(output, scratch, nscratch + 1);
    return nscratch;
}

int format_long_decimal_amount(size_t input_size, char *input, size_t output_size, char *output, int nomination) {
    int len = format_long_int_amount(input_size, input, output_size, output);

    if (len < 0 || (size_t) len + 2 > output_size) {
        // Output buffer is too small
        output[0] = '\0';
        return -1;
    }

    if (len <= nomination) {
        // < 1.0
        os_memmove(output + 2 + (nomination - len), output, len);
        os_memset(output + 2, '0', (nomination - len));
        output[0] = '0';
        output[1] = '.';
        len = nomination + 2;
    } else {
        // >= 1.0
        int int_len = len - nomination;
        os_memmove(output + int_len + 1, output + int_len, nomination);
        output[int_len] = '.';
        len = len + 1;
    }

    // Remove trailing zeros and dot
    output[len] = '0';
    while (len > 0 && (output[len] == '0' || output[len] == '.')) {
        output[len--] = 0;
    }

    return len;
}

void check_overflow(unsigned int processed, unsigned int size) {
    PRINTF("check_overflow %d %d %d\n", processed, size, tmp_ctx.signing_context.buffer_used);
    if (size >= tmp_ctx.signing_context.buffer_used || processed + size >= tmp_ctx.signing_context.buffer_used) {
        THROW(SW_BUFFER_OVERFLOW);
    }
}

#define PRINT_REMAINING_BUFFER() \
    PRINTF("remaining buffer: %.*h\n", tmp_ctx.signing_context.buffer_used - processed, &tmp_ctx.signing_context.buffer[processed]);

uint8_t borsh_read_uint8(unsigned int *processed) {
    check_overflow(*processed, 1);
    uint8_t result = *((uint8_t *) &tmp_ctx.signing_context.buffer[*processed]);
    *processed += 1;
    return result;
}

uint32_t borsh_read_uint32(unsigned int *processed) {
    check_overflow(*processed, 4);
    uint32_t result = *((uint32_t *) &tmp_ctx.signing_context.buffer[*processed]);
    *processed += 4;
    return result;
}

void borsh_read_buffer(uint32_t *buffer_len, char **buffer, unsigned int *processed) {
    *buffer_len = borsh_read_uint32(processed);
    check_overflow(*processed, *buffer_len);
    *buffer = &tmp_ctx.signing_context.buffer[*processed];
    *processed += *buffer_len;
}

char *borsh_read_fixed_buffer(unsigned int buffer_len, unsigned int *processed) {
    check_overflow(*processed, buffer_len);
    char *buffer = &tmp_ctx.signing_context.buffer[*processed];
    *processed += buffer_len;
    return buffer;
}

void strcpy_ellipsis(size_t dst_size, char *dst, size_t src_size, char *src) {
    if (dst_size >= src_size + 1) {
        os_memmove(dst, src, src_size);
        dst[src_size] = 0;
        return;
    }

    os_memmove(dst, src, dst_size);
    size_t ellipsis_start = dst_size >= 4 ? dst_size - 4 : 0;
    for (size_t i = ellipsis_start; i < dst_size; i++) {
        dst[i] = '.';
    }
    dst[dst_size - 1] = 0;
    return;
}

#define BORSH_SKIP(size) \
    check_overflow(processed, size); \
    processed += size;

#define BORSH_DISPLAY_STRING(var_name, ui_line) \
    uint32_t var_name##_len; \
    char *var_name; \
    borsh_read_buffer(&var_name##_len, &var_name, &processed); \
    strcpy_ellipsis(sizeof(ui_line), ui_line, var_name##_len, var_name); \
    PRINTF("%s: %s\n", #var_name, ui_line);

#define BORSH_DISPLAY_AMOUNT(var_name, ui_line) \
    char *var_name = &tmp_ctx.signing_context.buffer[processed]; \
    processed += 16; \
    format_long_decimal_amount(16, var_name, sizeof(ui_context.line1), ui_context.line1, 24);

#define DISPLAY_VERIFY_UI(ui, step_count, prepro_fn) \
    ux_step = 0; \
    ux_step_count = step_count; \
    ui_state = UI_VERIFY; \
    UX_DISPLAY(ui, prepro_fn); \

#define COPY_LITERAL(dst, src) \
    os_memmove(dst, src, sizeof(src))

typedef enum {
    at_create_account,
    at_deploy_contract,
    at_function_call,
    at_transfer,
    at_stake,
    at_add_key,
    at_delete_key,
    at_delete_account,
    at_last_value = at_delete_account
} action_type_t;

// Show the transaction details for the user to approve
void menu_sign_init() {
    os_memset(&ui_context, 0, sizeof(uiContext_t));

    // TODO: Validate data when parsing tx

    unsigned int processed = 0;

    // signer
    BORSH_DISPLAY_STRING(signer_id, ui_context.line3);

    // public key
    BORSH_SKIP(33);

    // nonce
    BORSH_SKIP(8);

    // receiver
    BORSH_DISPLAY_STRING(receiver_id, ui_context.line2);

    // block hash
    BORSH_SKIP(32);

    // actions
    uint32_t actions_len = borsh_read_uint32(&processed);
    PRINTF("actions_len: %d\n", actions_len);

    // TODO: Parse more than one action
    // action type
    uint8_t action_type = borsh_read_uint8(&processed);
    PRINTF("action_type: %d\n", action_type);

    // TODO: assert action_type <= at_last_value

    // transfer
    if (action_type == at_transfer) {
        BORSH_DISPLAY_AMOUNT(amount, ui_context.line1);

        DISPLAY_VERIFY_UI(ui_verify_transfer_nanos, 4, ui_verify_transfer_prepro);
        return;
    }

    // functionCall
    if (action_type == at_function_call) {
        // method name
        BORSH_DISPLAY_STRING(method_name, ui_context.line1);

        // args
        uint32_t args_len;
        char *args;
        borsh_read_buffer(&args_len, &args, &processed);
        if (args_len > 0 && args[0] == '{') {
            // Args look like JSON
            strcpy_ellipsis(sizeof(ui_context.line4), ui_context.line4, args_len, args);
            PRINTF("args: %s\n", ui_context.line4);
        } else {
            // TODO: Hexdump args otherwise
        }

        // gas
        BORSH_SKIP(8);

        // deposit
        BORSH_DISPLAY_AMOUNT(deposit, ui_context.line5);

        DISPLAY_VERIFY_UI(ui_verify_function_call_nanos, 5, ui_verify_function_call_prepro);
        return;
    }

    // add key
    if (action_type == at_add_key) {
        // TODO: Assert that sender/receiver are the same?

        // public key

        // key type
        BORSH_SKIP(1);
        // TODO: assert ed25519 key type

        // key data
        char *data = borsh_read_fixed_buffer(32, &processed);
        // TODO: Display Base58 key?

        // access key

        // nonce
        BORSH_SKIP(8);

        // permission
        uint8_t permission_type = borsh_read_uint8(&processed);
        PRINTF("permission_type: %d\n", permission_type);
        if (permission_type == 0) {
            // function call

            // allowance
            uint8_t has_allowance = borsh_read_uint8(&processed);
            if (has_allowance) {
                BORSH_DISPLAY_AMOUNT(allowance, ui_context.line5);
            } else {
                COPY_LITERAL(ui_context.line5, "Unlimited");
            }

            // receiver
            BORSH_DISPLAY_STRING(permission_receiver_id, ui_context.line2);

            // TODO: read method names array
            // TODO: Need to display one (multiple not supported yet – can just display "multiple methods")
            DISPLAY_VERIFY_UI(ui_verify_add_function_call_access_key, 4, simple_scroll_prepro);
            return;
        } else {
            // full access

            DISPLAY_VERIFY_UI(ui_verify_add_full_access_key, 2, simple_scroll_prepro);
            return;
        }
    }

    PRINT_REMAINING_BUFFER();

    DISPLAY_VERIFY_UI(ui_verify_transaction_nanos, 3, ui_verify_transaction_prepro);
}