avr-gcc -I".." -I"../../usbdrv" -mmcu=atmega328p -DF_CPU=16000000UL -Wall -gdwarf-2 -Os -fsigned-char -MD -MP -MT usbdrv.o -MF tmp/usbdrv.o.d  -c  ../../usbdrv/usbdrv.c
avr-gcc -I".." -I"../../usbdrv" -mmcu=atmega328p -DF_CPU=16000000UL -mmcu=atmega328p -DF_CPU=16000000UL -Wall -gdwarf-2 -Os -fsigned-char -MD -MP -MT usbdrvasm.o -MF tmp/usbdrvasm.o.d  -x assembler-with-cpp -Wa,-gdwarf2 -c  ../../usbdrv/usbdrvasm.S
avr-gcc -I".." -I"../../usbdrv" -mmcu=atmega328p -DF_CPU=16000000UL -Wall -gdwarf-2 -Os -fsigned-char -MD -MP -MT oddebug.o -MF tmp/oddebug.o.d  -c  ../../usbdrv/oddebug.c
avr-gcc -I".." -I"../../usbdrv" -mmcu=atmega328p -DF_CPU=16000000UL -Wall -gdwarf-2 -Os -fsigned-char -MD -MP -MT serial.o -MF tmp/serial.o.d  -c  ../serial.c
avr-gcc -I".." -I"../../usbdrv" -mmcu=atmega328p -DF_CPU=16000000UL -Wall -gdwarf-2 -Os -fsigned-char -MD -MP -MT main.o -MF tmp/main.o.d  -c  ../main.c
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL  usbdrv.o usbdrvasm.o oddebug.o serial.o main.o     -o binary.elf
avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature  binary.elf binary.hex