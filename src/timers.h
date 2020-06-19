
#ifndef TIMERS_H
#define TIMERS_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include <stdint.h>

/******************************************************************************
***************** S T R U C T U R E   D E C L A R A T I O N S *****************
******************************************************************************/

typedef struct {
    uint8_t mode;
    uint8_t d1;
    uint8_t d2;
    uint8_t d3;
    uint8_t d4;
} display_s;

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void timers_init(void);
void timer_ms_set(uint8_t state);
void timer_sec_set(uint8_t state);
volatile display_s * timer_get_display_handler(void);
volatile uint8_t * timer_get_loop_flag(void);

#endif 	/* TIMERS_H */