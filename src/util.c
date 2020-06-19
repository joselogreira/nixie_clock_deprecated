/**
 * @file util.c
 * @brief Miscelaneous functions
 *
 * @author Jose Logreira
 * @date 07.05.2018
 *
 */
/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "util.h"

#include <stdint.h> 
#include <avr/io.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

// Button time counts: These macros determine the time it takes for 
// different flags within btnXYZ structure to be set (in milliseconds)
#define BTN_DTCT_TIME   7		// Detect time to assume the button is pressed
#define BTN_LOCK_TIME	30		// lock time after button released
#define BTN_DLY1_TIME	300		// time for delay 1
#define BTN_DLY2_TIME	65		// time for delay 2
#define BTN_DLY3_TIME	2000	// time for delay 3
#define BTN_BEEP_TIME	50		// duration of beep sound

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static void tubes_off(void);

/*===========================================================================*/
/*
* Sets only one tubes' anode
*/
void set_tube(uint8_t t)
{
	tubes_off();
	if(t == TUBE_A) PORTB &= ~(1<<PORTB1);
	else if (t == TUBE_B) PORTB &= ~(1<<PORTB2);
	else if (t == TUBE_C) PORTB &= ~(1<<PORTB3);
	else if (t == TUBE_D) PORTD &= ~(1<<PORTD3);
}

/*===========================================================================*/
/*
* Sets only one tubes' cathode
*/
void set_digit(uint8_t n)
{
	switch (n) {
		case 0:
			PORTD &= ~(1<<PORTD5);
			PORTD &= ~(1<<PORTD6);
			PORTD &= ~(1<<PORTD7);
			PORTB &= ~(1<<PORTB0);
			break;
		case 9:
			PORTD |= (1<<PORTD5);
			PORTD &= ~(1<<PORTD6);
			PORTD &= ~(1<<PORTD7);
			PORTB &= ~(1<<PORTB0);
			break;
		case 8:
			PORTD &= ~(1<<PORTD5);
			PORTD |= (1<<PORTD6);
			PORTD &= ~(1<<PORTD7);
			PORTB &= ~(1<<PORTB0);
			break;
		case 7:
			PORTD |= (1<<PORTD5);
			PORTD |= (1<<PORTD6);
			PORTD &= ~(1<<PORTD7);
			PORTB &= ~(1<<PORTB0);
			break;
		case 6:
			PORTD &= ~(1<<PORTD5);
			PORTD &= ~(1<<PORTD6);
			PORTD |= (1<<PORTD7);
			PORTB &= ~(1<<PORTB0);
			break;
		case 5:
			PORTD |= (1<<PORTD5);
			PORTD &= ~(1<<PORTD6);
			PORTD |= (1<<PORTD7);
			PORTB &= ~(1<<PORTB0);
			break;
		case 4:
			PORTD &= ~(1<<PORTD5);
			PORTD |= (1<<PORTD6);
			PORTD |= (1<<PORTD7);
			PORTB &= ~(1<<PORTB0);
			break;
		case 3:
			PORTD |= (1<<PORTD5);
			PORTD |= (1<<PORTD6);
			PORTD |= (1<<PORTD7);
			PORTB &= ~(1<<PORTB0);
			break;
		case 2:
			PORTD &= ~(1<<PORTD5);
			PORTD &= ~(1<<PORTD6);
			PORTD &= ~(1<<PORTD7);
			PORTB |= (1<<PORTB0);
			break;
		case 1:
			PORTD |= (1<<PORTD5);
			PORTD &= ~(1<<PORTD6);
			PORTD &= ~(1<<PORTD7);
			PORTB |= (1<<PORTB0);
			break;
		default:
			PORTD |= (1<<PORTD5);
			PORTD |= (1<<PORTD6);
			PORTD |= (1<<PORTD7);
			PORTB |= (1<<PORTB0);
			break;
	}
}

/*===========================================================================*/
/*
* Random number algorithm.
* Based on a seed, these constant parameters selection makes the output to
* go through all numbers 0 to 9 before they start repeating.
* Can't remember where I took it, but it works nicely.
*/
uint8_t random_number(uint8_t seed)
{
	uint8_t rand;
	rand = ((4 * seed) + 1) % 9;
	return rand;
}

/*===========================================================================*/
void key_check(uint8_t key, volatile btn_s *btn)
{
	switch(btn->state){

		case BTN_IDLE:
			if(key == btn->n) btn->count++;
			else if(btn->count > 0) btn->count--;

			if(btn->count == BTN_DTCT_TIME){
				btn->action = TRUE;
				btn->lock = TRUE;
				btn->state = BTN_PUSHED;
				btn->count = 0;
			} else if(btn->count == 0){
				btn->action = FALSE;
				btn->lock = FALSE;
				btn->query = FALSE;
			}
			break;

		case BTN_PUSHED:
			if(key == btn->n){
				btn->count++;	
			} else {
				btn->count = 0;
				btn->state = BTN_RELEASED;
			}
			if(btn->count == BTN_DLY1_TIME) btn->delay1 = TRUE;
			if((btn->delay1) && (!(btn->count % BTN_DLY2_TIME))) btn->delay2 = TRUE;
			if(btn->count >= BTN_DLY3_TIME) btn->delay3 = TRUE;
			break;

		case BTN_RELEASED:
			if(key != btn->n)
				btn->count++;
			if(btn->count == BTN_LOCK_TIME){
				btn->query = FALSE;
				btn->action = FALSE;
				btn->lock = FALSE;
				btn->state = BTN_IDLE;
				btn->count = 0;
				btn->delay1 = FALSE;
				btn->delay2 = FALSE;
				btn->delay3 = FALSE;
			}
			break;
		default:
			break;
	}
}

/*-----------------------------------------------------------------------------
--------------------- I N T E R N A L   F U N C T I O N S ---------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*
* Disables all tubes' anodes
*/
static void tubes_off(void)
{
	PORTB |= (1<<PORTB1);
	PORTB |= (1<<PORTB2);
	PORTB |= (1<<PORTB3);
	PORTD |= (1<<PORTD3);
}