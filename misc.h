#ifndef MISC_H_
#define MISC_H_

#define PIX_gre 0x00FF00
#define PIX_red 0xFF0000
#define PIX_whi 0x111111

#define PIX_g 0x008000

#include <stdint.h>

void put_pixel(uint32_t pixel_grb);
void halt_with_error(uint32_t err, uint32_t bits);
void gpio_disable_input_output(int pin);
void gpio_enable_input_output(int pin);
void finish_pins_except_leds();
void reset_cpu();

#endif /* MISC_H_ */

