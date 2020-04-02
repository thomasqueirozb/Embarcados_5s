/*
 * oled.c
 *
 * Created: 4/2/2020 11:50:14 AM
 *  Author: Thomas
 */ 

#include "oled.h"

void pisca_led(int n, TickType_t t, Pio* p, uint32_t mask) {
	for (int i = 0; i < n; i++) {
		pio_clear(p, mask);
		vTaskDelay(t);
		pio_set(p, mask);
		vTaskDelay(t);
	}
}