// FUSE setting for ATMEGA88P : 0xF9,0xDD,0xDF

/* Name: usbconfig.h
 * Project: AVR USB driver
 * Author: Christian Starkjohann
 * Creation Date: 2005-04-01
 * Tabsize: 4
 * Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: Proprietary, free under certain conditions. See Documentation.
 * This Revision: $Id: usbconfig.h,v 1.4 2008-11-04 22:56:08 raph Exp $
 */

#ifndef __usbconfig_h_included__
#define __usbconfig_h_included__

/*
General Description:
This file contains parts of the USB driver which can be configured and can or
must be adapted to your hardware.
*/

/* ---------------------------- Hardware Config ---------------------------- */

#define USB_CFG_IOPORT          PORTD
/* This is the port where the USB bus is connected. When you configure it to
 * "PORTB", the registers PORTB, PINB (=PORTB+2) and DDRB (=PORTB+1) will be
 * used.
 */
#define USB_CFG_DMINUS_BIT      0
/* This is the bit number in USB_CFG_IOPORT where the USB D- line is connected.
 * This MUST be bit 0. All other values will result in a compile error!
 */
#define USB_CFG_DPLUS_BIT       2
/* This is the bit number in USB_CFG_IOPORT where the USB D+ line is connected.
 * This may be any bit in the port. Please note that D+ must also be connected
 * to interrupt pin INT0!
 */

/* --------------------------- Functional Range ---------------------------- */

#define USB_CFG_HAVE_INTRIN_ENDPOINT    1
/* Define this to 1 if you want to compile a version with two endpoints: The
 * default control endpoint 0 and an interrupt-in endpoint 1.
 */
#define USB_CFG_IMPLEMENT_HALT          0
/* Define this to 1 if you also want to implement the ENDPOINT_HALT feature
 * for endpoint 1 (interrupt endpoint). Although you may not need this feature,
 * it is required by the standard. We have made it a config option because it
 * bloats the code considerably.
 */
#define USB_CFG_INTR_POLL_INTERVAL      10
/* If you compile a version with endpoint 1 (interrupt-in), this is the poll
 * interval. The value is in milliseconds and must not be less than 10 ms for
 * low speed devices.
 */
#define USB_CFG_IS_SELF_POWERED         0
/* Define this to 1 if the device has its own power supply. Set it to 0 if the
 * device is powered from the USB bus.
 */
#define USB_CFG_MAX_BUS_POWER           100
/* Set this variable to the maximum USB bus power consumption of your device.
 * The value is in milliamperes. [It will be divided by two since USB
 * communicates power requirements in units of 2 mA.]
 */
#define USB_CFG_SAMPLE_EXACT            1
/* This variable affects Sampling Jitter for USB receiving. When it is 0, the
 * driver guarantees a sampling window of 1/2 bit. The USB spec requires
 * that the receiver has at most 1/4 bit sampling window. The 1/2 bit window
 * should still work reliably enough because we work at low speed. If you want
 * to meet the spec, set this value to 1. This will unroll a loop which
 * results in bigger code size.
 * If you have problems with long cables, try setting this value to 1.
 */
#define USB_CFG_IMPLEMENT_FN_WRITE      0
/* Set this to 1 if you want usbFunctionWrite() to be called for control-out
 * transfers. Set it to 0 if you don't need it and want to save a couple of
 * bytes.
 */
#define USB_CFG_IMPLEMENT_FN_READ       0
/* Set this to 1 if you need to send control replies which are generated
 * "on the fly" when usbFunctionRead() is called. If you only want to send
 * data from a static buffer, set it to 0 and return the data from
 * usbFunctionSetup(). This saves a couple of bytes.
 */

/* -------------------------- Device Description --------------------------- */

/* We cannot use Obdev's free shared VID/PID pair because this is a HID.
 * We use John Hyde's VID (author of the book "USB Design By Example") for
 * this example instead. John has offered this VID for use by students for
 * non-commercial devices. Well... This example is for demonstration and
 * education only... DO NOT LET DEVICES WITH THIS VID ESCAPE YOUR LAB!
 * The Product-ID is a random number.
 */
#define  USB_CFG_VENDOR_ID       0x81, 0x17
/* USB vendor ID for the device, low byte first. If you have registered your
 * own Vendor ID, define it here. Otherwise you use obdev's free shared
 * VID/PID pair. Be sure to read USBID-License.txt for rules!
 */
#define  USB_CFG_DEVICE_ID       0x9a, 0x0a
/* This is the ID of the product, low byte first. It is interpreted in the
 * scope of the vendor ID. If you have registered your own VID with usb.org
 * or if you have licensed a PID from somebody else, define it here. Otherwise
 * you use obdev's free shared VID/PID pair. Be sure to read the rules in
 * USBID-License.txt!
 */
#define USB_CFG_DEVICE_VERSION  0x02, 0x01
/* Version number of the device: Minor number first, then major number.
 */
#define USB_CFG_VENDOR_NAME     'r', 'a', 'p', 'h', 'n', 'e', 't', '.', 'n', 'e', 't'
#define USB_CFG_VENDOR_NAME_LEN 11
/* These two values define the vendor name returned by the USB device. The name
 * must be given as a list of characters under single quotes. The characters
 * are interpreted as Unicode (UTF-16) entities.
 * If you don't want a vendor name string, undefine these macros.
 * ALWAYS define a vendor name containing your Internet domain name if you use
 * obdev's free shared VID/PID pair. See the file USBID-License.txt for
 * details. 
 */
//#define USB_CFG_DEVICE_NAME     'G','C','/','N','6','4','_','U','S','B'
//#define USB_CFG_DEVICE_NAME_LEN 10
#define USB_CFG_DEVICE_NAME     'M','I','N','_','6','4','_','U','S','B'
#define USB_CFG_DEVICE_NAME_LEN 10
/* Same as above for the device name. If you don't want a device name, undefine
 * the macros. See the file USBID-License.txt before you assign a name.
 */
#define USB_CFG_SERIAL_NUMBER_LENGTH  4
/* Set this define to the number of charcters in the serial number if your
 * device should have a serial number to uniquely identify each hardware
 * instance. You must supply the serial number in a string descriptor with the
 * name "usbCfgSerialNumberStringDescriptor", e.g.:
 * #define USB_CFG_SERIAL_NUMBER_LENGTH  5
 * int usbCfgSerialNumberStringDescriptor[] PROGMEM = {
 *     USB_STRING_DESCRIPTOR_HEADER(USB_CFG_SERIAL_NUMBER_LENGTH),
 *     '1', '2', '3', '4', '5'
 * };
 * See usbdrv.h for more information about the USB_STRING_DESCRIPTOR_HEADER()
 * macro or usbdrv.c for example string descriptors.
 * You may want to put "usbCfgSerialNumberStringDescriptor" at a constant
 * flash memory address (with magic linker commands) so that you don't need
 * to recompile if you change it. 
 */
#define USB_CFG_DEVICE_CLASS    0   /* specify the class at the interface level */
#define USB_CFG_DEVICE_SUBCLASS 0
/* See USB specification if you want to conform to an existing device class.
 */
#define USB_CFG_INTERFACE_CLASS     0x03    /* HID class */
#define USB_CFG_INTERFACE_SUBCLASS  0       /* no boot interface */
#define USB_CFG_INTERFACE_PROTOCOL  0       /* no protocol */
/* See USB specification if you want to conform to an existing device class or
 * protocol.
 */
//#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH    42
/* Define this to the length of the HID report descriptor, if you implement
 * an HID device. Otherwise don't define it or define it to 0.
 */

/* Define this if you want to be able to set the report descriptor
 * dynamically at runtime. 
 * */
#define USB_CFG_HID_REPORT_DESCRIPTOR_RUNTIME

/* Define this if you want to be able to set the device descriptor
 * dynamically at runtime.
 */
#define USB_CFG_DEVICE_DESCRIPTOR_RUNTIME


#endif /* __usbconfig_h_included__ */
