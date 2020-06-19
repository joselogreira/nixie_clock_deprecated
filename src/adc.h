
#ifndef ADC_H
#define ADC_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include <stdint.h>

/******************************************************************************
***************** S T R U C T U R E   D E C L A R A T I O N S *****************
******************************************************************************/

typedef struct {
	uint8_t n;
	uint8_t query;	// flag; query button state
	uint8_t action;			// flag; button action activated
	uint8_t lock;			// flag; button locked
	uint8_t state;			// button state: IDLE, PUSHED, RELEASED
	uint16_t count;			// time counter
	uint8_t delay1;			// flag; delay 1 elapsed
	uint8_t delay2;			// flag; delay 2 elapsed
	uint8_t delay3;			// flag; delay 3 elapsed
} btn_s;

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void adc_init(uint8_t run);

uint8_t adc_key_press(void);
volatile btn_s * adc_get_button_handler(uint8_t btn);

#endif 	/* ADC_H */