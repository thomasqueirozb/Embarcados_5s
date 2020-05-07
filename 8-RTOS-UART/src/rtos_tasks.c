#include "rtos_tasks.h"
#include "oled.h"
#include "main.h"
#include <string.h>


#include <asf.h>
extern SemaphoreHandle_t xSemaphore;
extern SemaphoreHandle_t xSemaphore2;
extern SemaphoreHandle_t xSemaphore3;
extern QueueHandle_t xQueueCommand;
extern QueueHandle_t xQueueRx;

void task_led(void *pvParameters) {
	xSemaphore = xSemaphoreCreateBinary();

	/* init bot�o */
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP);
	pio_handler_set(BUT1_PIO, BUT1_PIO_ID, BUT1_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but1_callback);
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 4

	if (xSemaphore == NULL)
	printf("falha em criar o semaforo \n");

	for (;;)
	{
		if (xSemaphoreTake(xSemaphore, (TickType_t)500) == pdTRUE){
			LED_Toggle(LED0);
		}
	}
}

void task_led1(void *pvParameters){
	pmc_enable_periph_clk(LED1_PIO_ID);
	pio_configure(LED1_PIO, PIO_OUTPUT_0, LED1_IDX_MASK, PIO_DEFAULT);
	pio_set(LED1_PIO, LED1_IDX_MASK);

	/* Block for 2000ms. */
	const TickType_t xDelay = 3000 / portTICK_PERIOD_MS;
	const TickType_t xDelayLed = 50 / portTICK_PERIOD_MS;

	for (;;){
		pin_toggle(LED1_PIO, LED1_IDX_MASK);
		vTaskDelay(xDelayLed);
		pin_toggle(LED1_PIO, LED1_IDX_MASK);
		vTaskDelay(xDelayLed);

		pin_toggle(LED1_PIO, LED1_IDX_MASK);
		vTaskDelay(xDelayLed);
		pin_toggle(LED1_PIO, LED1_IDX_MASK);
		vTaskDelay(xDelayLed);

		pin_toggle(LED1_PIO, LED1_IDX_MASK);
		vTaskDelay(xDelayLed);
		pin_toggle(LED1_PIO, LED1_IDX_MASK);
		vTaskDelay(xDelayLed);

		vTaskDelay(xDelay);
	}
}

void task_led2(void *pvParameters){
	xSemaphore2 = xSemaphoreCreateBinary();

	/* init bot�o */
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP);
	pio_handler_set(BUT2_PIO, BUT2_PIO_ID, BUT2_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but2_callback);
	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 4); // Prioridade 4

	pmc_enable_periph_clk(LED2_PIO_ID);
	pio_configure(LED2_PIO, PIO_OUTPUT_0, LED2_IDX_MASK, PIO_DEFAULT);
	pio_set(LED2_PIO, LED2_IDX_MASK);

	if (xSemaphore2 == NULL)
	printf("falha em criar o semaforo \n");

	const TickType_t xDelayLed = 100 / portTICK_PERIOD_MS;

	for (;;)
	{
		if (xSemaphoreTake(xSemaphore2, (TickType_t)500) == pdTRUE)
		{
			pin_toggle(LED2_PIO, LED2_IDX_MASK);
			vTaskDelay(xDelayLed);
		}
	}
}

void task_led3(void *pvParameters){
	xSemaphore3 = xSemaphoreCreateBinary();

	/* init bot�o */
	pmc_enable_periph_clk(BUT3_PIO_ID);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP);
	pio_handler_set(BUT3_PIO, BUT3_PIO_ID, BUT3_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but3_callback);
	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 4); // Prioridade 4

	pmc_enable_periph_clk(LED3_PIO_ID);
	pio_configure(LED3_PIO, PIO_OUTPUT_0, LED3_IDX_MASK, PIO_DEFAULT);
	pio_set(LED3_PIO, LED3_IDX_MASK);

	if (xSemaphore3 == NULL)
	printf("falha em criar o semaforo \n");

	const TickType_t xDelayLed = 100 / portTICK_PERIOD_MS;

	for (;;)
	{
		if (xSemaphoreTake(xSemaphore3, (TickType_t)500) == pdTRUE)
		{
			pin_toggle(LED3_PIO, LED3_IDX_MASK);
			vTaskDelay(xDelayLed);
		}
	}
}


void task_monitor(void *pvParameters){
	static portCHAR szList[256];
	UNUSED(pvParameters);

	/* Block for 3000ms. */
	const TickType_t xDelay = 3000 / portTICK_PERIOD_MS;

	for (;;) {
		printf("--- task ## %u\n", (unsigned int)uxTaskGetNumberOfTasks());
		vTaskList((signed portCHAR *)szList);
		printf(szList);
		vTaskDelay(xDelay);
	}
}


void task_execute(void *pvParameters)
{
	char msgBuffer[64];

	xQueueCommand = xQueueCreate(5, sizeof(char[64]));

	while (1)
	{
		if (xQueueReceive(xQueueCommand, &msgBuffer, (TickType_t)500))
		{
			printf("comando: %s\n", msgBuffer);

			if (strcmp(msgBuffer, "led 1 toggle") == 0) {
				pin_toggle(LED1_PIO, LED1_IDX_MASK);
			}

			if (strcmp(msgBuffer, "led 3 toggle") == 0) {
				pin_toggle(LED3_PIO, LED3_IDX_MASK);
			}
		}
	}
}

void task_uartRx(void *pvParameters)
{
	char rxMSG;
	char msgBuffer[64] = {0};
	int i = 0;

	xQueueRx = xQueueCreate(32, sizeof(char));

	while (1)
	{
		if (xQueueReceive(xQueueRx, &rxMSG, (TickType_t)500))
		{
			printf("recebeu: %c\n", rxMSG);

			if (rxMSG != '\n')
			{
				msgBuffer[i] = rxMSG;
				i++;
			}
			else
			{
				msgBuffer[i] = 0;
				xQueueSend(xQueueCommand, &msgBuffer, 0);
				i = 0;
			}
		}
	}
}
