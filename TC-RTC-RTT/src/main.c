#include "asf.h"

/************************************************************************/
/* DEFINES                                                              */
/************************************************************************/

// Led Placa
#define LEDP_PIO			PIOC
#define LEDP_PIO_ID			ID_PIOC
#define LEDP_PIN			8
#define LEDP_IDX_MASK		(1 << LEDP_PIN)

#define LED2_PIO			PIOC
#define LED2_PIO_ID			ID_PIOC
#define LED2_PIN			30
#define LED2_IDX_MASK		(1 << LED2_PIN)

#define LED3_PIO			PIOB
#define LED3_PIO_ID			ID_PIOB
#define LED3_PIN			2
#define LED3_IDX_MASK		(1 << LED3_PIN)

typedef struct  {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t week;
	uint32_t hour;
	uint32_t minute;
	uint32_t seccond;
} calendar;

/************************************************************************/
/* VAR globais                                                          */
/************************************************************************/

volatile char flag_tc = 0;
volatile char flag_rtc = 0;
volatile Bool f_rtt_alarme = true;

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type);
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq);
static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses);
void pin_toggle(Pio *pio, uint32_t mask);
void pisca_led(int n, int t, Pio* p, uint32_t mask);

/************************************************************************/
/* Handlers                                                             */
/************************************************************************/

/**
*  Handle Interrupcao botao 1
*/
void TC1_Handler(void){
	volatile uint32_t ul_dummy;

	/****************************************************************
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	******************************************************************/
	ul_dummy = tc_get_status(TC0, 1);

	/* Avoid compiler warning */
	UNUSED(ul_dummy);

	/** Muda o estado do LED */
	flag_tc = 1;
}

void RTT_Handler(void){
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {  }

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		pisca_led(1, 100, LED2_PIO, LED2_IDX_MASK);
//		pin_toggle(LED2_PIO, LED2_IDX_MASK);	// BLINK Led
		f_rtt_alarme = true;						// flag RTT alarme
	}
}

void RTC_Handler(void)
{
	uint32_t ul_status = rtc_get_status(RTC);

	/*
	*  Verifica por qual motivo entrou
	*  na interrupcao, se foi por segundo
	*  ou Alarm
	*/
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
		flag_rtc = 1;
	}
	
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
}

/************************************************************************/
/* Funcoes                                                              */
/************************************************************************/

void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask)) pio_clear(pio, mask);
	else pio_set(pio,mask);
}

/*
* @Brief Pisca LED placa
*/
void pisca_led(int n, int t, Pio* p, uint32_t mask) {
	for (int i = 0; i < n; i++) {
		pio_clear(p, mask);
		delay_ms(t);
		pio_set(p, mask);
		delay_ms(t);
	}
}


/**
* Configura TimerCounter (TC) para gerar uma interrupcao no canal (ID_TC e TC_CHANNEL)
* na taxa de especificada em freq.
*/
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configura o PMC */
	/* O TimerCounter é meio confuso
	o uC possui 3 TCs, cada TC possui 3 canais
	TC0 : ID_TC0, ID_TC1, ID_TC2
	TC1 : ID_TC3, ID_TC4, ID_TC5
	TC2 : ID_TC6, ID_TC7, ID_TC8
	*/
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  4Mhz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura e ativa interrupçcão no TC canal 0 */
	/* Interrupção no C */
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);

	/* Inicializa o canal 0 do TC */
	tc_start(TC, TC_CHANNEL);
}

void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type){
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(rtc, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(rtc, t.year, t.month, t.day, t.week);
	rtc_set_time(rtc, t.hour, t.minute, t.seccond);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(id_rtc);
	NVIC_ClearPendingIRQ(id_rtc);
	NVIC_SetPriority(id_rtc, 0);
	NVIC_EnableIRQ(id_rtc);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(rtc,  irq_type);
}

static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses)
{
	uint32_t ul_previous_time;

	/* Configure RTT for a 1 second tick interrupt */
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);

	ul_previous_time = rtt_read_timer_value(RTT);
	while (ul_previous_time == rtt_read_timer_value(RTT));

	rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);

	/* Enable RTT interrupt */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 0);
	NVIC_EnableIRQ(RTT_IRQn);
	rtt_enable_interrupt(RTT, RTT_MR_ALMIEN);
}


/************************************************************************/
/* Main Code	                                                        */
/************************************************************************/
int main(void){
	/* Initialize the SAM system */
	sysclk_init();

	/* Disable the watchdog */
	WDT->WDT_MR = WDT_MR_WDDIS;

	/* Configura Leds */
	pmc_enable_periph_clk(LED2_PIO_ID);
	pio_set_output(LED2_PIO, LED2_IDX_MASK, 1, 0, 0);
	pmc_enable_periph_clk(LEDP_PIO_ID);
	pio_set_output(LEDP_PIO, LEDP_IDX_MASK, 1, 0, 0);
	pmc_enable_periph_clk(LED3_PIO_ID);
	pio_set_output(LED3_PIO, LED3_IDX_MASK, 1, 0, 0);


	/* Configura timer TC0, canal 1 */
	TC_init(TC0, ID_TC1, 1, 4);

	/* RTC */
	calendar rtc_initial = {2018, 3, 19, 12, 15, 45 ,1};
	RTC_init(RTC, ID_RTC, rtc_initial, RTC_IER_ALREN);
	rtc_set_date_alarm(RTC, 1, rtc_initial.month, 1, rtc_initial.day);
	rtc_set_time_alarm(RTC, 1, rtc_initial.hour, 1, rtc_initial.minute, 1, rtc_initial.seccond + 20);
	
	while (1) {
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);

		if(flag_tc) {
			pin_toggle(LED3_PIO, LED3_IDX_MASK);
			//pisca_led(1, 50, LED3_PIO, LED3_IDX_MASK);
			flag_tc = 0;
		}
				
		if (f_rtt_alarme) {

			/*
			* IRQ apos 4s -> 8*0.5
			*/
			uint16_t pllPreScale = (int) (((float) 32768) / 4.0);
			uint32_t irqRTTvalue = 8;

			// reinicia RTT para gerar um novo IRQ
			RTT_init(pllPreScale, irqRTTvalue);

			f_rtt_alarme = false;
		}
		if (flag_rtc) {  // Placa
			pisca_led(5, 100, LEDP_PIO, LEDP_IDX_MASK);
			flag_rtc = 0;
		}
	}
}