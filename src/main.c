
/******************************************************************************
******************* I N C L U D E   D E P E N D E N C I E S *******************
******************************************************************************/

#include "config.h"
#include "adc.h"
#include "init.h"
#include "rtc.h"
#include "timers.h"
#include "util.h"

#include <avr/interrupt.h>

/******************************************************************************
********************* F U S E S   D E F I N I T I O N S ***********************
******************************************************************************/

/*
* These fuse settings will be placed in a special section in the ELF output 
* file, after linking. Programming tools can take advantage of the fuse info
* embedded in the ELF file, by extracting this information and determining if 
* the fuses need to be programmed before programming the Flash and EEPROM 
* memories. This also allows a single ELF file to contain all the information 
* needed to program an AVR.
*/
#define FUSE_BITS_LOW       (0xFF)                             							// 0xFF
#define FUSE_BITS_HIGH      (FUSE_SPIEN & FUSE_EESAVE & FUSE_BOOTSZ1 & FUSE_BOOTSZ0)    // 0xD1
#define FUSE_BITS_EXTENDED  (FUSE_BODLEVEL1 & FUSE_BODLEVEL0)		                    // 0xFC
#define LOCK_BITS           (0xFF)                                                      // No Locks
// Place fuses in a special section (.fuse) in the .ELF output file
FUSES = {
    FUSE_BITS_LOW,              // .low
    FUSE_BITS_HIGH,             // .high
    FUSE_BITS_EXTENDED,         // .extended
};
//Place lockbits in a special section (.lock) in the .ELF output file
LOCKBITS = LOCK_BITS;

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

#define MODE_0 		0x00
#define MODE_1 		0x01

/******************************************************************************
*************************** M A I N   P R O G R A M ***************************
******************************************************************************/

int main(void)
{
	boot();
	
	uint8_t display_mode = MODE_0;
	uint8_t key;

	volatile uint8_t *loop = timer_get_loop_flag();
	volatile display_s *display = timer_get_display_handler();
	volatile time_s *time = rtc_get_time_handler();
	volatile btn_s *btn1 = adc_get_button_handler(1);
	volatile btn_s *btn2 = adc_get_button_handler(2);
	volatile btn_s *btn3 = adc_get_button_handler(3);
	volatile btn_s *btn4 = adc_get_button_handler(4);
	
	// change hour mode (12h/24h)
	// if any key is pressed at startup, change hour mode.
	if (adc_key_press() < 5)
		rtc_change_hour_mode();
	// Wait 'til key is released
	while(adc_key_press() != 5);

	// First clock read
	rtc_read_time();

	// Main Infinite Loop
	while(TRUE) {

		switch (display_mode) {

			case MODE_0:
				display->d1 = time->h_tens;
				display->d2 = time->h_units;	
				display->d3 = time->m_tens;
				display->d4 = time->m_units;
				break;
			
			case MODE_1:
				// some other states or animations
				break;

			default:
				break;
		}

		// Continuously read buttons:
		key = adc_key_press();
		key_check(key, btn1);
		key_check(key, btn2);
		key_check(key, btn3);
		key_check(key, btn4);
		/*-----------------------------------*/
		// button 1 press and hold
		if((btn1->action) && (btn1->state == BTN_PUSHED) && (btn1->delay2)){
			btn1->delay2 = FALSE;
			rtc_change_minutes(DOWN);
		}
		// button 1 short press
		if((btn1->action) && (btn1->state == BTN_RELEASED) && (!btn1->delay1)){
			btn1->action = FALSE;
			rtc_change_minutes(DOWN);
		}
		/*-----------------------------------*/
		// button 2 press and hold
		if((btn2->action) && (btn2->state == BTN_PUSHED) && (btn2->delay2)){
			btn2->delay2 = FALSE;
			rtc_change_minutes(UP);
		}
		// button 2 short press
		if((btn2->action) && (btn2->state == BTN_RELEASED) && (!btn2->delay1)){
			btn2->action = FALSE;
			rtc_change_minutes(UP);
		}
		/*-----------------------------------*/
		// button 3 press and hold
		if((btn3->action) && (btn3->state == BTN_PUSHED) && (btn3->delay2)){
			btn3->delay2 = FALSE;
			rtc_change_hours(DOWN);
		}
		// button 3 short press
		if((btn3->action) && (btn3->state == BTN_RELEASED) && (!btn3->delay1)){
			btn3->action = FALSE;
			rtc_change_hours(DOWN);
		}
		/*-----------------------------------*/
		// button 4 press and hold
		if((btn4->action) && (btn4->state == BTN_PUSHED) && (btn4->delay2)){
			btn4->delay2 = FALSE;
			rtc_change_hours(UP);
		}
		// button 4 short press
		if((btn4->action) && (btn4->state == BTN_RELEASED) && (!btn4->delay1)){
			btn4->action = FALSE;
			rtc_change_hours(UP);
		}
		
		/*
		* Loop timing syncronization
		* ISRs are only enabled when the CPU is waiting for the next loop execution
		*/
		sei();
		while(!(*loop));
		*loop = FALSE;
		cli();
	}
}