#ifndef _reportdesc_h__
#define _reportdesc_h__

#include <avr/pgmspace.h>

#define N64_REPORT_SIZE	4
#define GC_REPORT_SIZE	8

extern const char gcn64_usbHidReportDescriptor[] PROGMEM;
int getUsbHidReportDescriptor_size(void);

#endif // _reportdesc_h__

