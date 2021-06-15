// Author: Dylan Muller
// Student Number: MLLDYL002

#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>


extern unsigned char ypr;
extern unsigned char xpr;
extern unsigned char zpr;
extern unsigned char qpr;
extern unsigned char xlb[];
extern unsigned char ylb[];

typedef union dword_usb
{
    unsigned char bytes[4];
    unsigned long lvalue;
} dword_usb_t;

extern void puart();
extern void iuart(unsigned char db,       //databits,
                  unsigned char dp,       // parity,
                  unsigned char sb,       // stop bit,
                  unsigned long tx_rate); // transfer rate);

#endif
