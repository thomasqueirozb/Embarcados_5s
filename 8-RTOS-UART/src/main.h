#ifndef MAIN_H_
#define MAIN_H_
#include <asf.h>

void but1_callback(void);
void but2_callback(void);
void but3_callback(void);

extern void xPortSysTickHandler(void);
uint32_t usart1_puts(uint8_t *pstring);


#endif /* MAIN_H_ */