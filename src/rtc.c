
/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "rtc.h"
#include "i2c.h"
#include "config.h"
#include "timers.h"

#include <avr/io.h>

/******************************************************************************
*************** G L O B A L   V A R S   D E F I N I T I O N S *****************
******************************************************************************/

volatile time_s 	time;

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

// TIME handler
#define MODE_12H		0x12
#define MODE_24H		0x24
#define PERIOD_AM		0xAA
#define PERIOD_PM		0xFF

// RTC
#define RTC_SLAVE_ID_W		0b11010000			// DS1307 address + w
#define RTC_SLAVE_ID_R		0b11010001			// DS1307 address + r

#define RTC_SECONDS_REG 	0x00
#define RTC_MINUTES_REG 	0x01
#define RTC_HOURS_REG 		0x02
#define RTC_DAYOFWK_REG 	0x03
#define RTC_DAYS_REG 		0x04
#define RTC_MONTHS_REG 		0x05
#define RTC_YEARS_REG 		0x06
#define RTC_CONTROL_REG 	0x07
#define RTC_RAM_BEGIN 		0x08
#define RTC_RAM_END 		0x3F

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static void rtc_halt(uint8_t flag);

/*===========================================================================*/
void rtc_init(void)
{	
	// RTC_CONTROL_REG:
	// 	- Clear all
	i2c_master_start(RTC_SLAVE_ID_W);
	i2c_master_write(RTC_CONTROL_REG);
	i2c_master_write(_BV(4));
		
	// RTC_SECONDS_REG:
	//	- Clear CH bit -> starts clock
	i2c_master_start(RTC_SLAVE_ID_W);
	i2c_master_write(RTC_SECONDS_REG);
	i2c_master_start(RTC_SLAVE_ID_R);
	uint8_t reg = i2c_master_read(LAST_BYTE);
	reg &= ~_BV(7);
	i2c_master_start(RTC_SLAVE_ID_W);
	i2c_master_write(RTC_SECONDS_REG);
	i2c_master_write(reg);
	
	i2c_stop();

	// TIME handler init
	time.sec = 0;
	time.min = 0;
	time.hour = 12;
	time.s_units = 0;
	time.s_tens = 0;
	time.m_units = 0;
	time.m_tens = 0;
	time.h_units = 0;
	time.h_tens = 0;
	time.update = FALSE;
	time.hour_mode = MODE_12H;
	time.day_period = PERIOD_AM;
}

/*===========================================================================*/
void rtc_read_time(void)
{
	i2c_master_start(RTC_SLAVE_ID_W);
	i2c_master_write(RTC_SECONDS_REG);
	i2c_master_start(RTC_SLAVE_ID_R);
	uint8_t s_reg = i2c_master_read(NOT_LAST_BYTE);
	uint8_t m_reg = i2c_master_read(NOT_LAST_BYTE);
	uint8_t h_reg = i2c_master_read(LAST_BYTE);
	i2c_stop();

	// seconds register
	s_reg &= ~_BV(7);
	time.s_tens 	= s_reg >> 4;
	time.s_units 	= s_reg & 0x0F;
	time.sec 		= ((time.s_tens) * 10) + (time.s_units);
	// minutes register
	time.m_tens 	= m_reg >> 4;
	time.m_units 	= m_reg & 0x0F;
	time.min 		= ((time.m_tens) * 10) + (time.m_units);
	// hours register
	if (h_reg & _BV(6)) {
		// 12h mode
		time.hour_mode = MODE_12H;
		if (h_reg & _BV(5)) time.day_period = PERIOD_PM;
		else time. day_period = PERIOD_AM;
		time.h_tens 	= (h_reg >> 4) & 0x01;
		time.h_units 	= h_reg & 0x0F;
		time.hour 		= ((time.h_tens) * 10) + (time.h_units);
	} else {
		time.hour_mode = MODE_24H;
		// 24h mode
		time.h_tens 	= (h_reg >> 4) & 0x03;
		time.h_units 	= h_reg & 0x0F;
		time.hour 		= ((time.h_tens) * 10) + (time.h_units);
		if (time.hour >= 12) time.day_period = PERIOD_PM;
		else time.day_period = PERIOD_AM;
	}
}

/*===========================================================================*/
void rtc_change_minutes(uint8_t up)
{
	uint8_t m_reg;

	rtc_halt(TRUE);

	if (up) {
		if (time.min == 59) time.min = 0;
		else (time.min)++;
	} else {
		if (time.min == 00) time.min = 59;
		else (time.min)--;
	}
	time.m_tens 	= time.min / 10;
	time.m_units 	= time.min % 10;
	m_reg = (time.m_tens << 4) + (time.m_units);

	i2c_master_start(RTC_SLAVE_ID_W);
	i2c_master_write(RTC_MINUTES_REG);	
	i2c_master_write(m_reg);
	rtc_halt(FALSE);
	i2c_stop();
}

/*===========================================================================*/
void rtc_change_hours(uint8_t up)
{
	uint8_t h_reg = 0;

	rtc_halt(TRUE);

	if (time.hour_mode == MODE_24H) {
		if (up) {
			if (time.hour == 23) time.hour = 0;
			else (time.hour)++;
		} else {
			if (time.hour == 00) time.hour = 23;
			else (time.hour)--;
		}
		if (time.hour >= 12) time.day_period = PERIOD_PM;
		else time.day_period = PERIOD_AM;
	} else if (time.hour_mode == MODE_12H) {
		if (up) {
			if (time.hour == 12) {
				time.hour = 1;
			} else if (time.hour == 11) {
				(time.hour)++;
				if (time.day_period == PERIOD_AM) 
					time.day_period = PERIOD_PM;
				else
					time.day_period = PERIOD_AM;
			} else {
				(time.hour)++;
			}
		} else {
			if (time.hour == 1) {
				time.hour = 12;
			} else if (time.hour == 12) {
				(time.hour)--;
				if (time.day_period == PERIOD_AM) 
					time.day_period = PERIOD_PM;
				else
					time.day_period = PERIOD_AM;
			} else {
				(time.hour)--;
			}
		}
	}

	time.h_tens 	= time.hour / 10;
	time.h_units 	= time.hour % 10;

	if (time.hour_mode == MODE_24H) {
		h_reg = (time.h_tens << 4) + (time.h_units);
		h_reg &= ~_BV(6);
	} else if (time.hour_mode == MODE_12H) {
		h_reg = (time.h_tens << 4) + (time.h_units);
		h_reg |= _BV(6);
		if (time.day_period == PERIOD_PM)
			h_reg |= _BV(5);
		else if (time.day_period == PERIOD_AM)
			h_reg &= ~_BV(5);
	}

	i2c_master_start(RTC_SLAVE_ID_W);
	i2c_master_write(RTC_HOURS_REG);	
	i2c_master_write(h_reg);
	rtc_halt(FALSE);
	i2c_stop();
}

/*===========================================================================*/
void rtc_change_hour_mode(void)
{
	uint8_t h_reg = 0;

	rtc_halt(TRUE);
	rtc_read_time();

	if (time.hour_mode == MODE_24H) {
		time.hour_mode = MODE_12H;
		if (time.hour > 12) {
			time.hour -= 12;
			time.h_tens = time.hour / 10;
			time.h_units = time.hour % 10;
			h_reg = (time.h_tens << 4) + time.h_units;
			h_reg |= _BV(5);
		} else {
			h_reg = (time.h_tens << 4) + time.h_units;
		}
		h_reg |= _BV(6);	// change bit to 1: 12h
	} else if (time.hour_mode == MODE_12H) {
		time.hour_mode = MODE_24H;
		if ((time.day_period == PERIOD_PM) && (time.hour < 12))
			time.hour += 12;
		else if ((time.day_period == PERIOD_AM) && (time.hour == 12))
			time.hour = 0;
		time.h_tens = time.hour / 10;
		time.h_units = time.hour % 10;
		h_reg = (time.h_tens << 4) + time.h_units;
		h_reg &= ~_BV(6);	// change bit to 0: 24h
	}	

	i2c_master_start(RTC_SLAVE_ID_W);
	i2c_master_write(RTC_HOURS_REG);
	i2c_master_write(h_reg);
	
	rtc_halt(FALSE);
	i2c_stop();
}

/*===========================================================================*/
volatile time_s * rtc_get_time_handler(void)
{
	return &time;
}

/*-----------------------------------------------------------------------------
-------------------------- L O C A L   F U N C T I O N S ----------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
static void rtc_halt(uint8_t flag)
{
	i2c_master_start(RTC_SLAVE_ID_W);
	i2c_master_write(RTC_SECONDS_REG);
	i2c_master_start(RTC_SLAVE_ID_R);
	uint8_t s_reg = i2c_master_read(LAST_BYTE);

	if (flag) s_reg |= _BV(7);
	else s_reg &= ~_BV(7);

	i2c_master_start(RTC_SLAVE_ID_W);
	i2c_master_write(RTC_SECONDS_REG);
	i2c_master_start(s_reg);
}