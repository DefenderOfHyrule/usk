#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "hardware/xosc.h"
#include "hardware/watchdog.h"
#include "boot_detect.h"
#include "board_detect.h"
#include "config.h"

#include "misc.h"
#include "glitch.h"
#include "fuses.h"
#include "pio_upload.h"
#include "pins.h"

bool write_payload();

// optimized: increased voltage for better stability at higher clock
void init_system() {
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    set_sys_clock_khz(200000, true);
}

extern uint8_t cid_buf[17];

void rewrite_payload()
{
    put_pixel(PIX_whi);
    write_payload();
    put_pixel(PIX_gre);
    init_config(cid_buf + 1);
}

// optimized: reduced timeout and combined checks
bool safe_test_voltage(int pin, float target, float range)
{
    gpio_enable_input_output(pin);
    adc_gpio_init(pin);
    adc_select_input(pin - 26);
    uint16_t result = adc_read();
    gpio_disable_input_output(pin);
    float voltage = result * 3.3f / (1 << 12);
    return voltage >= (target - range) && voltage <= (target + range);
}

// optimized: reduced timeout from 2500ms to 1500ms, faster convergence
void self_test()
{
    absolute_time_t tio_time = make_timeout_time_ms(1500);
    adc_init();
    uint8_t check_mask = 0; // bit flags: 0=rst, 1=cmd, 2=d0
    
    while (!time_reached(tio_time) && check_mask != 0x07) {
        if (!(check_mask & 0x01))
            check_mask |= safe_test_voltage(PIN_RST, 1.8f, 0.2f) ? 0x01 : 0;
        if (!(check_mask & 0x02))
            check_mask |= safe_test_voltage(PIN_CMD, 1.8f, 0.2f) ? 0x02 : 0;
        if (!(check_mask & 0x04))
            check_mask |= safe_test_voltage(PIN_DAT, 1.8f, 0.2f) ? 0x04 : 0;
    }
    
    if(!(check_mask & 0x01))
        halt_with_error(0, 2);
    if(!(check_mask & 0x02))
        halt_with_error(1, 2);
    if(!(check_mask & 0x04))
        halt_with_error(2, 2);
}

extern bool was_self_reset;

int main()
{
    // stop watchdog
    *(uint32_t*)(0x40058000 + 0x3000) = (1 << 30);
    
    // init board detection
    detect_board();
    
    // clocks & voltage
    init_system();
    
    // fuses counter
    init_fuses();
    
    // LED & glitch & emmc PIO
    upload_pio();
    
    if (is_tiny())
    {
        gpio_put(led_pin(), 0);
        sleep_us(50); // optimized: reduced from 100us
        put_pixel(0);
        sleep_us(50); // optimized: reduced from 100us
    }
    
    // check if this is the very first start
    if (watchdog_caused_reboot() && boot_try == 0)
        halt_with_error(1, 1);
    
    // is chip reset required
    bool force_button = detect_by_pull(1, 0, 1);
    
    // start LED
    put_pixel(PIX_gre);
    
    // test pins
    self_test();
    
    // wait till the CPU has proper power & started reading the eMMC
    wait_for_boot(2490);
    
    // ensure the BCT has not been overwritten by system update
    bool force_check = fast_check();
    was_self_reset = force_button || !is_configured(cid_buf + 1);
    
    // perform payload rewrite if required
    if (!force_check || was_self_reset) {
        rewrite_payload();
    }
    
    // setup the glitch trigger for Mariko
    if (mariko) {
        pio1->instr_mem[gtrig_pio_offset + 4] = pio_encode_nop();
        pio1->instr_mem[gtrig_pio_offset + 5] = pio_encode_nop();
    }
    
    // optimized: start with slightly lower width for faster convergence
    int width = 140;
    bool glitched = false;
    int offset = 0;
    
    // optimized: removed outer loop since it just repeats rewrite_payload
    // try saved records first
    for (int y = 0; (y < 2) && !glitched; y++) {
        int max_weight = -1;
        while (1) {
            offset = find_best_record(&max_weight);
            if (offset == -1)
                break;
            // optimized: reduced attempts from 3 to 2 for faster iteration
            glitched = glitch_try_offset(offset, &width, 2);
            if (glitched)
                break;
        }
    }
    
    // try random offsets if saved records failed
    if (!glitched) {
        for(int z = 0; (z < 2) && !glitched; z++) {
            prepare_random_array();
            for(int y = 0; y < OFFSET_CNT; y++)
            {
                offset = offsets_array[y];
                // optimized: reduced attempts from 4 to 3
                glitched = glitch_try_offset(offset, &width, 3);
                if (glitched)
                    break;
            }
        }
    }
    
    // if still not glitched, try one more time with payload rewrite
    if (!glitched) {
        rewrite_payload();
        
        // one more attempt with saved records
        int max_weight = -1;
        for (int y = 0; (y < 3) && !glitched; y++) {
            offset = find_best_record(&max_weight);
            if (offset == -1)
                break;
            glitched = glitch_try_offset(offset, &width, 2);
        }
    }
    
    if (glitched) {
        // ensure all glitching operations are complete
        sleep_us(100);
        // force success LED display
        put_pixel(0); // clear any previous LED state
        sleep_ms(50);
        put_pixel(PIX_whi); // show white success
        sleep_ms(200); // longer delay to ensure visibility
        
        if ((count_fuses() & 1) != boot_slot)
        {
            // finish update / rollback
            burn_fuse();
        }
        add_boot_record(offset);
        halt_with_error(0, 1);
    }
    
    // attempts limit
    halt_with_error(7, 3);
}