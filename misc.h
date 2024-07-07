#define PIX_gre 0xFF9900
#define PIX_red 0xFF0000
#define PIX_whi 0x111111

#define PIX_b 0x00000F

void put_pixel(uint32_t pixel_grb);

void halt_with_error(uint32_t err, uint32_t bits);

void gpio_disable_input_output(int pin);

void gpio_enable_input_output(int pin);

void finish_pins_except_leds();

void reset_cpu();
