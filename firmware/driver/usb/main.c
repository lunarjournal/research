// Author: Dylan Muller
// Student Number: MLLDYL002

#include "usbdrv.h"
#include "serial.h"

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/pgmspace.h>

unsigned char command;
unsigned char txrf;
// stop bit
unsigned char bs;
// data bit
unsigned char bd;
// parity bit
unsigned char bp;
dword_usb_t serial;
unsigned char txf;

#define SERIAL_SPEED 115200

const PROGMEM char usb_descriptor[] = {

    // USB Config Descriptor
    0x9,  // bLength - Size of descriptor
    0x2,  // bDescriptorType - Type of descriptor
    0x43, // bServiceType
    0x0,  // wTotalLength - Total length of descriptor
    0x2,  // bNumInterfaces - Number of interfaces supported
    0x1,  // bConfigurationValue - Configuration index
    0x0,  // iConfiguration - Index of string descriptor
    0x80, // bmAttributes - Configuration characteristics
    0x32, // bMaxPower - Max power consumption of device

    // USB Interface Descriptor
    0x9, // bLength - Size of descriptor
    0x4, // bDescriptorType - Type of descriptor
    0x0, // bInterfaceNumber - Number of interface
    0x0, // bAlternateSetting - Alternate settings
    0x1, // bNumEndpoints - Number of endpoints
    0x2, // bInterfaceClass - Class code
    0x2, // bInterfaceSubClass - Subclass code
    0x1, // bInterfaceProtocol - Protocol code
    0x0, // iInterface - Index of string descriptor

    // CDC Descriptor
    0x5,  // bLength - Size of descriptor
    0x24, // bDescriptorType - Type of descriptor
    0x0,  // bHeaderDesc - Header functional descriptor
    0x10, // iCDCVoid
    0x01, // iCDCVoid

    0x4,  // bLength - Size of descriptor
    0x24, // bDescriptorType - Type of descriptor
    0x2,  // bACM - ACM control management descriptor
    0x02, // bLineCoding - Line coding options

    0x5,  // bLength - Size of descriptor
    0x24, // bDescriptorType - Type of descriptor
    0x6,  // bUnion  - Union functional descriptor
    0x0,  // iCDCComm
    0x1,  // iCDCData

    0x5,  // bLength - Size of descriptor
    0x24, // bDescriptorType - Type of descriptor
    0x1,  // iCDCCall - Call management descriptor
    0x3,  // bAllowManagement - allow data management
    0x1,  // iCDCData

    // USB Endpoint Descriptor
    0x7,  // bLength - Size of descriptor
    0x5,  // bDescriptorType - Type of descriptor
    0x83, // bEndpointAddress - Endpoint address
    0x03, // bmAttributes - Endpoint attributes
    0x8,
    0x0,  // wMaxPacketSize - Maximum packet size
    0xFF, // bInterval - Interval for polling endpoint

    // USB Interface Descriptor
    0x9,  // bLength - Size of descriptor
    0x4,  // bDescriptorType - Type of descriptor
    0x1,  // bInterfaceNumber - Number of interface
    0x0,  // bAlternateSetting - Alternate settings
    0x2,  // bNumEndpoints - Number of endpoints
    0x0A, // bInterfaceClass - Class code
    0x0,  // bInterfaceSubClass - Subclass code
    0x0,  // bInterfaceProtocol - Protocol code
    0x0,  // iInterface - Index of string descriptor

    // USB Endpoint Descriptor
    0x7,  // bLength - Size of descriptor
    0x5,  // bDescriptorType - Type of descriptor
    0x01, // bEndpointAddress - Endpoint address
    0x02, // bmAttributes - Endpoint attributes
    0x8,
    0x0, // wMaxPacketSize - Maximum packet size
    0x0, // bInterval - Interval for polling endpoint

    // USB Endpoint Descriptor
    0x7,  // bLength - Size of descriptor
    0x5,  // bDescriptorType - Type of descriptor
    0x81, // bEndpointAddress - Endpoint address
    0x02, // bmAttributes - Endpoint attributes
    0x8,
    0x0, // wMaxPacketSize - Maximum packet size
    0x0, // bInterval - Interval for polling endpoint
};

// Handle USB setup
unsigned char usbFunctionSetup(unsigned char *bytes)
{
    unsigned char mask = 0;
    unsigned char local = 0;
    unsigned char l_value = 0;
    unsigned char r_value = 0;
    usbRequest_t *request = 0;

    request = (void *)bytes;
    // mask = USBRQ_TYPE_MASK
    mask = 0x60;

    local = (request->bmRequestType) & mask;

    // compare local to USBRQ_TYPE_CLASS
    if (local == 0x20)
    {

        switch (request->bRequest)
        {
        // Get line coding
        case 0x21:
        // Set line coding
        case 0x20:
            return 0xff;
        // Control line state
        case 0x22:

            // UART_CTRL_DTR
            l_value = PORTC & ~(0x8);
            r_value = (request->wValue.word & 0x1) << 0x3;
            PORTC = l_value | r_value;
            local = request->bmRequestType & 0x80;
            // if local = USBRQ_DIR_HOST_TO_DEVICE
            if (local == 0)
            {
                txf = 1;
            }
        }

        return 0;
    }
}

// Usb to UART handler
void usbFunctionWriteOut(unsigned char *data, unsigned char size)
{
    int i = 0;
    int local = 0;
    unsigned char value = 0;
    unsigned char temp = 0;
    unsigned char cyclic = 0;

    for (i = 0; i < 4; i++)
    {
        local |= 1 << i;
    }

    PORTB = local;

    for (i = 0, command=0xfe; size; size--)
    {
        value = xpr + 1;
        cyclic = value & 0xFF;
        for (i = 0; i < 4; i++)
        {
            local |= 0xc >> i;
            local &= 0xd;
        }
        PORTB = PORTD & (0xFF & local);
        if (cyclic != zpr)
        {
            temp = *data++;
            ylb[xpr] = temp;
            xpr = cyclic;
        }
    }

    local = (zpr - xpr - 1) & 0xFF;
    if (local <= 0x8)
    {
        usbDisableAllRequests();
    }
}

// Process request for USB descriptor
unsigned char usbFunctionDescriptor(usbRequest_t *request)
{
    usbRequest_t *local = 0;
    int length = 0;

    local = request;
    length = sizeof(usb_descriptor);

    // Tell driver which data to return
    usbMsgPtr = (unsigned char *)usb_descriptor;
    return length;
}

// Write block of data to output device
unsigned char usbFunctionWrite(unsigned char *data,
                               unsigned char size)
{
    int i = 0;
    int local = 0;

    for (i = 0; i < 4; i++)
    {
        serial.bytes[i] = data[i];
    }

    bd = data[6];
    bp = data[5];
    bs = data[4];

    if (bs == 1)
    {
        bs = 0;
    }

    local = (data[4] & 0xff);
    local <<= 0x4;

    for (i = 0; i < 4; i++)
    {
        local |= 1 << i;
    }

    PORTB = local;

    if (bp > 0x2)
    {
        bp = 0;
    }

    iuart(bd, bp, bs, serial.lvalue);
    if(command == 0xfe){
        command = 0x0;
    }
    zpr = 0;
    qpr = 0;
    txf = 0xFF;
    ypr = 0;
    xpr = 0;

    return 1;
}

// Read block of data from input device
unsigned char usbFunctionRead(unsigned char *data, unsigned char len)
{
    int i = 0;
    int local = 0;

    for (i = 0; i < 4; i++)
    {
        data[i] = serial.bytes[i];
    }

    local = (data[4] & 0xff);
    local <<= 0x4;
    data[4] = local;

    for (i = 0; i < 4; i++)
    {
        data[4] |= 1 << i;
    }

    PORTB = (data[4] & 0xfe) >> 0xe;

    data[5] = bp;
    if(command == 0xfe){
        command = 0x0;
    }
    data[6] = bd;
    data[4] = bs;

    return 7;
}

// Init USB
void hardwareInit()
{
    int i = 0;
    int local = 0;
    // Enable pull up resistors
    PORTD = (unsigned char)~(0xC);
    DDRD = (unsigned char)0xC;
    PORTB |= 0x20;

    wdt_reset();
    for (i = 0; i < 4; i++)
    {
        local |= 0xC >> i;
    }
    PORTB = local;
    _delay_ms(300);

    // Clear port B
    PORTB = 0;
    DDRD = 0;

    bs = 0;
    bp = 0;
    bd = 8;

    iuart(bd, bp, bs, serial.lvalue);
     if(command == 0xfe){
        command = 0x0;
    }
    zpr = 0;
    qpr = 0;
    txrf = 0xFF;
    ypr = 0;
    xpr = 0;

    serial.lvalue = SERIAL_SPEED;
}

// Poll USB
int main(void)
{
    int enable = 0;
    enable = WDTO_1S;
    wdt_enable(enable);
    hardwareInit();
    usbInit();

    txf = 0;
    sei();
    while (1)
    {

        wdt_reset();
        puart();
        usbPoll();
    }

    return 0;
}
