/* Name: main.c
 * Project: Testing driver features
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-29
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary
 * (CommercialLicense.txt)
 */

/*
This module is a do-nothing test code linking against (or including) the USB
driver. It is used to determine the code size for various options and to
check whether the code compiles with all options.
*/
// #include <avr/eeprom.h>
#include <avr/interrupt.h> /* for sei() */
#include <avr/io.h>
#include <avr/pgmspace.h> /* required by usbdrv.h */
// #include <avr/wdt.h>
#include <util/delay.h> /* for _delay_ms() */

#include "usbdrv.h"
#if USE_INCLUDE
#include "usbdrv.c"
#endif

#define HW_CDC_BULK_OUT_SIZE 8
#define HW_CDC_BULK_IN_SIZE 8

// /* defines CDC class requests: */
#define SEND_ENCAPSULATED_COMMAND 0
#define GET_ENCAPSULATED_RESPONSE 1
#define SET_COMM_FEATURE 2
#define GET_COMM_FEATURE 3
#define CLEAR_COMM_FEATURE 4
#define SET_LINE_CODING 0x20
#define GET_LINE_CODING 0x21
#define SET_CONTROL_LINE_STATE 0x22
#define SEND_BREAK 0x23

// /* defines for 'requestType' */
#define REQUEST_TYPE_LINE_CODING 0     // CDC GET/SET_LINE_CODING
#define REQUEST_TYPE_HID_FIRST 1       // first block of HID reporting
#define REQUEST_TYPE_HID_SUBSEQUENT 2  // subsequent block of HID reporting
#define REQUEST_TYPE_HID_DEBUGDATA 3  // read/write data from/to debug interface
#define REQUEST_TYPE_VENDOR 4         // vendor request for get/set debug data

// /* The following variables store the status of the current data transfer */
// static uchar currentAddress;
// static uchar bytesRemaining;
// static uchar requestType;

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

#if USB_CFG_IMPLEMENT_FN_WRITE
/* usbFunctionWrite() is called when the host sends a chunk of data to the
 * device. For more information see the documentation in usbdrv/usbdrv.h.
 */
uchar usbFunctionWrite(uchar *data, uchar len) {}
#endif

#if USB_CFG_IMPLEMENT_FN_READ
// usbFunctionRead() is called when the host requests a chunk of data from
// the device. For more information see the documentation in usbdrv/usbdrv.h.
uchar usbFunctionRead(uchar *data, uchar len) {}
#endif

#if USB_CFG_IMPLEMENT_FN_WRITEOUT
void usbFunctionWriteOut(uchar *data, uchar len) {}
#endif

static PROGMEM const char configurationDescriptor[] = {
    9,                // bLength: size of the descriptor in Bytes
    USBDESCR_CONFIG,  // bDescriptorType
    67,
    0,  // wTotalLength: Total length in bytes of data returned
    2,  // bNumInterfaces
    1,  // bConfigurationValue: argument value to select this configuration
    0,  // iConfiguration:
    0,  // bmAttributes:
    USB_CFG_MAX_BUS_POWER / 2,  // bMaxPower: max power consumption in 2mA

    /* Communications Class Interface Descriptor for CDC-ACM Control */
    9,                   // bLength: size of the descriptor in Bytes
    USBDESCR_INTERFACE,  // bDescriptorType

    0,  // bInterfaceNumber: index of this interface
    0,  // bAlternateSetting: alternate setting for this interface
    1,  // bNumEndpoints: number of endpoints used for this interface
    USB_CFG_INTERFACE_CLASS, USB_CFG_INTERFACE_SUBCLASS,
    USB_CFG_INTERFACE_PROTOCOL,
    0,  // iInterface: Index of String Descriptor Describing this interface

    /* CDC Class-Specific Descriptors */
    /* Header Functional Descriptor */
    5,           // bLength: size of the descriptor in Bytes
    0x24,        // bDescriptorType
    0,           // bDescriptorSubtype: header functional descriptor
    0x10, 0x01,  // bcdCDC: CDC spec release number in BCD

    /* ACM Functional Descriptor */
    4,     // bLength: size of the descriptor in Bytes
    0x24,  // bDescriptorType
    2,     // bDescriptorSubtype: ACM Functional Descriptor
    0x02,  // bmCapabilities: Supports subset of ACM commands

    /* Union Functional Descriptor */
    5,     // bLength: size of the descriptor in Bytes
    0x24,  // bDescriptorType
    6,     // bDescriptorSubtype: Union Functional Descriptor
    0,     // bControlInterface: master
    1,     // bSubordinateInterface: slave

    /* Call Management Functional Descriptor */
    5,     // bLength: size of the descriptor in Bytes
    0x24,  // bDescriptorType
    1,     // bDescriptorSubtype: Call Management Functional Descriptor
    0x03,  // bmCapabilities: DIY
    1,     // bDataInterface: data interface index

    /* Endpoint Descriptor */
    7,                           // bLength
    USBDESCR_ENDPOINT,           // bDescriptorType
    0x83,                        // bEndpointAddress: IN endpoint number 3
    0x03,                        // bmAttributes: Interrupt endpoint
    8, 0,                        // wMaxPacketSize: maximum packet size
    USB_CFG_INTR_POLL_INTERVAL,  // bInterval: in ms

    /* Data Class Interface Descriptor for CDC-ACM  */
    9,                   // bLength
    USBDESCR_INTERFACE,  // bDescriptorType
    1,                   // bInterfaceNumber: index of this interface
    0,     // bAlternateSetting: alternate setting for this interface
    2,     // bNumEndpoints: number of endpoints used for this interface
    0x0a,  // bInterfaceClass: Data Interface Class Codes
    0,     // bInterfaceSubclass: Data Interface Sublass Codes
    0,     // bDeviceProtocol: Data Interface Class Protocol Codes
    0,     // iInterface: Index of String Descriptor Describing this interface

    /* Endpoint Descriptor */
    7,                  // bLength
    USBDESCR_ENDPOINT,  // bDescriptorType
    0x01,               // bEndpointAddress: OUT endpoint number 1
    0x03,               // bmAttributes: Bulk endpoint
    8, 0,               // wMaxPacketSize: maximum packet size
    0,                  // bInterval: in ms

    /* Endpoint Descriptor */
    7,                  // bLength
    USBDESCR_ENDPOINT,  // bDescriptorType
    0x81,               // bEndpointAddress: IN endpoint number 1
    0x03,               // bmAttributes: Bulk endpoint
    8, 0,               // wMaxPacketSize: maximum packet size
    0,                  // bInterval: in ms
};

USB_PUBLIC usbMsgLen_t usbFunctionDescriptor(usbRequest_t *rq)
{
    // uchar *p = 0, len = 0;
    // if (rq->wValue.bytes[1] == USBDESCR_DEVICE) {
    //     p = (uchar *)deviceDescriptor;
    //     len = sizeof(deviceDescriptor);
    // } else {
    //     p = (uchar *)(myDescriptorConfiguration);
    //     len = sizeof(myDescriptorConfiguration);
    // }
    // usbMsgPtr = (usbMsgPtr_t)p;
    // return len;

    usbMsgPtr = (usbMsgPtr_t)(configurationDescriptor);
    return sizeof(configurationDescriptor);
}

USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t *rq = (void *)data;

    // request using usbFunctionRead()/usbFunctionWrite()
    if (rq->bRequest == 0) return 0xff;
    // default for not implemented requests: return no data back to host
    return 0;
}

/* -------------------------------------------------------------------------
 */

static void hardwareInit(void)
{
    /* activate pull-ups except on USB lines */
    USB_CFG_IOPORT =
        (uchar) ~((1 << USB_CFG_DMINUS_BIT) | (1 << USB_CFG_DPLUS_BIT));
    /* all pins input except USB (-> USB reset) */
#ifdef USB_CFG_PULLUP_IOPORT /* use usbDeviceConnect()/usbDeviceDisconnect() \
                                if available */
    USBDDR = 0;              /* we do RESET by deactivating pullup */
    usbDeviceDisconnect();
#else
    USBDDR = (1 << USB_CFG_DMINUS_BIT) | (1 << USB_CFG_DPLUS_BIT);
#endif

    /* 250 ms disconnect */
    wdt_reset();
    _delay_ms(250);

#ifdef USB_CFG_PULLUP_IOPORT
    usbDeviceConnect();
#else
    USBDDR = 0; /*  remove USB reset condition */
#endif
}

int main(void)
{
    uchar i;

    usbInit();
    usbDeviceDisconnect(); /* enforce re-enumeration, do this while interrupts
                              are disabled! */
    i = 0;
    while (--i) { /* fake USB disconnect for > 250 ms */
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
    for (;;) { /* main event loop */
        usbPoll();
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
