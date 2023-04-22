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
#include <string.h>
#include <avr/pgmspace.h> /* required by usbdrv.h */

// #include <avr/wdt.h>
#include <avr/eeprom.h>
#include <util/delay.h> /* for _delay_ms() */

#include "usbdrv.h"
#if USE_INCLUDE
#include "usbdrv.c"
#endif

/* defines for 'requestType' */
#define REQUEST_TYPE_LINE_CODING 0    // CDC GET/SET_LINE_CODING
#define REQUEST_TYPE_HID_FIRST 1      // first block of HID reporting
#define REQUEST_TYPE_HID_SUBSEQUENT 2 // subsequent block of HID reporting
#define REQUEST_TYPE_HID_DEBUGDATA 3  // read/write data from/to debug interface
#define REQUEST_TYPE_VENDOR 4         // vendor request for get/set debug data

/* The following variables store the status of the current data transfer */
static uchar currentAddress;
static uchar bytesRemaining;
static uchar requestType;
static uchar reportId;
static char sample_data[8] = {'f', 'f', 'f', 'f', 'f', 'f', 'f', 'f'};

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

uchar usbFunctionWrite(uchar *data, uchar len)
{
    if (len > sizeof(sample_data)) // Check if the received data is larger than the sample_data buffer
    {
        len = sizeof(sample_data); // If yes, limit the data length to the size of the buffer
    }
    memcpy(&sample_data, data, len);
    return 1;
}

uchar usbFunctionRead(uchar *data, uchar len)
{
    if (len > sizeof(sample_data)) // Check if the requested data size is larger than the sample_data buffer
    {
        len = sizeof(sample_data); // If yes, limit the data length to the size of the buffer
    }
    memcpy(data, &sample_data, len);
    return len;
}

static PROGMEM const char configurationDescriptor[] = {
    9,               // bLength: size of the descriptor in Bytes
    USBDESCR_CONFIG, // bDescriptorType
    41,
    0,                         // wTotalLength: Total length in bytes of data returned
    1,                         // bNumInterfaces
    1,                         // bConfigurationValue: argument value to select this
                               // configuration
    0,                         // iConfiguration:
    0x80,                      // bmAttributes: Bus Powered
    USB_CFG_MAX_BUS_POWER / 2, // bMaxPower: max power consumption in 2mA

    /* Communications Class Interface Descriptor for CDC-ACM Control */
    9,                  // bLength: size of the descriptor in Bytes
    USBDESCR_INTERFACE, // bDescriptorType

    0, // bInterfaceNumber: index of this interface
    0, // bAlternateSetting: alternate setting for this interface
    2, // bNumEndpoints: number of endpoints used for this interface
    USB_CFG_INTERFACE_CLASS, USB_CFG_INTERFACE_SUBCLASS,
    USB_CFG_INTERFACE_PROTOCOL,
    0, // iInterface: Index of String Descriptor Describing this
       // interface

    /* HID Descriptors */
    9,            // bLength
    USBDESCR_HID, // bDescriptorType
    0x01, 0x01,   // bcdHID: BCD representation of HID version
    0x1e,         // bCountryCode: Taiwan (30)
    0x01,         // bNumDescriptors: number of cs descriptors
    0x22,         // bDescriptorType: HID Report
    USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH,
    0, // wDescriptorLength: total length of report descriptor

    /* Endpoint Descriptor */
    7,                          // bLength
    USBDESCR_ENDPOINT,          // bDescriptorType
    0x81,                       // bEndpointAddress: IN endpoint number 1
    0x03,                       // bmAttributes: Interrupt endpoint
    0x08, 0x00,                 // wMaxPacketSize: maximum packet size                         change here   WCT
    USB_CFG_INTR_POLL_INTERVAL, // bInterval: in ms

    /* Endpoint Descriptor */
    7,                          // bLength
    USBDESCR_ENDPOINT,          // bDescriptorType
    0x01,                       // bEndpointAddress: OUT endpoint number 1
    0x03,                       // bmAttributes: Interrupt endpoint
    0x08, 0x00,                 // wMaxPacketSize: maximum packet size                         change here   WCT     
    USB_CFG_INTR_POLL_INTERVAL, // bInterval: in ms
};

const PROGMEM char usbDescriptorHidReport[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
    0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
    0x09, 0x00,       // USAGE (Undefined)
    0xa1, 0x01,       // COLLECTION (Application)
    0x15, 0x00,       //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00, //   LOGICAL_MAXIMUM (255)
    0x75, 0x40,       //   REPORT_SIZE (32)                                                     change here   WCT
    0x95, 0x01,       //   REPORT_COUNT (1)
    0x09, 0x00,       //   USAGE (Undefined)
    0x82, 0x02, 0x01, //   INPUT (Data, Var, Abs, Buf)
    0x09, 0x00,       //   USAGE (Undefined)
    0x92, 0x02, 0x01, //   OUTPUT (Data, Var, Abs, Buf)
    0xc0              // END_COLLECTION
};


USB_PUBLIC usbMsgLen_t usbFunctionDescriptor(usbRequest_t *rq)
{
    uchar *p = 0, len = 0;
    if (rq->wValue.bytes[1] == USBDESCR_CONFIG)
    {
        p = (uchar *)configurationDescriptor;
        len = sizeof(configurationDescriptor);
    }
    else
    {
        p = (uchar *)(configurationDescriptor + 18);
        len = 9;
    }
    usbMsgPtr = (usbMsgPtr_t)p;
    return len;
}

USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t *rq = (void *)data;
    uchar rqType = rq->bmRequestType & USBRQ_TYPE_MASK;
    reportId = rq->wValue.bytes[0];

    if (rqType != USBRQ_TYPE_CLASS)
        return 0;
    if (rq->bRequest == USBRQ_HID_GET_REPORT)
    {
        return USB_NO_MSG;
    }
    else if (rq->bRequest == USBRQ_HID_SET_REPORT)
    {
        return USB_NO_MSG;
    }
    // the driver counts the total number of bytes for us
    return 0xff;
}

/* -------------------------------------------------------------------------
 */

// static void hardwareInit(void)
// {
//     /* activate pull-ups except on USB lines */
//     USB_CFG_IOPORT =
//         (uchar) ~((1 << USB_CFG_DMINUS_BIT) | (1 << USB_CFG_DPLUS_BIT));
//     /* all pins input except USB (-> USB reset) */
// #ifdef USB_CFG_PULLUP_IOPORT /* use usbDeviceConnect()/usbDeviceDisconnect()
// \
//                                 if available */
//     USBDDR = 0;              /* we do RESET by deactivating pullup */
//     usbDeviceDisconnect();
// #else
//     USBDDR = (1 << USB_CFG_DMINUS_BIT) | (1 << USB_CFG_DPLUS_BIT);
// #endif

//     /* 250 ms disconnect */
//     wdt_reset();
//     _delay_ms(250);

// #ifdef USB_CFG_PULLUP_IOPORT
//     usbDeviceConnect();
// #else
//     USBDDR = 0; /*  remove USB reset condition */
// #endif
// }

int main(void)
{
    uchar i;

    usbInit();
    usbDeviceDisconnect(); /* enforce re-enumeration, do this while interrupts
                              are disabled! */
    i = 0;
    while (--i)
    { /* fake USB disconnect for > 250 ms */
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
    for (;;)
    { /* main event loop */
        usbPoll();
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
