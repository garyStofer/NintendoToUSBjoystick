/* Name: reportdesc.c
 * Project: Gamecube/N64 to USB converter
 * Author: Raphael Assenat <raph@raphnet.net
 * Copyright: (C) 2007 Raphael Assenat <raph@raphnet.net>
 * License: Proprietary, free under certain conditions. See Documentation.
 * Tabsize: 4
 */

#include "reportdesc.h"

const char gcn64_usbHidReportDescriptor[] PROGMEM = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    // USAGE (Gamepad)
    0xa1, 0x01,                    // COLLECTION (Application)
	
    0x09, 0x01,                    //   USAGE (Pointer)    
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
	
//	0x09, 0x33,					   //     USAGE (Rx)
//	0x09, 0x34,						//	  USAGE (Ry)

//	0x09, 0x35,						//	  USAGE (Rz)	
//	0x09, 0x36,						//	  USAGE (Slider)	

    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,              //     LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0xc0,                          //   END_COLLECTION (Physical)

    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
    0x29, 0x10,                    //   USAGE_MAXIMUM (Button 14)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x10,                    //   REPORT_COUNT (16)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)

    0xc0                           // END_COLLECTION (Application)
};


int getUsbHidReportDescriptor_size(void)
{
	return sizeof(gcn64_usbHidReportDescriptor);
}

