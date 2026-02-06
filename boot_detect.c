#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pins.h"
#include "pico/stdlib.h"
#include "glitch.h"
#include "misc.h"
#include "fuses.h"
#include "board_detect.h"

bool mariko = true;
bool board_detected = false;

// maximum number of retries for boot detection before giving up
#define BOOT_DETECT_MAX_RETRIES 4

bool wait_for_boot(int timeout_ms) {
    // retry the entire boot detection sequence up to BOOT_DETECT_MAX_RETRIES times
    for (int retry = 0; retry < BOOT_DETECT_MAX_RETRIES; retry++) {
        absolute_time_t tio_full = make_timeout_time_ms(timeout_ms);
        absolute_time_t tio_cmd1 = tio_full;
        init_glitch_pio();
        reset_cpu();
        uint32_t word=0, last_word=0;
        bool was_read_zero = false;
        bool was_cmd1 = false;
        int reset_attempts = 0;

        while(!time_reached(tio_full)) {
            if (time_reached(tio_cmd1))
            {
                if (reset_attempts > 4)
                {
                    // internal timeout, break and retry the whole sequence
                    break;
                }
                reset_attempts++;
                reset_cpu();
                tio_cmd1 = tio_full;
            }
            if(!pio_sm_is_rx_fifo_empty(pio1, 0))
            {
                word = pio_sm_get(pio1, 0);
                if (last_word == 0x41000000 && word == 0x00F9) // cmd1 request
                {
                    tio_cmd1 = make_timeout_time_ms(20);
                    was_cmd1 = true;
                }
                else if (last_word == 0x00F9 && (word >> 24) == 0x3F) // cmd1 responce
                {
                    tio_cmd1 = tio_full;
                }
                else if (last_word == 0x51000000 && word == 0x0055) //read block 0
                {
                    // OLED models sometimes need more time between block 0 and block 1
                    // original was 100ms, increased to 250ms for (hopefully) better OLED compatibility
                    tio_full = make_timeout_time_ms(250);
                    was_read_zero = true;
                } else if (was_read_zero && last_word == 0x4D000200 && word == 0x00B1) // read status - erista only
                {
                    mariko = false;
                } else if (last_word == 0x51000000 && word == 0x0147) // read block 1, can finish now
                {
                    deinit_glitch_pio();
                    return true;
                }
                last_word = word;
            }
        }
        
        // properly clean up all state machines before retrying
        // original deinit only disabled SM 0 and 1, but SM 2 (G_DAT0_SM) was left running
        pio_set_sm_mask_enabled(pio1, 0x7, false); // Disable SM 0, 1, AND 2 (0x7 = 0b111)
        
        // clean up GPIO pins
        for (int i = PIN_CLK; i <= PIN_DAT; i++)
        {
            gpio_deinit(i);
            gpio_disable_pulls(i);
            gpio_disable_input_output(i);
        }
        gpio_deinit(gli_pin());
        
        // only halt with error if all retries are exhausted
        if (retry == BOOT_DETECT_MAX_RETRIES - 1) {
            if (was_read_zero) {
                halt_with_error(1, 3);
            }
            else if (was_cmd1) {
                halt_with_error(2, 3);
            } else {
                halt_with_error(3, 3);
            }
        }
        
        // small delay before retrying to let hardware settle
        sleep_ms(50);
    }
    
    return false;
}
