#define PIX_gre 0x8000ff
#define PIX_red 0xc90000
#define PIX_whi 0x00ffff

#define PIX_g 0xff0080

void put_pixel(uint32_t pixel_grb);

void halt_with_error(uint32_t err, uint32_t bits);

void gpio_disable_input_output(int pin);

void gpio_enable_input_output(int pin);

void finish_pins_except_leds();

void reset_cpu();
