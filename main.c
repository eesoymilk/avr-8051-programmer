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
#include <avr/pgmspace.h>  /* required by usbdrv.h */
#include <string.h>

#include "inc/chip.h"
#include "inc/param.h"
#include "inc/program.h"
// #include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <util/delay.h> /* for _delay_ms() */

#include "usbdrv.h"
#if USE_INCLUDE
#include "usbdrv.c"
#endif

/* defines for 'Report Type' */
#define USB_HID_REPORT_TYPE_INPUT 1
#define USB_HID_REPORT_TYPE_OUTPUT 2
#define USB_HID_REPORT_TYPE_FEATURE 3

/* The following variables store the status of the current data transfer */
//----------------------------------------WCT modify
//------------------------------------//
// static uchar currentAddress;
// static uchar bytesRemaining;
// static uchar requestType;
//----------------------------------------end
// modify-------------------------------------//
static uchar reportId, reportType;
static uchar buffer[8] = {0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77};
static uchar inputBuffer[8] = "soymilk!";
static uchar featureBuffer = 'f';
typedef enum { IDLE, WRITING_FLASH, READING_FLASH, TEST } State;
typedef enum { WRITE = 'W', READ = 'R', END = 'E' } Command;

State currentState = IDLE;

static unsigned int program_cnt = 0;
// uchar erase_flag;
// uchar write_flag;
// uchar read_flag;

// command operation codes
const uchar CMD_SETTING_IO[6] = {0xaa, 0xbb, 0xcc, 0xdd, 0x00, 0x01};
const uchar CMD_RELEASE_IO[6] = {0xaa, 0xbb, 0xcc, 0xdd, 0x00, 0x02};
const uchar CMD_PROG_EN[6] = {0xaa, 0xbb, 0xcc, 0xdd, 0x00, 0x03};
const uchar CMD_ERASE_FLASH[6] = {0xaa, 0xbb, 0xcc, 0xdd, 0x00, 0x04};
const uchar CMD_RELEASE_PORT[6] = {0xaa, 0xbb, 0xcc, 0xdd, 0x00, 0x05};
//                                 cmd   cmd   cmd   cmd    no   read    h_b_n
//                                 l_b_n
const uchar CMD_READ[6] = {0xaa, 0xbb, 0xcc, 0xdd, 0x00, 0x06};
//                                 cmd   cmd   cmd   cmd    no   write   h_b_n
//                                 l_b_n
const uchar CMD_WRITE[6] = {0xaa, 0xbb, 0xcc, 0xdd, 0x00, 0x07};

// operation
#define SETTING_IO 6
#define RELEASE_IO 7
#define PROG_EN 8
#define ERASE_FLASH 9
#define READ 10
#define WRITE 11
#define RELEASE_PORT 12

// mode
#define IDLE 13
#define WRITING_FLASH 14
#define READING_FLASH 15
#define TEST 18

// set portB cmd
#define PORTB_BURN 16
#define PORTB_RELEASE 17
/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

void Set_PORTB(uchar Set_Cmd);

int compare_commands(const uchar *a, const uchar *b, int len)
{
    for (int i = 0; i < len; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

int CMD_judge(uchar *buf)
{
    if (compare_commands(buf, CMD_SETTING_IO, 6)) {
        return SETTING_IO;
    }
    if (compare_commands(buf, CMD_RELEASE_IO, 6)) {
        return RELEASE_IO;
    }
    if (compare_commands(buf, CMD_PROG_EN, 6)) {
        return PROG_EN;
    }
    if (compare_commands(buf, CMD_ERASE_FLASH, 6)) {
        return ERASE_FLASH;
    }
    if (compare_commands(buf, CMD_READ, 6)) {
        return READ;
    }
    if (compare_commands(buf, CMD_WRITE, 6)) {
        return WRITE;
    }
    if (compare_commands(buf, CMD_RELEASE_PORT, 6)) {
        return RELEASE_PORT;
    }
    return -1;
}
// Step1: Setting IO for Program Mode//
// Step2: Programming Enable//
// Step3: Erase Chip Flash Area//
// Step4: Write Chip Flash Area//
// Step6: Release IO for AT89S51 can start to work//

static uchar mode;
static int RW_cnt;

uchar usbFunctionWrite(uchar *data, uchar len)
{
    // if (len > sizeof(buffer))  // Check if the received data is larger than
    //                            // the buffer buffer
    //     len = sizeof(buffer);  // If yes, limit the data length to the size
    //                            // of the buffer

    if (reportType == USB_HID_REPORT_TYPE_FEATURE) {
        switch ((char)*data) {
            case WRITE:
                /* code */
                break;

            default:
                break;
        }
    } else if (reportType == USB_HID_REPORT_TYPE_OUTPUT) {
    }

    memcpy(buffer, data, len);

    if (mode == IDLE || mode == TEST) {
        switch (CMD_judge(buffer)) {
            case WRITE:
                PORTC &= ~(1 << PC1);  // additional setting for Program Mode
                program_cnt = 0;
                AT89S51_Program_erase();
                program_cnt = 0;
                mode = WRITING_FLASH;
                Set_PORTB(PORTB_BURN);
                CASE_SETTING_IO();
                CASE_PROG_EN();
                CASE_ERASE_FLASH();
                RW_cnt = 0;
                RW_cnt = buffer[6];
                RW_cnt <<= 8;
                RW_cnt += buffer[7];
                break;
            default:
                mode = TEST;
        }
    } else if (mode == WRITING_FLASH) {
        if (RW_cnt <= 0) {
            while (1) {
                PORTC |= (1 << PC0);
                PORTC |= (1 << PC1);
            }
        }

        CASE_WRITE(buffer, program_cnt);
        program_cnt += 8;
        RW_cnt -= 8;

        if (RW_cnt <= 0) {
            RW_cnt = 0;
            mode = IDLE;
            CASE_RELEASE_IO();
            Set_PORTB(PORTB_RELEASE);
            // for(int k = 0; k<8; k++){buffer[k] = 0x77;}
            program_cnt = 0;
            // wdt_enable(WDTO_15MS);
            // while (1) {}
        }
    } else {
        mode = TEST;
    }

    return 1;
}

// This function is responsible for sending the input/feature reports to
// host.
uchar usbFunctionRead(uchar *data, uchar len)
{
    // // Check if the requested data size is larger than the buffer buffer
    // if (len > sizeof(buffer)) {
    //     // If yes, limit the data length to the size of the buffer
    //     len = sizeof(buffer);
    // }

    if (reportType == USB_HID_REPORT_TYPE_FEATURE) {
        memcpy(data, &featureBuffer, len);
    } else if (reportType == USB_HID_REPORT_TYPE_INPUT) {
        memcpy(data, inputBuffer, len);
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
        0x95, 0x01,        //   REPORT_COUNT (1)
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

/* -------------------------------------------------------------------------
 */

// static void hardwareInit(void)
// {
//     /* activate pull-ups except on USB lines */
//     USB_CFG_IOPORT =
//         (uchar) ~((1 << USB_CFG_DMINUS_BIT) | (1 << USB_CFG_DPLUS_BIT));
//     /* all pins input except USB (-> USB reset) */
// #ifdef USB_CFG_PULLUP_IOPORT /* use usbDeviceConnect()/usbDeviceDisconnect()
//
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
    mode = IDLE;

    DDRC |= (1 << PC0) | (1 << PC1);
    PORTC &= ~(1 << PC0);
    PORTC |= (1 << PC1);

    usbInit();
    usbDeviceDisconnect(); /* enforce re-enumeration, do this while interrupts
                              are disabled! */
    i = 0;
    while (--i) {          /* fake USB disconnect for > 250 ms */
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
    for (;;) { /* main event loop */
        usbPoll();

        if (mode == IDLE) {
            PORTC |= (1 << PC1);
        } else if (mode == WRITING_FLASH) {
            PORTC &= ~(1 << PC1);
        } else if (mode == TEST) {
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

void Set_PORTB(uchar Set_Cmd)
{
    if (Set_Cmd == PORTB_BURN) {
        DDRB |= (1 << PB2) | (1 << PB3) | (1 << PB5);
        DDRB &= ~(1 << PB4);

        PORTB |= (1 << PB2);
        PORTB |= (1 << PB3);
        PORTB &= ~(1 << PB5);
    } else if (Set_Cmd == PORTB_RELEASE) {
        DDRB &= ~((1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4) |
                  (1 << PB5) | (1 << PB6) | (1 << PB7));
    } else {
        PORTC |= (1 << PC0);
        PORTC |= (1 << PC1);
    }
}
/* ------------------------------------------------------------------------- */