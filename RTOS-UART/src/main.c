#include <asf.h>
#include <string.h>
#include "conf_board.h"
#include "main.h"
#include "oled.h"
#include "rtos_tasks.h"

// #define c(_a, _b) _a##_b
// #define TASK_CREATE(FUNCTION, NAME)										\
// if (xTaskCreate(FUNCTION, #NAME, c(TASK_##NAME, _STACK_SIZE), NULL,		\
// c(TASK_##NAME, _STACK_PRIORITY), NULL) != pdPASS) {						\
// printf("Failed to create " #NAME " task\r\n");							\
// }


/** Semaforo a ser usado pela task led */
SemaphoreHandle_t xSemaphore;
SemaphoreHandle_t xSemaphore2;
SemaphoreHandle_t xSemaphore3;


/** Queue for IRQ -> taskRx */
QueueHandle_t xQueueRx;

QueueHandle_t xQueueCommand;


/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/

void USART1_Handler(void){
	uint32_t ret = usart_get_status(USART1);

	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	char c;

	// Verifica por qual motivo entrou na interrupçcao
	//  - Dadodisponível para leitura
	if(ret & US_IER_RXRDY){
		usart_serial_getchar(USART1, (uint8_t *) &c);
		xQueueSendFromISR(xQueueRx, (void *) &c, &xHigherPriorityTaskWoken);

	// -  Transmissoa finalizada
	} else if(ret & US_IER_TXRDY){

	}
}

void but1_callback(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	printf("but_callback \n");
	xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
	printf("semafaro tx \n");
}

void but2_callback(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	printf("but2_callback \n");
	xSemaphoreGiveFromISR(xSemaphore2, &xHigherPriorityTaskWoken);
	printf("semafaro2 tx \n");
}

void but3_callback(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	printf("but3_callback \n");
	xSemaphoreGiveFromISR(xSemaphore3, &xHigherPriorityTaskWoken);
	printf("semafaro3 tx \n");
}

static void USART1_init(void){
	/* Configura USART1 Pinos */
	sysclk_enable_peripheral_clock(ID_PIOB);
	sysclk_enable_peripheral_clock(ID_PIOA);
	pio_set_peripheral(PIOB, PIO_PERIPH_D, PIO_PB4); // RX
	pio_set_peripheral(PIOA, PIO_PERIPH_A, PIO_PA21); // TX
	MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;

	/* Configura opcoes USART */
	const sam_usart_opt_t usart_settings = {
		.baudrate       = 115200,
		.char_length    = US_MR_CHRL_8_BIT,
		.parity_type    = US_MR_PAR_NO,
		.stop_bits   	= US_MR_NBSTOP_1_BIT	,
		.channel_mode   = US_MR_CHMODE_NORMAL
	};

	/* Ativa Clock periferico USART0 */
	sysclk_enable_peripheral_clock(ID_USART1);

	stdio_serial_init(CONF_UART, (usart_serial_options_t *) &usart_settings);

	/* Enable the receiver and transmitter. */
	usart_enable_tx(USART1);
	usart_enable_rx(USART1);

	/* map printf to usart */
	ptr_put = (int (*)(void volatile*,char))&usart_serial_putchar;
	ptr_get = (void (*)(void volatile*,char*))&usart_serial_getchar;

	/* ativando interrupcao */
	usart_enable_interrupt(USART1, US_IER_RXRDY);
	NVIC_SetPriority(ID_USART1, 4);
	NVIC_EnableIRQ(ID_USART1);

}


/************************************************************************/
/* main                                                                 */
/************************************************************************/

/**
 *  \brief FreeRTOS Real Time Kernel example entry point.
 *
 *  \return Unused (ANSI-C compatibility).
 */
int main(void){
	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	/* Initialize the console uart */
	// configure_console();
	USART1_init();

	/* Output demo information. */
	printf("-- Freertos Example --\n\r");
	printf("-- %s\n\r", BOARD_NAME);
	printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

	/* Create task to monitor processor activity */
	if (xTaskCreate(task_monitor, "Monitor", TASK_MONITOR_STACK_SIZE, NULL, TASK_MONITOR_STACK_PRIORITY, NULL) != pdPASS)
	{
		printf("Failed to create Monitor task\r\n");
	}

	if (xTaskCreate(task_led, "Led", TASK_LED_STACK_SIZE, NULL,
			TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create test led task\r\n");
	}

	if (xTaskCreate(task_led2, "Led2", TASK_LED2_STACK_SIZE, NULL,
					TASK_LED2_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create test led2 task\r\n");
	}

	if (xTaskCreate(task_led3, "Led3", TASK_LED3_STACK_SIZE, NULL,
					TASK_LED3_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create test led3 task\r\n");
	}

	if (xTaskCreate(task_led1, "Led1", TASK_LED1_STACK_SIZE, NULL,
	TASK_LED1_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create test led1 task\r\n");
	}

	if (xTaskCreate(task_uartRx, "UartRx", TASK_UARTRX_STACK_SIZE, NULL,
					TASK_UARTRX_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create UartRx task\r\n");
	}

	if (xTaskCreate(task_execute, "EXECUTE", TASK_EXECUTE_STACK_SIZE, NULL,
	TASK_EXECUTE_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create EXECUTE task\r\n");
	}


	/* Start the scheduler. */
	vTaskStartScheduler();

	// while(1){	}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}