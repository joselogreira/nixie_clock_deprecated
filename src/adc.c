
/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "adc.h"
#include "config.h"
#include "util.h"

#include <avr/io.h>

/******************************************************************************
*************** G L O B A L   V A R S   D E F I N I T I O N S *****************
******************************************************************************/

volatile btn_s 		btn1, btn2, btn3, btn4;

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

#define ADC_PRESCALER_DIV128 	((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0))
#define ADC_MUX_MASK			((1<< MUX3) | (1<<MUX2) | (1<<MUX1) | (1<<MUX0))

#define ADC_READ_N 		3

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static uint16_t adc_read(uint8_t run);

/*===========================================================================*/
void adc_init(uint8_t run)
{
	ADMUX &= ~(ADC_MUX_MASK);			/* Clear ADC mux selection */
	ADCSRA |= ADC_PRESCALER_DIV128;		/* Set prescaler */
	ADMUX |= (1<<REFS0);                // Voltage reference from Avcc (5v)
	ADMUX |= (1<<MUX0);  				// Sectlect channel 1 as ADC input
	ADCSRA |= (1<<ADEN);				/* Enable ADC conversions */

	if (run) // Free running conversion
		ADCSRA |= (1<<ADATE);	// Autotrigger enable
	
	ADCSRA |= (1<<ADSC);     	//Do an initial conversion because this one is the slowest and to ensure that everything is up and running

	// Button handlers init
	btn1.n = 1;
	btn1.query = FALSE;
	btn1.action = FALSE;
	btn1.lock = FALSE;
	btn1.state = BTN_IDLE;
	btn1.count = 0;
	btn1.delay1 = FALSE;
	btn1.delay2 = FALSE;
	btn1.delay3 = FALSE;

	btn2.n = 2;
	btn2.query = FALSE;
	btn2.action = FALSE;
	btn2.lock = FALSE;
	btn2.state = BTN_IDLE;
	btn2.count = 0;
	btn2.delay1 = FALSE;
	btn2.delay2 = FALSE;
	btn2.delay3 = FALSE;

	btn3.n = 3;
	btn3.query = FALSE;
	btn3.action = FALSE;
	btn3.lock = FALSE;
	btn3.state = BTN_IDLE;
	btn3.count = 0;
	btn3.delay1 = FALSE;
	btn3.delay2 = FALSE;
	btn3.delay3 = FALSE;

	btn4.n = 4;
	btn4.query = FALSE;
	btn4.action = FALSE;
	btn4.lock = FALSE;
	btn4.state = BTN_IDLE;
	btn4.count = 0;
	btn4.delay1 = FALSE;
	btn4.delay2 = FALSE;
	btn4.delay3 = FALSE;
}

/*===========================================================================*/
uint8_t adc_key_press(void)
{
	uint16_t data = adc_read(TRUE);
	int key = 5; 
	
	if (data < 0x3C1) 			//(data < 0x384) V<4.7V
	{
		if(data > 0x2F5)		//(data > 0x2b7) V>3.7              
			key = 4;
		else if(data > 0x199)	//(data > 0x199) V>2V          
			key = 3;
		else if(data > 0x08f)	//(data > 0x08f) V>0.7       
			key = 2;
		else if(data < 0x08f)	//(data < 0x08f) V<0.7       
			key = 1;
	}
	
	return key;  // button;                    //Returns the button pressed
}

/*===========================================================================*/
volatile btn_s * adc_get_button_handler(uint8_t btn)
{
	if (btn == 1) return &btn1;
	else if (btn == 2) return &btn2;
	else if (btn == 3) return &btn3;
	else if (btn == 4) return &btn4;
	else return NULL;
}

/*-----------------------------------------------------------------------------
-------------------------- L O C A L   F U N C T I O N S ----------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
static uint16_t adc_read(uint8_t run)
{	
	uint16_t avg = 0;
	
	if (run) {
		avg = ADC;
	} else {
		ADCSRA |= (1<<ADSC);			/* This starts the conversion. */
		// perform n reads
		for (uint8_t i = 0; i < ADC_READ_N; i++){
			while ((ADCSRA & (1<<ADSC)));	/* Wait 'til conversion completed */
			avg += ADC;
		}
		avg = avg / ((uint16_t)ADC_READ_N);	// average value	
	}
	
	return avg;
}