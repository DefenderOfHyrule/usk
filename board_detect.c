#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pins.h"
#include "ws2812.pio.h"
#include "misc.h"
#include "board_detect.h"

extern int ws_pio_offset;

enum board_type {
    BOARD_WS = 0,
    BOARD_XO,
    BOARD_IB,
    BOARD_PI,
    BOARD_SQ,
    BOARD_WT
};

enum board_type cur_board = BOARD_WS;

bool detect_by_pull(int frc_pin, int det_pin, bool up)
{
    bool result = false;
    if (frc_pin >= 0)
        gpio_init(frc_pin);
    gpio_init(det_pin);
    if (frc_pin >= 0)
        gpio_set_dir(frc_pin, true);
    gpio_set_pulls(det_pin, up, !up);
    sleep_us(15);
    result = gpio_get(det_pin) ^ up;
    gpio_deinit(det_pin);
    if (frc_pin >= 0)
        gpio_deinit(frc_pin);
    gpio_disable_pulls(det_pin);
    return result;
}

bool test_xiao()
{
    return detect_by_pull(1, 2, 1);
}

bool test_itsy()
{
    return detect_by_pull(3, 2, 1);
}

bool test_pico()
{
    return detect_by_pull(-1, 22, 1);
}

bool test_ws()
{
    return detect_by_pull(-1, 25, 1);
}

bool test_wt()
{
    return detect_by_pull(-1, 16, 0);
}

bool test_sqc()
{
    return detect_by_pull(-1, 17, 1);
}

void detect_board()
{
    gpio_pull_down(PIN_GLI_WS);
    gpio_pull_down(PIN_GLI_PICO);
    gpio_pull_down(PIN_GLI_XIAO);
    gpio_pull_down(PIN_GLI_ITSY);
    gpio_disable_input_output(PIN_RST);
    if (test_ws()) {
        cur_board = BOARD_WS;
    } else if (test_xiao()) {
        cur_board = BOARD_XO;
    } else if (test_itsy()) {
        cur_board = BOARD_IB;
    } else if (test_pico()) {
        cur_board = BOARD_PI;
    } else if (test_sqc()) {
        cur_board = BOARD_SQ;
    } else if (test_wt()) {
        cur_board = BOARD_WT;
    } else {
        cur_board = BOARD_WS;
    }
}

int led_pin()
{
    switch(cur_board){
        case BOARD_XO:
            return PIN_LED_XIAO;
        case BOARD_PI:
            return PIN_LED_PICO;
        case BOARD_IB:
            return PIN_LED_ITSY;
        default:
            return PIN_LED_WS;
    }; 
}

int pwr_pin()
{
    switch(cur_board){
        case BOARD_XO:
            return PIN_LED_PWR_XIAO;
        case BOARD_IB:
            return PIN_LED_PWR_ITSY;
        default:
            return 31;
    }; 
}

int scl_pin()
{
    switch(cur_board){
        case BOARD_XO:
            return PIN_SCL_XIAO;
        case BOARD_IB:
            return PIN_SCL_ITSY;
        case BOARD_PI:
            return PIN_SCL_PICO;
        case BOARD_SQ:
            return PIN_SCL_SQC;
        default:
            return PIN_SCL_WS;
    }; 
}

int sda_pin()
{
    switch(cur_board){
        case BOARD_XO:
            return PIN_SDA_XIAO;
        case BOARD_IB:
            return PIN_SDA_ITSY;
        case BOARD_PI:
            return PIN_SDA_PICO;
        case BOARD_SQ:
            return PIN_SDA_SQC;
        default:
            return PIN_SDA_WS;
    }; 
}

int gli_pin()
{
    switch(cur_board){
        case BOARD_XO:
            return PIN_GLI_XIAO;
        case BOARD_IB:
            return PIN_GLI_ITSY;
        case BOARD_PI:
            return PIN_GLI_PICO;
        default:
            return PIN_GLI_WS;
    }; 
}

bool is_pico()
{
    return cur_board == BOARD_PI;
}

bool is_tiny()
{
    return cur_board == BOARD_WT;
}
