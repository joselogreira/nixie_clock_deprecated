
#ifndef RTC_H
#define RTC_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include <stdint.h>

/******************************************************************************
***************** S T R U C T U R E   D E C L A R A T I O N S *****************
******************************************************************************/

typedef struct {
	uint8_t	sec;			// seconds
	uint8_t min;			// minutes
	uint8_t hour;			// hours
	uint8_t s_units;		// BCD seconds' units
	uint8_t s_tens;			// BCD seconds' tens
	uint8_t m_units;		// BCD minutes' units
	uint8_t m_tens;			// BCD minutes' tens
	uint8_t h_units;		// BCD hours' units
	uint8_t h_tens;			// BCD hours' tens
	uint8_t update;			// flag. 1Hz update?
	uint8_t hour_mode;		// 12/24h 
	uint8_t day_period;		// AM/PM
} time_s;

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void rtc_init(void);
void rtc_read_time(void);
void rtc_change_minutes(uint8_t up);
void rtc_change_hours(uint8_t up);
void rtc_change_hour_mode(void);
volatile time_s * rtc_get_time_handler(void);

#endif	/* INIT_H */