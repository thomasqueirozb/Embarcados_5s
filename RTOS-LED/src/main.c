/*
* Example RTOS Atmel Studio
*/

#define TASK_LED1_STACK_SIZE (1024/sizeof(portSTACK_TYPE))
#define TASK_LED1_STACK_PRIORITY (tskIDLE_PRIORITY)


#include <asf.h>
#include "oled.h"
#include "conf_board.h"

#define TASK_MONITOR_STACK_SIZE            (2048/sizeof(portSTACK_TYPE))
#define TASK_MONITOR_STACK_PRIORITY        (tskIDLE_PRIORITY)
#define TASK_LED_STACK_SIZE                (1024/sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY            (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
		signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

SemaphoreHandle_t xSemaphore1;

/**
* callback do botao
* libera semaforo: xSemaphore
*/
void but1_callback(){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	printf("but_callback \n");
	xSemaphoreGiveFromISR(xSemaphore1, &xHigherPriorityTaskWoken);
	printf("semafaro tx \n");
}

/**
 * \brief Called if stack overflow during execution
 */
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
		signed char *pcTaskName)
{
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
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

/**
 * \brief This task, when activated, send every ten seconds on debug UART
 * the whole report of free heap and total tasks status
 */
static void task_monitor(void *pvParameters)
{
	static portCHAR szList[256];
	UNUSED(pvParameters);
	const TickType_t xDelay = 3000 / portTICK_PERIOD_MS;

	for (;;) {
		printf("--- task ## %u\n", (unsigned int)uxTaskGetNumberOfTasks());
		vTaskList((signed portCHAR *)szList);
		printf(szList);
		vTaskDelay(xDelay);
	}
}

/**
 * \brief This task, when activated, make LED blink at a fixed rate
 */
static void task_led1(void *pvParameters) {
	UNUSED(pvParameters);
	pmc_enable_periph_clk(LED1_PIO_ID);
	pio_configure(LED1_PIO, PIO_OUTPUT_0, LED1_PIO_IDX_MASK, PIO_DEFAULT);
	const TickType_t xDelay = 3000 / portTICK_PERIOD_MS;
	const TickType_t pDelay = 50 / portTICK_PERIOD_MS;

	for (;;) {
		pisca_led(3, pDelay, LED1_PIO, LED1_PIO_IDX_MASK);
		
// 		for (int i = 0; i < 5; i++) {
// 			pio_clear(LED1_PIO, LED1_PIO_IDX_MASK);
// 			vTaskDelay(pDelay);
// 			pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
// 			vTaskDelay(pDelay);
			vTaskDelay(xDelay);
	}
}

// static void task_led(void *pvParameters)
// {
// 	const TickType_t xDelay = 2000 / portTICK_PERIOD_MS;
// 	UNUSED(pvParameters);
// 	for (;;) {
// 		LED_Toggle(LED0);
// 		vTaskDelay(xDelay);
// 	}
// }
static void task_led(void *pvParameters) {
  /* We are using the semaphore for synchronisation so we create a binary
  semaphore rather than a mutex.  We must make sure that the interrupt
  does not attempt to use the semaphore before it is created! */
  xSemaphore1 = xSemaphoreCreateBinary();

  /* devemos iniciar a interrupcao no pino somente apos termos alocado
  os recursos (no caso semaforo), nessa funcao inicializamos 
  o botao e seu callback*/
  /* init bot�o */
  pmc_enable_periph_clk(BUT1_PIO_ID);
  pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP);
  pio_handler_set(BUT1_PIO, BUT1_PIO_ID, BUT1_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but1_callback);
  pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
  NVIC_EnableIRQ(BUT1_PIO_ID);
  NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 4

  if (xSemaphore1 == NULL)
    printf("falha em criar o semaforo \n");

  for (;;) {
    if( xSemaphoreTake(xSemaphore1, ( TickType_t ) 500) == pdTRUE ){
      LED_Toggle(LED0);
    }
  }
}


/**
 * \brief Configure the console UART.
 */
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
#if (defined CONF_UART_CHAR_LENGTH)
		.charlength = CONF_UART_CHAR_LENGTH,
#endif
		.paritytype = CONF_UART_PARITY,
#if (defined CONF_UART_STOP_BITS)
		.stopbits = CONF_UART_STOP_BITS,
#endif
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
#if defined(__GNUC__)
	setbuf(stdout, NULL);
#else
	/* Already the case in IAR's Normal DLIB default configuration: printf()
	 * emits one character at a time.
	 */
#endif
}

/**
 *  \brief FreeRTOS Real Time Kernel example entry point.
 *
 *  \return Unused (ANSI-C compatibility).
 */
int main(void)
{
	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	/* Initialize the console uart */
	configure_console();

	/* Output demo information. */
	printf("-- Freertos Example --\n\r");
	printf("-- %s\n\r", BOARD_NAME);
	printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);


	/* Create task to monitor processor activity */
	if (xTaskCreate(task_monitor, "Monitor", TASK_MONITOR_STACK_SIZE, NULL,
			TASK_MONITOR_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create Monitor task\r\n");
	}
	if (xTaskCreate(task_led1, "Led1", TASK_LED1_STACK_SIZE, NULL, TASK_LED1_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create Monitor task\r\n");
	}

	/* Create task to make led blink */
	if (xTaskCreate(task_led, "Led", TASK_LED_STACK_SIZE, NULL,
			TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create test led task\r\n");
	}

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}
