
/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "init.h"
#include "adc.h"
#include "config.h"
#include "i2c.h"
#include "rtc.h"
#include "timers.h"

#include <avr/io.h>

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static void ports_init(void);

/*===========================================================================*/
/* 
* Initialize pin ports and default values of variables
*/
void boot(void)
{
	ports_init();
    timers_init();
    adc_init(TRUE);
	i2c_init();
	rtc_init();

	timer_ms_set(ENABLE);
	timer_sec_set(ENABLE);
}

/*-----------------------------------------------------------------------------
-------------------------- L O C A L   F U N C T I O N S ----------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
static void ports_init(void)
{
	DDRD |= (1<<DDD5);		// DIG_IMP_1
	DDRD |= (1<<DDD6);		// DIG_IMP_2
	DDRD |= (1<<DDD7);		// DIG_IMP_3
	DDRB |= (1<<DDB0);		// DIG_IMP_4

	PORTD &= ~(1<<PORTD5);
	PORTD &= ~(1<<PORTD6);
	PORTD &= ~(1<<PORTD7);
	PORTB &= ~(1<<PORTB0);

	DDRB |= (1<<DDB1);		// DIG_IMP_5
	DDRB |= (1<<DDB2);		// DIG_IMP_6
	DDRB |= (1<<DDB3);		// DIG_IMP_7
	DDRD |= (1<<DDD3);		// DIG_IMP_8

	PORTB &= ~(1<<PORTB1);
	PORTB &= ~(1<<PORTB2);
	PORTB &= ~(1<<PORTB3);
	PORTD &= ~(1<<PORTD3);

	DDRB &= ~(1<<DDB4);		// not used
	DDRB &= ~(1<<DDB5);		// not used
	DDRC &= ~(1<<DDC0);		// not used
	DDRC &= ~(1<<DDC1);		// PSH_BTN (Analog in)
	DDRC &= ~(1<<DDC2);		// not used (light sensor)
	
	DDRC &= ~(1<<DDC4);		// RTC_SDA
	DDRC &= ~(1<<DDC5);		// RTC_SCL

	DDRD |= (1<<DDD0);		// not used, TP1, RXD
	DDRD |= (1<<DDD1);		// not used, TP2, TXD
	DDRD &= ~(1<<DDD2);		// not used
	DDRD |= (1<<DDD4);		// PD4, seconds indicator, not used

	PORTD &= ~(1<<PORTD0);
	PORTD &= ~(1<<PORTD1);	
	PORTD &= ~(1<<PORTD4);
}