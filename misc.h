#define PIX_gre 0x16537e
#define PIX_red 0xc90000
#define PIX_whi 0xff9200

#define PIX_g 0x6fa8dc

void put_pixel(uint32_t pixel_grb);

void halt_with_error(uint32_t err, uint32_t bits);

void gpio_disable_input_output(int pin);

void gpio_enable_input_output(int pin);

void finish_pins_except_leds();

void reset_cpu();
