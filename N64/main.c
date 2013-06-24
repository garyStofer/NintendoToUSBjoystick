/* Name: main.c
 * Project: Gamecube/N64 to USB converter
 * Author: Raphael Assenat <raph@raphnet.net
 * Copyright: (C) 2007 Raphael Assenat <raph@raphnet.net>
 * License: Proprietary, free under certain conditions. See Documentation.
 * Comments: Based on HID-Test by Christian Starkjohann
 * Tabsize: 4
 */
// FUSE for ATMEGA88P 0xF9,0xDD,0xDF

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <string.h>

#include "usbdrv.h"
#include "oddebug.h"
#include "gamepad.h"
#include "gamecube.h"
#include "n64.h"

#include "devdesc.h"
#include "reportdesc.h"

int const usbCfgSerialNumberStringDescriptor[] PROGMEM = {
 	USB_STRING_DESCRIPTOR_HEADER(USB_CFG_SERIAL_NUMBER_LENGTH),
 	'0', '0', '0', '3'
 };

static Gamepad *curGamepad;


/* ----------------------- hardware I/O abstraction ------------------------ */

static void hardwareInit(void)
{
	// init port C as input with pullup
	DDRC = 0x00;
	PORTC = 0xff;
	
	/* 1101 1000 bin: activate pull-ups except on USB lines 
	 *
	 * USB signals are on bit 0 and 2. 
	 *
	 * Bit 1 is connected with bit 0 (rev.C pcb error), so the pullup
	 * is not enabled.
	 * */
	PORTD = 0xf8;   

	/* Usb pin are init as outputs */
	DDRD = 0x01 | 0x04;    
	_delay_ms(11); 
	DDRD = 0x00;    /* 0000 0000 bin: remove USB reset condition */
			/* configure timer 0 for a rate of 12M/(1024 * 256) = 45.78 Hz (~22ms) */
	TCCR0B = 5;      /* timer 0 prescaler: 1024 */

	//TCCR2 = (1<<WGM21)|(1<<CS22)|(1<<CS21)|(1<<CS20); on ATMEGA8
	TCCR2B = (1<<CS22)|(1<<CS21)|(1<<CS20);
	TCCR2A = (1<<WGM21);
	OCR2A = 196; // for 60 hz

}

static uchar    reportBuffer[8];    /* buffer for HID reports */



/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

static uchar    idleRate;           /* in 4 ms units */


uchar	usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (void *)data;

	usbMsgPtr = reportBuffer;
	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
		if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
			/* we only have one report type, so don't look at wValue */
			curGamepad->buildReport(reportBuffer);
			return curGamepad->report_size;
		}else if(rq->bRequest == USBRQ_HID_GET_IDLE){
			usbMsgPtr = &idleRate;
			return 1;
		}else if(rq->bRequest == USBRQ_HID_SET_IDLE){
			idleRate = rq->wValue.bytes[1];
		}
	}else{
	/* no vendor specific requests implemented */
	}
	return 0;
}

/* ------------------------------------------------------------------------- */


int main(void)
{
	char must_report = 0, first_run = 1;
	uchar   idleCounter = 0;

	hardwareInit();
	odDebugInit();

	while(1)
	{
		/* Check for gamecube controller */
	//	curGamepad = gamecubeGetGamepad();
	//	curGamepad->init();
	//	if (curGamepad->probe()) 
	//		break;

	//	_delay_ms(30);

		/* Check for n64 controller */
		curGamepad = n64GetGamepad();
		curGamepad->init();
		if (curGamepad->probe()) 
			break;
		
		_delay_ms(30);
	}


	rt_usbHidReportDescriptor = curGamepad->reportDescriptor;//gcn64_usbHidReportDescriptor;
	rt_usbHidReportDescriptorSize = curGamepad->reportDescriptorSize; //getUsbHidReportDescriptor_size();

	rt_usbDeviceDescriptor = (void*)usbDescrDevice;
	rt_usbDeviceDescriptorSize = getUsbDescrDevice_size();

	// Do hardwareInit again. It causes a USB reset. 
	hardwareInit();
	curGamepad->init();

	wdt_enable(WDTO_2S);
	usbInit();
	sei();
	DBG1(0x00, 0, 0);
	
	
	while (1) 
	{	/* main event loop */
		wdt_reset();

		// this must be called at each 50 ms or less
		usbPoll();

		if (first_run) {
			curGamepad->update();
			first_run = 0;
		}
// Changed from atmega88
		if(TIFR0 & (1<<TOV0)){   /* 22 ms timer */
			TIFR0 = 1<<TOV0;
			if(idleRate != 0){
				if(idleCounter > 4){
					idleCounter -= 5;   /* 22 ms in units of 4 ms */
				}else{
					idleCounter = idleRate;
					must_report = 1;
				}
			}
		}
// Changed from atmega88
		if (TIFR2 & (1<<OCF2A))
		{
			TIFR2 = 1<<OCF2A;
			if (!must_report)
			{
				curGamepad->update();
				if (curGamepad->changed()) {
					must_report = 1;
				}
			}

		}
		
			
		if(must_report)
		{
			if (usbInterruptIsReady())
			{ 	
				must_report = 0;

				curGamepad->buildReport(reportBuffer);
				usbSetInterrupt(reportBuffer, curGamepad->report_size);
			}
		}
	}
	return 0;
}

/* ------------------------------------------------------------------------- */
