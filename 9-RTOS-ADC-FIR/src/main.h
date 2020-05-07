#ifndef MAIN_H__
#define MAIN_H__

#define c(_a, _b) _a##_b
#define TASK_CREATE(FUNCTION, NAME)                                            \
    if (xTaskCreate((TaskFunction_t) FUNCTION, #NAME, c(TASK_##NAME, _STACK_SIZE), NULL,        \
                    c(TASK_##NAME, _STACK_PRIORITY), NULL) != pdPASS) {        \
        printf("Failed to create " #NAME " task\r\n");                         \
    }

/************************************************************************/
/* RTOS                                                                  */
/************************************************************************/
#define TASK_MXT_STACK_SIZE            (2*1024/sizeof(portSTACK_TYPE))
#define TASK_MXT_STACK_PRIORITY        (tskIDLE_PRIORITY)

#define TASK_ADC_STACK_SIZE            (2*1024/sizeof(portSTACK_TYPE))
#define TASK_ADC_STACK_PRIORITY        (tskIDLE_PRIORITY)

#define TASK_LCD_STACK_SIZE            (6*1024/sizeof(portSTACK_TYPE))
#define TASK_LCD_STACK_PRIORITY        (tskIDLE_PRIORITY)

/************************************************************************/
/* LCD + TOUCH                                                          */
/************************************************************************/
#define MAX_ENTRIES        3

/************************************************************************/
/* AFEC                                                                 */
/************************************************************************/
#define AFEC_POT AFEC1
#define AFEC_POT_ID ID_AFEC1
#define AFEC_POT_CHANNEL 6 // Canal do pino PC31


#define BLOCK_SIZE 1   // Sem processamento por blocos.
#define NUM_TAPS   8

typedef struct {
	uint x;
	uint y;
} touchData;

typedef struct {
	uint value;
} adcData;

typedef struct {
	uint raw;
	uint filtrado;
} t_plot;

const float32_t firCoeffs32[NUM_TAPS] ={
	0.12269166637219883,
	0.12466396327768503,
	0.1259892807712678,
	0.12665508957884833,
	0.12665508957884833,
	0.1259892807712678,
	0.12466396327768503,
	0.12269166637219883
};

#endif
