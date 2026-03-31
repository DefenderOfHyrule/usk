#define RGB(r, g, b) (((g) << 16) | ((r) << 8) | (b))

#define PIX_blu RGB(0x16, 0x53, 0x7e)  // blue (glitching)
#define PIX_b   RGB(0x6f, 0xa8, 0xdc)  // light blue dim (glitch pulse)
#define PIX_whi RGB(0xff, 0x92, 0x00)  // amber/yellow (success + comparison)
#define PIX_red RGB(0xc9, 0x00, 0x00)  // red (error codes)

void put_pixel(uint32_t pixel_grb);

void halt_with_error(uint32_t err, uint32_t bits);

void gpio_disable_input_output(int pin);

void gpio_enable_input_output(int pin);

void finish_pins_except_leds();

void reset_cpu();