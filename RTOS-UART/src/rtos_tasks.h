#ifndef RTOS_TASKS_H_
#define RTOS_TASKS_H_

/** RTOS  */
#define TASK_MONITOR_STACK_SIZE (2048 / sizeof(portSTACK_TYPE))
#define TASK_MONITOR_STACK_PRIORITY (tskIDLE_PRIORITY)
#define TASK_LED_STACK_SIZE (1024 / sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY (tskIDLE_PRIORITY)

#define TASK_LED1_STACK_SIZE (1024 / sizeof(portSTACK_TYPE))
#define TASK_LED1_STACK_PRIORITY (tskIDLE_PRIORITY)

#define TASK_LED2_STACK_SIZE (1024 / sizeof(portSTACK_TYPE))
#define TASK_LED2_STACK_PRIORITY (tskIDLE_PRIORITY)

#define TASK_LED3_STACK_SIZE (1024 / sizeof(portSTACK_TYPE))
#define TASK_LED3_STACK_PRIORITY (tskIDLE_PRIORITY)

// UART RX
#define TASK_UARTRX_STACK_SIZE (1024 / sizeof(portSTACK_TYPE))
#define TASK_UARTRX_STACK_PRIORITY (tskIDLE_PRIORITY)

// EXECUTE
#define TASK_EXECUTE_STACK_SIZE (1024 / sizeof(portSTACK_TYPE))
#define TASK_EXECUTE_STACK_PRIORITY (tskIDLE_PRIORITY)

void task_led(void *pvParameters);
void task_led1(void *pvParameters);
void task_led2(void *pvParameters);
void task_led3(void *pvParameters);
void task_monitor(void *pvParameters);
void task_execute(void *pvParameters);
void task_uartRx(void *pvParameters);


#endif /* RTOS_TASKS_H_ */