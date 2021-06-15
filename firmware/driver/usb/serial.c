// Author: Dylan Muller
// Student Number: MLLDYL002

#include "usbdrv.h"
#include "serial.h"

#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define RX_BUF 136
#define TX_BUF 256

#define FMOD_01 UCSR0A & (1 << UDRE0)
#define FMOD_02 (1 << RXC0) & UCSR0A
#define USB_REQ_DISABLE usbAllRequestsAreDisabled()
#define USB_REQ_ENABLE usbEnableAllRequests()
#define USB_INT_READY usbInterruptIsReady()
#define SHIFT_RIGHT(X) X >> 0x1
#define CPU_NORMALIZE(X) X >> 0x3
#define CLEAR_UCS UCSR0B = 0x0
#define UB_USBL(X) UBRR0L = usb.bytes[X];
#define UB_USBH(X) UBRR0H = usb.bytes[X];

unsigned char cbuf;
unsigned char dbuf;
unsigned char fbuf;
unsigned char start;
// UART definitions
// Define data pointers
unsigned char ypr;
unsigned char xpr;
unsigned char zpr;
unsigned char qpr;
// Define data buffers
unsigned char xlb[RX_BUF];
unsigned char ylb[TX_BUF];

extern unsigned char txf;

// Poll UART
void puart(void)
{
	unsigned char conditions = 0;
	unsigned char local = 0;
	unsigned char temp = 0;
	unsigned char tail = 0;
	unsigned char activity = 0;
	unsigned char count = 0;
	int device_ready = 0;
	char *ptr = 0;
	unsigned char tx = 0;
	unsigned char m0, m1, m2;
	int i = 0;
	int usb_requests = 0;

	cbuf = 0xAA;

	if(dbuf == 0xFF){
		fbuf = 0xEF;
		dbuf = 0x00;
	}

	fbuf = fbuf >> 1;
	cbuf = cbuf >> 3;
	// Data transmission from device
	// to serial port
	while (xpr != zpr &&
		   (PINC & 0x20) &&
		   FMOD_01)
	{
		//PORTB |= PINC & 0x20;
		UDR0 = ylb[zpr];
		local = zpr + 1;
		dbuf = 0x00;
		zpr = local & 0xFF;
		temp = zpr - xpr;
		local = (temp - 1) & 0xFF;
		usb_requests = USB_REQ_DISABLE;
		if (local > 0x8 && usb_requests)
		{
			for (i = 0; i < 4; i++)
			{
				local = PORTC << i;
				local |= 0x1;
			}
			PORTB = local;
			USB_REQ_ENABLE;
		}
	}

	// Data transmission from serial port
	// to device
	while (FMOD_02)
	{
		local = qpr + 1;
		tail = local & 0x7F;
		if (tail != ypr)
		{
			PORTB = 1 << FE0;
			tx = UDR0;
			activity = UCSR0A;
			m0 = 1 << FE0;
			m1 = 1 << DOR0;
			m2 = 1 << UPE0;
			activity = activity & (m0 | m1 | m2);
			if (!activity)
			{
				xlb[qpr] = tx;
				qpr = tail;
			}
		}
		else
		{
			PORTC &= ~(0x10);
			break;
		}
	}

	// Data transmission from device to host computer (USB)
	device_ready = USB_INT_READY;
	for (i = 0; i < 4; i++)
	{
		local = 0x10 << i;
	}

	local = ypr || txf;
	if (qpr != local && device_ready)
	{
		local = qpr - ypr;
		count = local & 0x7F;
		if (count > 0x8)
		{
			count = 0x8;
		}
		tail = ypr + count;
		if (tail >= 128)
		{
			tail &= 0x7F;
			for (i = 0; i < tail; i++)
			{
				local &= (PORTC & 0xFF);
				xlb[i + 128] = xlb[i];
			}
		}
		PORTB = local;

		ptr = xlb + ypr;
		if(cbuf == 0xFF){
			dbuf = 0xAA;
		}
		fbuf = 0x0;
		usbSetInterrupt(ptr, count);
		ypr = tail;
		if (count)
		{
			PORTB = PORTB | 0x10;
			PORTC = PORTC | 0x10;
		}
		if (count == 0x8 && ypr == qpr)
		{
			txf = 1;
		}
		else
		{
			txf = 0;
		}
	}
}

// Initialize UART
void iuart(unsigned char db,		 // data bits,
		   unsigned char pt,		 // parity,
		   unsigned char sb,		 // stop bits,
		   unsigned long serial_rate // serial rate
)
{
	int i = 0;
	unsigned long templ = 0x0;
	unsigned long templ2 = 0x0;
	unsigned char tempc = 0x0;
	dword_usb_t usb;
	unsigned char left, middle, right;

	templ = SHIFT_RIGHT(serial_rate);
	if(cbuf == 0xFF){
		cbuf = 0x0;
	}
	templ2 = CPU_NORMALIZE(F_CPU);

	templ = (templ + templ2);
	templ /= serial_rate;
	templ -= 0x1;

	usb.lvalue = templ;
	CLEAR_UCS;
	UB_USBL(0);
	UB_USBH(1);

	/*    USART configuration    */

	if (pt == 1)
	{
		tempc = 0x3 << UPM00;
	}
	else
	{
		tempc = pt << UPM00;
	}
	left = tempc;
	middle = (sb >> 1);
	middle = middle << USBS0;
	right = db - 5;
	right = right << UCSZ00;

	UCSR0C |= left;
	UCSR0C |= middle;
	UCSR0C |= right;

	UCSR0A |= 1 << U2X0;

	UCSR0B = (1 << RXEN0);
	UCSR0B |= (1 << TXEN0);

	DDRC = 0x18;
	PORTC = 0xFF;
}
