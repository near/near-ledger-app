
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
#ifdef TARGET_BLUE
#include "blue/ui_menus_blue.h"
#include "blue/ui_menus_blue_prepro.h"
#endif

ux_state_t ux;

// UI currently displayed
enum UI_STATE ui_state;

int ux_step, ux_step_count;


void menu_address_init() {
    ux_step = 0;
    ux_step_count = 2;
    #if defined(TARGET_BLUE)
        UX_DISPLAY(ui_address_blue, ui_address_blue_prepro);
    #elif defined(TARGET_NANOS)
        UX_DISPLAY(ui_address_nanos, ui_address_prepro);
    #endif // #if TARGET_ID
}

// Idle state, sow the menu
void ui_idle() {
    ux_step = 0; ux_step_count = 0;
    ui_state = UI_IDLE;
    #if defined(TARGET_BLUE)
        UX_DISPLAY(ui_idle_blue, ui_idle_blue_prepro);
    #elif defined(TARGET_NANOS)
        UX_MENU_DISPLAY(0, menu_main, NULL);
    #endif // #if TARGET_ID
}

/*
 Adapted from https://en.wikipedia.org/wiki/Double_dabble#C_implementation 
 Returns: length of resulting string or -1 for error
*/
int format_long_int_amount(int n, uint16_t *arr, unsigned char *output) {
    int nbits = 16 * n;       /* length of arr in bits */
    int nscratch = nbits / 3; /* length of scratch in bytes */
    char scratch[43] = {};
    // TODO: Pass and check size of output, do not use temp buffer
    if (nscratch >= sizeof(scratch)) {
        // Scratch buffer is too small
        output[0] = '\0';
        return -1;
    }

    int i, j, k;
    int smin = nscratch - 2; /* speed optimization */

    for (i = 0; i < n; ++i) {
        for (j = 0; j < 16; ++j) {
            /* This bit will be shifted in on the right. */
            int shifted_in = (arr[n - i - 1] & (1 << (15 - j))) ? 1 : 0;

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
    memmove(scratch, scratch + k, nscratch + 1);

    /* Convert the scratch space from BCD digits to ASCII. */
    for (k = 0; k < nscratch; ++k) {
        scratch[k] += '0';
    }

    /* Resize and return */
    memmove(output, scratch, nscratch + 1);
    return nscratch;
}

int format_long_decimal_amount(int n, uint16_t *arr, char *output, int nomination) {
    int len = format_long_int_amount(n, arr, output);
    if (len <= nomination) {
        // < 1.0
        memmove(output + 2 + (nomination - len), output, len);
        memset(output + 2, '0', (nomination - len));
        output[0] = '0';
        output[1] = '.';
        len = nomination + 2;
    } else {
        // >= 1.0
        int int_len = len - nomination;
        memmove(output + int_len + 1, output + int_len, nomination);
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

// Show the transaction details for the user to approve
void menu_sign_init() {
    os_memset((unsigned char *) &ui_context, 0, sizeof(uiContext_t));

    // TODO: Validate data when parsing tx

    unsigned int processed = 0;
    
    uint32_t signer_id_len = *((uint32_t *) &tmp_ctx.signing_context.buffer[processed]);
    processed += 4;
    unsigned char *signer_id = &tmp_ctx.signing_context.buffer[processed];
    processed += signer_id_len;

    // public key
    processed += 33;

    // nonce
    processed += 8;

    uint32_t receiver_id_len = *((uint32_t *) &tmp_ctx.signing_context.buffer[processed]);
    processed += 4;
    unsigned char *receiver_id = &tmp_ctx.signing_context.buffer[processed];
    processed += receiver_id_len;

    // block hash
    processed += 32;

    // TODO: Make sure to trunc to max UI length
    os_memmove(ui_context.line2, receiver_id, receiver_id_len);
    PRINTF("receiver_id: %s\n", ui_context.line2);
    os_memmove(ui_context.line3, signer_id, signer_id_len);
    PRINTF("signer_id: %s\n", ui_context.line3);

    // actions
    uint32_t actions_len = *((uint32_t *) &tmp_ctx.signing_context.buffer[processed]);
    PRINTF("actions_len: %d\n", actions_len);
    processed += 4;

    // TODO: Parse more than one action
    uint8_t action_type = *((uint8_t *) &tmp_ctx.signing_context.buffer[processed]);
    processed += 1;

    // transfer
    PRINTF("action_type: %d\n", action_type);

    if (action_type == 3) {
        // NOTE: Have to copy to have word-aligned array (otherwise crashing on read)
        // Lots of time has been lost debugging this, make sure to avoid unaligned RAM access (as compiler in BOLOS SDK won't)
        uint16_t amount[8];
        os_memmove(amount, &tmp_ctx.signing_context.buffer[processed], 16);
        format_long_decimal_amount(8, amount, ui_context.line1, 24);

        processed += 16;

        // TODO: Should step count be adjusted?
        // Set the step/step count, and ui_state before requesting the UI
        ux_step = 0; ux_step_count = 9;
        ui_state = UI_VERIFY;

        #if defined(TARGET_BLUE)
            UX_DISPLAY(ui_verify_transfer_blue, NULL);
        #elif defined(TARGET_NANOS)
            UX_DISPLAY(ui_verify_transfer_nanos, ui_verify_transfer_prepro);
        #endif // #if TARGET_ID
        return;
    }

    ux_step = 0; ux_step_count = 3;
    ui_state = UI_VERIFY;
    #if defined(TARGET_BLUE)
        UX_DISPLAY(ui_approval_blue, ui_approval_blue_prepro);
    #elif defined(TARGET_NANOS)
        UX_DISPLAY(ui_verify_transaction_nanos, ui_verify_transaction_prepro);
    #endif // #if TARGET_ID
}