/*
 * main.h
 *
 * Created: 4/23/2020 11:33:00 AM
 *  Author: tqbs2
 */ 


#ifndef MAIN_H_
#define MAIN_H_
#include <asf.h>

typedef struct {
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t colorOn;
	uint32_t colorOff;
	void     (*callback)(void);
	uint8_t  status;
} button_t;

typedef struct {
	uint x;
	uint y;
} touchData;


void task_lcd(void);
void task_mxt(void);
void mxt_handler(struct mxt_device *device, uint *x, uint *y);
void font_draw_text(tFont *font, const char *text, int x, int y, int spacing);
uint32_t convert_axis_system_y(uint32_t touch_x);
uint32_t convert_axis_system_x(uint32_t touch_y);
void draw_screen(void);
void draw_button_t(button_t but);
int process_touch(touchData touch, button_t* botoes, uint32_t n);

void callback_but0(void);
void callback_but1(void);
void callback_but2(void);

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName);
extern void vApplicationMallocFailedHook(void);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);

#endif /* MAIN_H_ */