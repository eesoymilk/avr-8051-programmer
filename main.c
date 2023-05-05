/* Name: main.c
 * Project: USB 8051 Hex Programmer
 * Author: Yu-wei Chang, Wei-chen Tsai
 * Creation Date: 2023-05-08
 */

#include <avr/interrupt.h> /* for sei() */
#include <avr/io.h>
#include <avr/pgmspace.h>  /* required by usbdrv.h */
#include <string.h>
#include <util/delay.h>

#include "inc/atmega328_51_serial.h"
#include "usbdrv.h"
#if USE_INCLUDE
#include "usbdrv.c"
#endif

/* defines for 'Report Type' */
#define USB_HID_REPORT_TYPE_INPUT 1
#define USB_HID_REPORT_TYPE_OUTPUT 2
#define USB_HID_REPORT_TYPE_FEATURE 3

typedef enum { IDLE, WRITING_FLASH, READING_FLASH, TEST } State;
typedef enum { WRITE = 'W', READ = 'R', END = 'E' } Command;

static State currentState = IDLE;
static uint16_t currentAddress = 0;
static uint16_t hexSize;
static uchar reportId, reportType;
static uchar hexDataBuffer[8] = "soymilk!";
static uchar featureBuffer[3] = {'a', 'b', 'c'};

// set portB cmd
#define PORTB_BURN 16
#define PORTB_RELEASE 17
/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

void Set_PORTB(uchar Set_Cmd);

// Step1: Setting IO for Program Mode//
// Step2: Programming Enable//
// Step3: Erase Chip Flash Area//
// Step4: Write Chip Flash Area//
// Step6: Release IO for AT89S51 can start to work//

uchar usbFunctionWrite(uchar *data, uchar len)
{
    // if (len > sizeof(buffer))  // Check if the received data is larger than
    //                            // the buffer buffer
    //     len = sizeof(buffer);  // If yes, limit the data length to the size
    //                            // of the buffer

    if (reportType == USB_HID_REPORT_TYPE_FEATURE) {
        memcpy(featureBuffer, data, len);
        char command = featureBuffer[0];
        switch (command) {
            case WRITE:
                hexSize = (featureBuffer[1] << 8) | featureBuffer[2];

                ATMega328SPIInit();
                AT8051SPIInit();
                AT8051ProgrammingEnable();
                AT8951ChipErase();

                _delay_ms(500);

                currentState = WRITING_FLASH;
                currentAddress = 0;
                break;

            case END:
                AT8051SPIRelease();
                ATMega328SPIRelease();
                currentState = IDLE;
                break;

            default:
                break;
        }
    } else if (currentState == WRITING_FLASH) {
        PORTC |= (1 << PC1);
        memcpy(hexDataBuffer, data, len);
        AT8951WriteOctet(currentAddress, hexDataBuffer);
        currentAddress += 8;
    }

    return 1;
}

// This function is responsible for sending the input/feature reports to
// host.
uchar usbFunctionRead(uchar *data, uchar len)
{
    if (reportType == USB_HID_REPORT_TYPE_FEATURE) {
        memcpy(data, featureBuffer, len);
    } else if (reportType == USB_HID_REPORT_TYPE_INPUT) {
        memcpy(data, hexDataBuffer, len);
    }

    return len;
}

static PROGMEM const char configurationDescriptor[] = {
    9,                // bLength: size of the descriptor in Bytes
    USBDESCR_CONFIG,  // bDescriptorType
    41,
    0,                // wTotalLength: Total length in bytes of data returned
    1,                // bNumInterfaces
    1,                // bConfigurationValue: argument value
    0,                // iConfiguration:
    0x80,             // bmAttributes: Bus Powered
    USB_CFG_MAX_BUS_POWER / 2,  // bMaxPower: max power consumption in 2mA

    /* Communications Class Interface Descriptor for CDC-ACM Control */
    9,                   // bLength: size of the descriptor in Bytes
    USBDESCR_INTERFACE,  // bDescriptorType

    0,                   // bInterfaceNumber: index of this interface
    0,  // bAlternateSetting: alternate setting for this interface
    2,  // bNumEndpoints: number of endpoints used for this interface
    USB_CFG_INTERFACE_CLASS, USB_CFG_INTERFACE_SUBCLASS,
    USB_CFG_INTERFACE_PROTOCOL,
    0,  // iInterface: Index of String Descriptor

    /* HID Descriptors */
    9,             // bLength
    USBDESCR_HID,  // bDescriptorType
    0x01, 0x01,    // bcdHID: BCD representation of HID version
    0x1e,          // bCountryCode: Taiwan (30)
    0x01,          // bNumDescriptors: number of cs descriptors
    0x22,          // bDescriptorType: HID Report
    USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH,
    0,             // wDescriptorLength: total length of report descriptor

    /* Endpoint Descriptor */
    7,                  // bLength
    USBDESCR_ENDPOINT,  // bDescriptorType
    0x81,               // bEndpointAddress: IN endpoint number 1
    0x03,               // bmAttributes: Interrupt endpoint
    0x08, 0x00,         // wMaxPacketSize: maximum packet size change here   WCT
    USB_CFG_INTR_POLL_INTERVAL,  // bInterval: in ms

    /* Endpoint Descriptor */
    7,                  // bLength
    USBDESCR_ENDPOINT,  // bDescriptorType
    0x01,               // bEndpointAddress: OUT endpoint number 1
    0x03,               // bmAttributes: Interrupt endpoint
    0x08, 0x00,         // wMaxPacketSize: maximum packet size change here   WCT
    USB_CFG_INTR_POLL_INTERVAL,  // bInterval: in ms
};

const PROGMEM char
    usbDescriptorHidReport[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
        0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
        0x09, 0x00,        // USAGE (Undefined)
        0xa1, 0x01,        // COLLECTION (Application)
        0x15, 0x00,        //   LOGICAL_MINIMUM (0)
        0x26, 0xff, 0x00,  //   LOGICAL_MAXIMUM (255)
        0x75, 0x08,        //   REPORT_SIZE (8)
        0x95, 0x08,        //   REPORT_COUNT (8)
        0x09, 0x00,        //   USAGE (Undefined)
        0x82, 0x02, 0x01,  //   INPUT (Data, Var, Abs, Buf)
        0x09, 0x00,        //   USAGE (Undefined)
        0x92, 0x02, 0x01,  //   OUTPUT (Data, Var, Abs, Buf)
        0x95, 0x03,        //   REPORT_COUNT (3)
        0x09, 0x00,        //   USAGE (Undefined)
        0xB2, 0x02, 0x01,  //   FEATURE (Data, Var, Abs, Buf)
        0xc0               // END_COLLECTION
};

USB_PUBLIC usbMsgLen_t usbFunctionDescriptor(usbRequest_t *rq)
{
    uchar *p = 0, len = 0;
    if (rq->wValue.bytes[1] == USBDESCR_CONFIG) {
        p = (uchar *)configurationDescriptor;
        len = sizeof(configurationDescriptor);
    } else {
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

    if (rqType != USBRQ_TYPE_CLASS) {
        return 0;
    }

    reportId = rq->wValue.bytes[0];
    reportType = rq->wValue.bytes[1];
    if (rq->bRequest == USBRQ_HID_GET_REPORT) {
        return USB_NO_MSG;
    } else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
        return USB_NO_MSG;
    }
    // the driver counts the total number of bytes for us
    return 0xff;
}

int main(void)
{
    uchar i = 0;

    DDRC |= (1 << PC0) | (1 << PC1);
    PORTC &= ~(1 << PC0);
    PORTC |= (1 << PC1);

    usbInit();
    // enforce re-enumeration, do this while interrupts are disabled !
    usbDeviceDisconnect();

    // fake USB disconnect for > 250 ms
    while (--i) _delay_ms(1);

    usbDeviceConnect();
    sei();

    for (;;) { /* main event loop */
        usbPoll();

        if (currentState == IDLE) {
            PORTC |= (1 << PC1);
        } else if (currentState == WRITING_FLASH) {
            PORTC &= ~(1 << PC1);
        } else if (currentState == TEST) {
            PORTC &= ~(1 << PC0);
            PORTC |= (1 << PC1);
        } else {
            while (1) {
                PORTC |= (1 << PC0);
                PORTC &= ~(1 << PC1);
            }
        }
    }
    return 0;
}
