/* Name: main.c
 * Project: Gamecube/N64 to USB converter
 * Author: Raphael Assenat <raph@raphnet.net
 * Copyright: (C) 2007 Raphael Assenat <raph@raphnet.net>
 * License: Proprietary, free under certain conditions. See Documentation.
 * Tabsize: 4
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "gamepad.h"

#include "n64.h"
#include "reportdesc.h"


/******** IO port definitions **************/
#define N64_DATA_PORT	PORTC
#define N64_DATA_DDR	DDRC
#define N64_DATA_PIN	PINC
#define N64_DATA_BIT	(1<<5)

/*********** prototypes *************/
static void n64Init(void);
static void n64Update(void);
static char n64Changed(void);
static void n64BuildReport(unsigned char *reportBuffer);


/* What was most recently read from the controller */
static unsigned char last_built_report[N64_REPORT_SIZE];

/* What was most recently sent to the host */
static unsigned char last_sent_report[N64_REPORT_SIZE];

/* For probe */
static char last_failed;

static void n64Init(void)
{
	unsigned char sreg;
	sreg = SREG;
	cli();

	DDRB |= 0x02; // Bit 1 out
	PORTB &= ~0x02; // 0
	
	// data as input
	N64_DATA_DDR &= ~(N64_DATA_BIT);

	// keep data low. By toggling the direction, we make the
	// pin act as an open-drain output.
	N64_DATA_PORT &= ~N64_DATA_BIT;

	n64Update();

	SREG = sreg;
}



void _n64Update(unsigned char tmp)
{
	int i;
	unsigned char tmpdata[4];	
	unsigned char volatile results[65];
	unsigned char count;
	
	/*
	 * z: Points to current source byte
	 * r16: Holds the bit to be AND'ed with current byte
	 * r17: Holds the current byte
	 * r18: Holds the number of bits left to send.
	 * r19: Loop counter for delays
	 *
	 * Edit sbi/cbi/andi instructions to use the right bit!
	 * 
	 * 3 us == 36 cycles
	 * 1 us == 12 cycles
	 */
	asm volatile(
"			push r30				\n"
"			push r31				\n"
			
"			ldi r16, 0x80			\n" // 1
"nextBit:							\n" 
"			mov r17, %2				\n" // 2
//"			ldi r17, 0x0f			\n" // 2
"			and r17, r16			\n" // 1
"			breq send0				\n" // 2
"			nop						\n"

"send1:								\n"
"			sbi %1, 5				\n" // 2
"				nop\nnop\nnop\nnop	\n" // 4
"				nop\nnop\nnop\nnop	\n" // 4
"				nop\nnop\n			\n" // 3
"			cbi %1, 5				\n" // 2
"				ldi r19, 8			\n"	// 1 
"lp1:			dec r19				\n"	// 1 
"				brne lp1			\n"	// 2
"				nop\nnop			\n" // 2
"			lsr r16					\n" // 1
"			breq done				\n" // 1
"			rjmp nextBit			\n" // 2
		
/* Send a 0: 3us Low, 1us High */
"send0:		sbi %1, 5				\n"	// 2
"				ldi r19, 11			\n"	// 1
"lp0:			dec r19				\n"	// 1
"				brne lp0			\n"	// 2
"				nop					\n" // 1

"          	cbi %1,5				\n" // 2
"				nop\nnop\n				\n" // 2

"			lsr r16					\n" // 1
"			breq done				\n" // 1
"			rjmp nextBit			\n" // 2

"done:								\n"
"			cbi %1, 5				\n"
"			nop\nnop\nnop\nnop		\n"	// 4
"			nop\nnop\nnop\nnop		\n"	// 4
"			nop						\n"	// 1


// Stop bit (1us low, 3us high)
"          	sbi %1,5				\n" // 2
"			nop\nnop\nnop\nnop		\n" // 4
"			cbi %1,5				\n" 
			

			// Response reading part //
			// r16: loop Counter
			// r17: Reference state
			// r18: Current state
			// r19: bit counter
		
"			ldi r19, 32\n			" // 1  We will receive 32 bits

"st:\n								"
"			ldi r16, 0xff			\n" // setup a timeout
"waitFall:							\n" 
"			dec r16					\n" // 1
"			breq timeout			\n" // 1 
"			in r17, %4				\n" // 1  read the input port
"			andi r17, 0x20			\n" // 1  isolate the input bit
"			brne waitFall			\n" // 2  if still high, loop

// OK, so there is now a 0 on the wire.
// Worst case, we are at the 9th cycle.
// Best case, we are at the 4th cycle.
// 	Middle: cycle 6
// 
// cycle: 1-12 12-24 24-36 36-48
//  high:  0     1     1     1
//	 low:  0     0     0     1
//
// I check the pin on the 24th cycle which is the safest place.

"			cbi %5, 1\n"				// DEBUG
"			nop\nnop\n	" // 2
"			nop\nnop\nnop\nnop\n	" // 4
"			nop\nnop\nnop\nnop\n	" // 4
"			nop\nnop\nnop\nnop\n	" // 4
"			nop\n"
			
			// We are now more or less aligned on the 24th cycle.			
"			in r18, %4\n			" // 1  Read from the port
"			sbi %5, 1\n"				// DEBUG
"			andi r18, 0x20\n		" // 1  Isolate our bit
"			st z+, r18\n			" // 2  store the value

"			dec r19\n				" // 1 decrement the bit counter
"			breq ok\n				" // 1

"			ldi r16, 0xff\n			" // setup a timeout
"waitHigh:\n						"
"			dec r16\n				" // decrement timeout
"			breq timeout\n			" // handle timeout condition
"			in r18, %4\n			" // Read the port
"			andi r18, 0x20\n		" // Isolate our bit
"			brne st\n				" // Continue if set
"			rjmp waitHigh\n			" // loop otherwise..

"ok:\n"
"			clr %0\n				"
"			rjmp end\n				"

"error:\n"
"			sbi %5, 1\n"
"			mov %0, r19				\n" // report how many bits were read
"			rjmp end				\n"

"timeout:							\n"
"			clr %0					\n"
"			com %0					\n" // 255 is timeout

"end:\n"
"			pop r31\n"
"			pop r30\n"
"			cbi %5, 1\n"
			: "=r" (count)
			: "I" (_SFR_IO_ADDR(N64_DATA_DDR)), "r"(tmp), 
				"z"(results), "I" (_SFR_IO_ADDR(N64_DATA_PIN)),
				"I" (_SFR_IO_ADDR(PORTB))
			: "r16","r17","r18","r19"
			);

	if (count == 255) {
		last_failed = 1;
		return; // failure
	}
	
	
/*
	Bit	Function
	0	A
	1	B
	2	Z
	3	Start
	4	Directional Up
	5	Directional Down
	6	Directional Left
	7	Directional Right
	8	unknown (always 0)
	9	unknown (always 0)
	10	L
	11	R
	12	C Up
	13	C Down
	14	C Left
	15	C Right
	16-23: analog X axis
	24-31: analog Y axis
 */

	tmpdata[0]=0;
	tmpdata[1]=0;
	tmpdata[2]=0;
	tmpdata[3]=0;

	for (i=0; i<4; i++) // A B Z START
		tmpdata[2] |= results[i] ? (0x01<<i) : 0;

	for (i=0; i<4; i++) // C-UP C-DOWN C-LEFT C-RIGHT
		tmpdata[2] |= results[i+12] ? (0x10<<i) : 0;
	
	for (i=0; i<2; i++) // L R
		tmpdata[3] |= results[i+10] ? (0x01<<i) : 0;

	for (i=0; i<8; i++) // X axis
		tmpdata[0] |= results[i+16] ? (0x80>>i) : 0;
	
	for (i=0; i<8; i++) // Y axis
		tmpdata[1] |= results[i+24] ? (0x80>>i) : 0;	

	// analog joystick
	last_built_report[0] = ((int)((signed char)tmpdata[0]))+127;
	last_built_report[1] = ((int)( -((signed char)tmpdata[1])) )+127;



	// buttons
	last_built_report[2] = tmpdata[2];
	last_built_report[3] = tmpdata[3];

	// dpad as buttons
	if (results[4]) 
		last_built_report[3] |= 0x04;
	if (results[5])
		last_built_report[3] |= 0x08;
	if (results[6])
		last_built_report[3] |= 0x10;
	if (results[7])
		last_built_report[3] |= 0x20;
}

static void n64Update(void)
{
	_n64Update(0x01); // get status
}

static char n64Probe(void)
{
	char i;

	for (i=0; i<15; i++)
	{
		_delay_ms(30);
		
		last_failed = 0;
		n64Update();

		if (!last_failed)
			return 1;
	}
	return 0;
}


static char n64Changed(void)
{
	static int first = 1;
	if (first) { first = 0;  return 1; }
	
	return memcmp(last_built_report, last_sent_report, N64_REPORT_SIZE);
}

static void n64BuildReport(unsigned char *reportBuffer)
{
	if (reportBuffer) 
		memcpy(reportBuffer, last_built_report, N64_REPORT_SIZE);
	
	memcpy(	last_sent_report, last_built_report, N64_REPORT_SIZE);	
}

Gamepad N64Gamepad = {
	.report_size			= N64_REPORT_SIZE,
	.init					= n64Init,
	.update					= n64Update,
	.changed				= n64Changed,
	.buildReport			= n64BuildReport,
	.probe					= n64Probe,
};

Gamepad *n64GetGamepad(void)
{
	N64Gamepad.reportDescriptor = (void*)gcn64_usbHidReportDescriptor;
	N64Gamepad.reportDescriptorSize = getUsbHidReportDescriptor_size();
	return &N64Gamepad;
}

