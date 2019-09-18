
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
#include "../crypto/waves.h"
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

bool print_amount(uint64_t amount, unsigned char *out, uint8_t len);

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

// Show the transaction details for the user to approve
void menu_sign_init() {
    os_memset((unsigned char *) &ui_context, 0, sizeof(uiContext_t));

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

    // actions
    uint32_t actions_len = *((uint32_t *) &tmp_ctx.signing_context.buffer[processed]);
    processed += 4;

    // TODO: Parse more than one action
    uint8_t action_type = *((uint8_t *) &tmp_ctx.signing_context.buffer[processed]);
    processed += 1;

    // transfer
    if (action_type == 3) {
        uint64_t amount = *((uint64_t *) &tmp_ctx.signing_context.buffer[processed]);
        // TODO: Print 128-bit value
        print_amount(amount, (unsigned char *) ui_context.line1, 45);
        processed += 16;

        // TODO: Make sure to trunc to max UI length
        os_memmove(ui_context.line2, receiver_id, receiver_id_len);
        os_memmove(ui_context.line3, signer_id, signer_id_len);

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


// borrowed from the Stellar wallet code and modified
bool print_amount(uint64_t amount, unsigned char *out, uint8_t len) {
    int decimals = 18;

    char buffer[len];
    uint64_t dVal = amount;
    int i, j;

    if (decimals == 0) decimals--;

    memset(buffer, 0, len);
    for (i = 0; dVal > 0 || i < decimals + 2; i++) {
        if (dVal > 0) {
            buffer[i] = (char) ((dVal % 10) + '0');
            dVal /= 10;
        } else {
            buffer[i] = '0';
        }
        if (i == decimals - 1) {
            i += 1;
            buffer[i] = '.';
        }
        if (i >= len) {
            return false;
        }
    }
    // reverse order
    for (i -= 1, j = 0; i >= 0 && j < len-1; i--, j++) {
        out[j] = buffer[i];
    }
    if (decimals > 0) {
        // strip trailing 0s
        for (j -= 1; j > 0; j--) {
            if (out[j] != '0') break;
        }
        j += 1;
        if (out[j - 1] == '.') j -= 1;
    }

    out[j] = '\0';
    return  true;
}