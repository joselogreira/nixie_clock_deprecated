
/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "timers.h"
#include "config.h"
#include "rtc.h"
#include "util.h"

#include <avr/interrupt.h>

/******************************************************************************
*************** G L O B A L   V A R S   D E F I N I T I O N S *****************
******************************************************************************/

volatile display_s 	display;

volatile uint8_t loop = FALSE;

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

/*===========================================================================*/
void timers_init(void)
{
	/* TIMER COUNTER 0 */
	TCCR0A |= (1<<WGM01);	// CTC mode, TOP: OCR0A
	TCNT0 = 0;
	OCR0A = 250;			// 250 -> isr freq = 16MHz/64/250 = 1KHz
	TIFR0 |= (1<<OCF0A);	// clear interrupt flag, if set.
	TIMSK0 |= (1<<OCIE0A);	// Interrupts for compare match

	/* TIMER COUNTER 1 (16 bits) */
	TCCR1B |= (1<<WGM12);	// CTC mode, TOP: OCR1A
	TCNT1 = 0;
	OCR1A = 15624;			// 15625 -> isr freq = 16MHz/1024/15625 = 1Hz
	TIFR1 |= (1<<OCF1A);	// clear interrupt flag, if set.
	TIMSK1 |= (1<<OCIE1A);	// Interrupts for compare match

	// Display handler init
	display.mode = ON;
	display.d1 = 0;
	display.d2 = 0;
	display.d3 = 0;
	display.d4 = 0;
}

/*===========================================================================*/
void timer_ms_set(uint8_t state)
{
	if (state) {
		TIMSK0 |= (1<<OCIE0A);
		TCNT0 = 0;
		TCCR0B |= (1<<CS01) | (1<<CS00);
	} else {
		TCCR0B &= ~((1<<CS02) | (1<<CS01) | (1<<CS00));
		TIMSK0 &= ~(1<<OCIE0A);
	}
}

/*===========================================================================*/
void timer_sec_set(uint8_t state)
{
	if (state) {
		TIMSK1 |= (1<<OCIE1A);
		TCNT1 = 0;
		TCCR1B |= (1<<CS12) | (1<<CS10);
	} else {
		TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10));
		TIMSK1 &= ~(1<<OCIE1A);
	}
}

/*===========================================================================*/
volatile display_s * timer_get_display_handler(void)
{
	return &display;
}

/*===========================================================================*/
volatile uint8_t * timer_get_loop_flag(void)
{
	return &loop;
}

/******************************************************************************
********************* I N T E R R U P T   H A N D L E R S *********************
******************************************************************************/

/*===========================================================================*/
/*
* TIMER 3 is used as a general purpose counter. Interrupts are generated every
* 1ms and this time base is used for multiple purposes:
* - loop flag is set in every execution
* - Nixie tubes multiplexing routine is handled based on an internal counter
* - Nixie tubes fading routine is handled based on an internal counter
*/
ISR (TIMER0_COMPA_vect)
{
	static uint8_t n_tube = 1;   // determines which tube to light up (1, 2, 3 or 4)
	static uint8_t n_fade = 5;      // 4 Fading levels (1 to 4)
	                                // 0: fully off; 5: fully on
	static uint16_t cnt = 0;        // general purpose counter

	// execute main loop every 1ms.
	loop = TRUE;

    // change tube selection
    n_tube++;
    if(n_tube >= 4) n_tube = 0;
    // enable tube anode
    set_tube(n_tube);

    if(display.mode){
        if(n_tube == TUBE_D) set_digit(display.d1);
        else if(n_tube == TUBE_C) set_digit(display.d2);
        else if(n_tube == TUBE_B) set_digit(display.d3);
        else if(n_tube == TUBE_A) set_digit(display.d4);
    } else {
        set_digit(BLANK);
    }
          
    // general counter reset
    cnt++;
    if(cnt == 1000) cnt = 0;
    // fade level counter
    n_fade++;
    if(n_fade > 5) n_fade = 1;
	
}

/*===========================================================================*/
ISR (TIMER1_COMPA_vect)
{
	rtc_read_time();
}