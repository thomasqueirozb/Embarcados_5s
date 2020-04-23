#include <asf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "conf_board.h"
#include "conf_uart_serial.h"
#include "maxTouch/maxTouch.h"
#include "tfont.h"
#include "digital521.h"
#include "main.h"

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/
void but1_callback(void);


/************************************************************************/
/* LCD + TOUCH                                                          */
/************************************************************************/
#define MAX_ENTRIES        10

struct ili9488_opt_t g_ili9488_display_opt;
const uint32_t BUTTON_W = 120;
const uint32_t BUTTON_H = 150;
const uint32_t BUTTON_BORDER = 2;
const uint32_t BUTTON_X = ILI9488_LCD_WIDTH/2;
const uint32_t BUTTON_Y = ILI9488_LCD_HEIGHT/2;

/************************************************************************/
/* Botoes lcd                                                           */
/************************************************************************/


/************************************************************************/
/* RTOS                                                                  */
/************************************************************************/
#define TASK_MXT_STACK_SIZE            (2*1024/sizeof(portSTACK_TYPE))
#define TASK_MXT_STACK_PRIORITY        (tskIDLE_PRIORITY)

#define TASK_LCD_STACK_SIZE            (4*1024/sizeof(portSTACK_TYPE))
#define TASK_LCD_STACK_PRIORITY        (tskIDLE_PRIORITY)

QueueHandle_t xQueueTouch;

/************************************************************************/
/* handler/callbacks                                                    */
/************************************************************************/

void callback_but0(){
  puts("but0");
}
void callback_but1(){
  puts("but1");
}
void callback_but2(){
  puts("but2");
}

/************************************************************************/
/* RTOS hooks                                                           */
/************************************************************************/

/**
* \brief Called if stack overflow during execution
*/
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName)
{
  printf("stack overflow %x %s\r\n", (unsigned int) pxTask, (portCHAR *)pcTaskName);
  /* If the parameters have been corrupted then inspect pxCurrentTCB to
  * identify which task has overflowed its stack.
  */
  for (;;) {
  }
}

/**
* \brief This function is called by FreeRTOS idle task
*/
extern void vApplicationIdleHook(void)
{
  pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
}

/**
* \brief This function is called by FreeRTOS each tick
*/
extern void vApplicationTickHook(void)
{
}

extern void vApplicationMallocFailedHook(void)
{
  /* Called if a call to pvPortMalloc() fails because there is insufficient
  free memory available in the FreeRTOS heap.  pvPortMalloc() is called
  internally by FreeRTOS API functions that create tasks, queues, software
  timers, and semaphores.  The size of the FreeRTOS heap is set by the
  configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

  /* Force an assert. */
  configASSERT( ( volatile void * ) NULL );
}

/************************************************************************/
/* init                                                                 */
/************************************************************************/

static void configure_lcd(void){
  /* Initialize display parameter */
  g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
  g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
  g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
  g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

  /* Initialize LCD */
  ili9488_init(&g_ili9488_display_opt);
}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

int touched(touchData touch, button_t* botoes, uint32_t n) {
  for (uint32_t i = 0; i < n; i++) {
    const uint32_t bx = botoes[i].x;
    const uint32_t by = botoes[i].y;

    const uint32_t bwh = botoes[i].width  / 2 ; // Half width
    const uint32_t bhh = botoes[i].height / 2;  // Half height
    if(touch.x > (bx - bwh) && touch.x < (bx + bwh) &&
       touch.y > (by - bhh) && touch.y < (by + bhh)) {
      return i;
    }
  }
  return -1;
}
  
void draw_button_t(button_t but){
    uint32_t color;
    if (but.status) color = but.colorOn;
    else color = but.colorOff;
    // printf("on:  %lu\noff: %lu\nnow: %lu\n", but.colorOn, but.colorOff, color);
    
    ili9488_set_foreground_color(COLOR_CONVERT(color));
	const uint32_t bx = but.x;
	const uint32_t by = but.y;

	const uint32_t bwh = but.width  / 2 ; // Half width
	const uint32_t bhh = but.height / 2;  // Half height
    ili9488_draw_filled_rectangle(bx - bwh, by - bhh, bx + bwh, by + bhh);
}

void draw_screen(void) {
  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
  ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
}

void draw_button(uint32_t clicked) {
  static uint32_t last_state = 255; // undefined
  if(clicked == last_state) return;
  
  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
  ili9488_draw_filled_rectangle(BUTTON_X-BUTTON_W/2, BUTTON_Y-BUTTON_H/2, BUTTON_X+BUTTON_W/2, BUTTON_Y+BUTTON_H/2);
  if(clicked) {
    ili9488_set_foreground_color(COLOR_CONVERT(COLOR_TOMATO));
    ili9488_draw_filled_rectangle(BUTTON_X-BUTTON_W/2+BUTTON_BORDER, BUTTON_Y+BUTTON_BORDER, BUTTON_X+BUTTON_W/2-BUTTON_BORDER, BUTTON_Y+BUTTON_H/2-BUTTON_BORDER);
    } else {
    ili9488_set_foreground_color(COLOR_CONVERT(COLOR_GREEN));
    ili9488_draw_filled_rectangle(BUTTON_X-BUTTON_W/2+BUTTON_BORDER, BUTTON_Y-BUTTON_H/2+BUTTON_BORDER, BUTTON_X+BUTTON_W/2-BUTTON_BORDER, BUTTON_Y-BUTTON_BORDER);
  }
  last_state = clicked;
}

uint32_t convert_axis_system_x(uint32_t touch_y) {
  // entrada: 4096 - 0 (sistema de coordenadas atual)
  // saida: 0 - 320
  return ILI9488_LCD_WIDTH - ILI9488_LCD_WIDTH*touch_y/4096;
}

uint32_t convert_axis_system_y(uint32_t touch_x) {
  // entrada: 0 - 4096 (sistema de coordenadas atual)
  // saida: 0 - 320
  return ILI9488_LCD_HEIGHT*touch_x/4096;
}

void update_screen(uint32_t tx, uint32_t ty) {
  if(tx >= BUTTON_X-BUTTON_W/2 && tx <= BUTTON_X + BUTTON_W/2) {
    if(ty >= BUTTON_Y-BUTTON_H/2 && ty <= BUTTON_Y) {
      draw_button(1);
      } else if(ty > BUTTON_Y && ty < BUTTON_Y + BUTTON_H/2) {
      draw_button(0);
    }
  }
}

void font_draw_text(tFont *font, const char *text, int x, int y, int spacing) {
  const char *p = text;
  while(*p != '\0') {
    char letter = *p;
    int letter_offset = letter - font->start_char;
    if(letter <= font->end_char) {
      const tChar *current_char = font->chars + letter_offset;
      ili9488_draw_pixmap(x, y, current_char->image->width, current_char->image->height, current_char->image->data);
      x += current_char->image->width + spacing;
    }
    p++;
  }
}

void mxt_handler(struct mxt_device *device, uint *x, uint *y)
{
  /* USART tx buffer initialized to 0 */
  uint8_t i = 0; /* Iterator */

  /* Temporary touch event data struct */
  struct mxt_touch_event touch_event;
  
  /* first touch only */
  uint first = 0;

  /* Collect touch events and put the data in a string,
  * maximum 2 events at the time */
  do {

    /* Read next next touch event in the queue, discard if read fails */
    if (mxt_read_touch_event(device, &touch_event) != STATUS_OK) {
      continue;
    }
    
    /************************************************************************/
    /* Envia dados via fila RTOS                                            */
    /************************************************************************/
    if(first == 0 ){
      *x = convert_axis_system_x(touch_event.y);
      *y = convert_axis_system_y(touch_event.x);
      first = 1;
    }
    
    i++;

    /* Check if there is still messages in the queue and
    * if we have reached the maximum numbers of events */
  } while ((mxt_is_message_pending(device)) & (i < MAX_ENTRIES));
}

/************************************************************************/
/* tasks                                                                */
/************************************************************************/

void task_mxt(void){
  
  struct mxt_device device; /* Device data container */
  mxt_init(&device);       	/* Initialize the mXT touch device */
  touchData touch;          /* touch queue data type*/
  
  while (true) {
    /* Check for any pending messages and run message handler if any
    * message is found in the queue */
    if (mxt_is_message_pending(&device)) {
      mxt_handler(&device, &touch.x, &touch.y);
      xQueueSend( xQueueTouch, &touch, 0);           /* send mesage to queue */
      vTaskDelay(200);
      
      // limpa touch
      while (mxt_is_message_pending(&device)){
        mxt_handler(&device, NULL, NULL);
        vTaskDelay(50);
      }
    }
    
    vTaskDelay(300);
  }
}

void task_lcd(void){
  xQueueTouch = xQueueCreate( 10, sizeof( touchData ) );
  configure_lcd();
  
  draw_screen();
  
  // Escreve DEMO - BUT no LCD
  // font_draw_text((tFont *)&digital52, "DEMO - BUT", 0, 0, 1);

  button_t but0 = {.width = 120, .height = 75, .colorOn = COLOR_TOMATO,     .colorOff = COLOR_BLACK, .x = ILI9488_LCD_WIDTH/2, .y = 40,  .status = 0, .callback = &callback_but0};
  button_t but1 = {.width = 120, .height = 75, .colorOn = COLOR_BLUEVIOLET, .colorOff = COLOR_BLACK, .x = ILI9488_LCD_WIDTH/2, .y = 140, .status = 0, .callback = &callback_but1};
  button_t but2 = {.width = 120, .height = 75, .colorOn = COLOR_DARKGREEN,  .colorOff = COLOR_BLACK, .x = ILI9488_LCD_WIDTH/2, .y = 240, .status = 0, .callback = &callback_but2};
  
  button_t botoes[] = {but0, but1, but2};
  const size_t n_but = sizeof(botoes)/sizeof(botoes[0]);

  for (size_t i = 0; i < n_but; i++){
    draw_button_t(botoes[i]);
  }

  // strut local para armazenar msg enviada pela task do mxt
  touchData touch;
  
  while (true) {
    if (xQueueReceive( xQueueTouch, &(touch), ( TickType_t )  500 / portTICK_PERIOD_MS)) {
      const int idx = touched(touch, botoes, n_but);
      if (idx != -1) {
        button_t *b = (button_t *)&botoes[idx];
        b->callback();
        b->status = 1 - b->status;
        draw_button_t(*b);
      }
      printf("x:%d y:%d\n", touch.x, touch.y);
    }
  }
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/

int main(void)
{
  /* Initialize the USART configuration struct */
  const usart_serial_options_t usart_serial_options = {
    .baudrate     = USART_SERIAL_EXAMPLE_BAUDRATE,
    .charlength   = USART_SERIAL_CHAR_LENGTH,
    .paritytype   = USART_SERIAL_PARITY,
    .stopbits     = USART_SERIAL_STOP_BIT
  };

  sysclk_init(); /* Initialize system clocks */
  board_init();  /* Initialize board */
  
  /* Initialize stdio on USART */
  stdio_serial_init(USART_SERIAL_EXAMPLE, (const usart_serial_options_t *)&usart_serial_options);
  
  /* Create task to handler touch */
  if (xTaskCreate((TaskFunction_t)task_mxt, "mxt", TASK_MXT_STACK_SIZE, NULL, TASK_MXT_STACK_PRIORITY, NULL) != pdPASS) {
    printf("Failed to create test led task\r\n");
  }
  
  /* Create task to handler LCD */
  if (xTaskCreate((TaskFunction_t)task_lcd, "lcd", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_STACK_PRIORITY, NULL) != pdPASS) {
    printf("Failed to create test led task\r\n");
  }
  
  /* Start the scheduler. */
  vTaskStartScheduler();

  while(1){

  }


  return 0;
}
