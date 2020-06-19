
/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "i2c.h"
#include "config.h"

#include <avr/io.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

#define F_SCL 	100000UL

/* I2C Control Codes --------------------------------------------------------*/
#define TW_START (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)	// TWCR = 0b10100100: send start condition (TWINT,TWSTA,TWEN)
#define TW_STOP  (1<<TWINT)|(1<<TWSTO)|(1<<TWEN)	// TWCR = 0b10010100: send stop condition (TWINT,TWSTO,TWEN)
#define TW_ACK   (1<<TWINT)|(1<<TWEA)|(1<<TWEN)		// TWCR = 0b11000100: return ACK to slave
#define TW_NACK  (1<<TWINT)|(1<<TWEN)				// TWCR = 0b10000100: don't return ACK to slave
#define TW_SEND  (1<<TWINT)|(1<<TWEN)				// TWCR = 0b10000100: send data (TWINT,TWEN)

#define TW_READY (TWCR & (1<<TWINT))				// ready when TWINT returns to logic 1.
#define TW_STATUS (TWSR & 0xF8)						// returns value of status register

/* I2C Status Codes ---------------------------------------------------------*/
#define TWSR_MT_START				0x08
#define TWSR_MT_REPEATED_START		0x10
#define TWSR_MT_SLA_W_ACK			0x18
#define TWSR_MT_SLA_W_NACK			0x20
#define TWSR_MT_DATA_ACK			0x28
#define TWSR_MT_DATA_NACK			0x30
#define TWSR_MT_ARB_LOST			0x38
#define TWSR_MR_SLA_R_ACK			0x40
#define TWSR_MR_SLA_R_NACK			0x48
#define TWSR_MR_DATA_ACK			0x50
#define TWSR_MR_DATA_NACK			0x58

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

/*===========================================================================*/
/*
* at 16 MHz, the SCL frequency will be 16/(16+2(TWBR)), assuming prescalar of 0.
* so for 100KHz SCL, TWBR = ((F_CPU/F_SCL)-16)/2 = ((16/0.1)-16)/2 = 144/2 = 72.
*/
void i2c_init(void)
{
   TWSR = 0; // set prescalar to zero
   TWBR = ((F_CPU/F_SCL)-16)/2; // set SCL frequency in TWI bit register
}

/*===========================================================================*/
void i2c_stop(void)
{
	TWCR = TW_STOP;
}

/*===========================================================================*/
int8_t i2c_master_start(uint8_t addr_rw)
{
	TWCR = TW_START;	// send start condition
	while (!TW_READY);	// wait for start condition to happen
	// If status code is not as expected (start successfully sent)
	if (((TWSR & 0xF8) != TWSR_MT_START) && ((TWSR & 0xF8) != TWSR_MT_REPEATED_START)) {
		i2c_stop();
		return -1;
	}

	TWDR = addr_rw;		// load device's bus address + read/write instruction
	TWCR = TW_SEND;		// and send it
	while (!TW_READY);	// wait for acknowledge
	// If status code is not as expected (address not acknowledged)
	if (((TWSR & 0xF8) != TWSR_MT_SLA_W_ACK) && ((TWSR & 0xF8) != TWSR_MR_SLA_R_ACK)) {
		i2c_stop();
		return -1;
	}

	return 0;
}

/*===========================================================================*/
/*
* once the I2C communication has started (slave address acknowledged), if
* there's a bus transmission error, no re-try of individual operations can
* be performed. Instead, all the communication protocol must re-start again.
*/	
int8_t i2c_master_write(uint8_t data)
{
	TWDR = data; 			// load data to be sent
	TWCR = TW_SEND; 		// and send it
	while (!TW_READY); 		// wait to be sent
	if ((TWSR & 0xF8) != TWSR_MT_DATA_ACK) {
		i2c_stop();
		return -1;
	}
	
	return 0;
}

/*===========================================================================*/
uint8_t i2c_master_read(uint8_t last)
{
	uint16_t cnt = 0;

	if (last == LAST_BYTE)
		TWCR = TW_NACK;			// read with not-acknowledge
	else
		TWCR = TW_ACK;			// read with acknowledge
	
	while (!TW_READY) {			// wait to read
		cnt++;
		if(cnt == 2000){		// if 2ms have elapsed
			i2c_stop();
			return -1;
		}
	}

	if (((last == LAST_BYTE) && ((TWSR & 0xF8) != TWSR_MR_DATA_NACK)) ||
		((last == NOT_LAST_BYTE) && ((TWSR & 0xF8) != TWSR_MR_DATA_ACK)))
		return -1;

	return TWDR;
}