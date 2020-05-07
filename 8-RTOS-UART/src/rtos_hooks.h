#ifndef RTOS_HOOKS_H_
#define RTOS_HOOKS_H_
#include <asf.h>

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);

#endif /* RTOS_HOOKS_H_ */