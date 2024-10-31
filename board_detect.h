#include "stdbool.h"

bool detect_by_pull(int frc_pin, int det_pin, bool up);

void detect_board();
int gli_pin();
int pwr_pin();
int led_pin();
bool is_pico();
bool is_tiny();
int scl_pin();
int sda_pin();
