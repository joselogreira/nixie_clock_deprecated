
#ifndef MAIN_H
#define MAIN_H

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

#define FIRMWARE_DATE 	"16-JUN-2020"

// 16MHz ceramic resonator
#define F_CPU			16000000UL

// GENERIC BOOLEAN MACROS
#define TRUE		0x01
#define FALSE 		0x00
#define ENABLE		TRUE
#define DISABLE		FALSE
#define ON 			TRUE
#define OFF			FALSE
#define HIGH		TRUE
#define LOW			FALSE
#define UP 			TRUE
#define DOWN 		FALSE
#define PASS 		0xAA
#define FAIL 		0xBB

// TUBES NAMES
#define TUBE_A		0
#define TUBE_B		1
#define TUBE_C		2
#define TUBE_D		3
// When tube is blank
#define BLANK 		0xFF

// NULL pointer
#define NULL 			((void *)0)

#endif	/* MAIN_H */