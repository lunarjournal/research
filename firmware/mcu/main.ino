// Author: Dylan Muller
// Student Number: MLLDYL002

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "rsa.h"
#include "fs.h"

int hexC2bin(const char *s)
{
    int ret = 0;
    int i;
    for (i = 0; i < 2; i++)
    {
        char c = *s++;
        int n = 0;
        if ('0' <= c && c <= '9')
            n = c - '0';
        else if ('a' <= c && c <= 'f')
            n = 10 + c - 'a';
        else if ('A' <= c && c <= 'F')
            n = 10 + c - 'A';
        ret = n + ret * 16;
    }
    return ret;
}
int hex2bin(const char *hex, char *output, int length)
{
    const char *in = hex;
    int i = 0;

    for (i = 0; i < length; i++)
    {
        output[i] = hexC2bin(in);
        in += 2;
    }
}

void hex_print(unsigned char *z, unsigned char size)
{
    for (int i = 0; i < size; i++)
    {
        if (z[i] < 0x10)
            Serial.print("0");
        Serial.print(z[i], HEX);
    }
}

void replace_newline(char *str, int length)
{
    int i = 0;
    for (i = 0; i < length; i++)
    {
        if (str[i] == '\n')
        {
            str[i] = 0;
        }
    }
}

int count_chars(char *str, char c)
{
    int i = 0;
    for (i = 0; str[i]; str[i] == c ? i++ : *str++)
        ;
    return i;
}

int cmd_ok()
{
    Serial.println("OK");
}
int cmd_fail()
{
    Serial.println("FAIL");
}

int trim_buffer(char *buffer, int size)
{
    int i = 0;
    int offset = 0;

    for (i = 0; i < size; i++)
    {
        if (buffer[i] == '$')
        {
            offset = i;
            break;
        }
    }

    for (i = 0; i < size - offset; i++)
    {
        buffer[i] = buffer[i + offset];
    }

    return 0;
}

char output[128];
char *buffer = 0;
char *modulus_buffer = 0;
char *data_buffer = 0;
char *filename_buffer = 0;
int pid = 0;


void free_memory()
{
    free(buffer);
    free(modulus_buffer);
    free(data_buffer);
    free(filename_buffer);
}

void setup()
{
    Serial.begin(115200);
    struct fs_header header = read_fs_header();
    if(header.magic !=0xABCD){
      format_fs();
      Serial.println(F("format complete.."));
    }
}

void loop()
{

    // Poll message
    // Extract command line arguments

    while (Serial.available())
    {
       
        buffer = malloc(400);
        char b = Serial.readBytes(buffer, 400);
        replace_newline(buffer, 400);
        int chars = count_chars(buffer, '$');
        if (chars < 2)
        {
            cmd_fail();
            Serial.flush();
            free(buffer);
            break;
        }
        trim_buffer(buffer, 400);
        char *cmd = strtok(buffer, "$");

        if(strcmp_P(cmd, (PGM_P)F("help")) == 0){
          pid = 7;
        }
        if(strcmp_P(cmd, (PGM_P)F("ready")) == 0)
        {
          pid = 6;
        }
        if (strcmp_P(cmd, (PGM_P)F("list")) == 0)
        {
            pid = 4;
        }
        if (strcmp_P(cmd, (PGM_P)F("format")) == 0)
        {
            pid = 2;
        }

        if (strcmp_P(cmd, (PGM_P)F("read")) == 0)
        {

            filename_buffer = (char *)malloc(6);

            char *filename = strtok(0, "$");
            if (filename == 0)
            {
                cmd_fail();
                free(filename_buffer);
                free(buffer);
                break;
            }

            strcpy(filename_buffer, filename);
            pid = 3;
        }

        if (strcmp_P(cmd, (PGM_P)F("delete")) == 0)
        {

            filename_buffer = (char *)malloc(6);

            char *filename = strtok(0, "$");
            if (filename == 0)
            {
                cmd_fail();
                free(filename_buffer);
                free(buffer);
                break;
            }

            strcpy(filename_buffer, filename);
            pid = 5;
        }
        if (strcmp_P(cmd, (PGM_P)F("encrypt")) == 0)
        {

            modulus_buffer = (char *)malloc(129);
            data_buffer = (char *)malloc(100);
            filename_buffer = (char *)malloc(6);

            char *modulus = strtok(0, "$");
            if (modulus == 0)
            {
                cmd_fail();
                free_memory();
                break;
            }
            if(strlen(modulus) != 256)
            {
              cmd_fail();
              free_memory();
              break;
            }
            hex2bin(modulus, modulus_buffer, 128);
            char *data = strtok(0, "$");
            if (data == 0)
            {
                cmd_fail();
                free_memory();
                break;
            }
            strcpy(data_buffer, data);
            char *filename = strtok(0, "$");
            if (filename == 0)
            {
                cmd_fail();
                free_memory();
                break;
            }
            strcpy(filename_buffer, filename);
            pid = 1;
        }

        free(buffer);
    }

    // Message Processors

    if(pid == 7){
      Serial.println(F("$encrypt$MODULUS$DATA$FILENAME$ - Encrypt file"));
      Serial.println(F("$list$ - List all files on filesystem"));
      Serial.println(F("$read$FILENAME$ - Read file"));
      Serial.println(F("$delete$FILENAME$ - Delete file"));
      Serial.println(F("$format$ - Format filesystem"));
      Serial.println(F("Note: MODULUS specified as hex string"));
      Serial.println(F("Note: data returned from $read$ is hex string"));
      
      pid = 0;
    }
    if (pid == 6)
    {
      Serial.println("READY");
      pid = 0;
    }
    // $delete$FILENAME$
    // Deletes file from filesystem
    if (pid == 5)
    {
        struct fs_header header = read_fs_header();
        signed int result = delete_file(&header, filename_buffer);
        if (result == -1)
        {
            cmd_fail();
        }
        else
        {
            cmd_ok();
        }
        free(filename_buffer);
        pid = 0;
    }

    // $list$
    // List all files on filesystem
    if (pid == 4)
    {
        int i = 0;
        int n = 0;
        struct fs_header header = read_fs_header();
        struct fs_gft_entry *entries = list_files(&header, &n);
       
        Serial.print("[");

        for (int i = 0; i < n; i++)
        {
            Serial.print((char *)entries[i].filename);
            if (i < n - 1)
            {
                Serial.print(",");
            }
        }
      
        Serial.print("]");
        Serial.println();
        
        cmd_ok();
        free(entries);
        pid = 0;
    }

    // $read$FILENAME$
    // Return contents of file as hex string
    if (pid == 3)
    {

        int size = 0;

        struct fs_header header = read_fs_header();
        char *data = read_file(&header, filename_buffer, &size);
        if (data == -1)
        {
            cmd_fail();
        }
        else
        {
            Serial.print("[");
            hex_print(data, 128);
            Serial.println("]");
            cmd_ok();
            free(data);
        }
        free(filename_buffer);
        pid = 0;
    }

    // $format$
    // Formats filesystem
    if (pid == 2)
    {
        format_fs();
        cmd_ok();
        pid = 0;
    }

    // $encrypt$MODULUS$DATA$FILENAME$
    // Encrypts DATA using RSA-1024
    // and specified modulus. File is
    // saved on filesystem under FILENAME
    if (pid == 1)
    {
        int length = 0;
        pid = 0;
        length = strlen(data_buffer);
        // Seed RNG using floating pin
        number_rng_set_seed(analogRead(0));
        number_rsa_pkcs(data_buffer, length, modulus_buffer, output, 1024);
        struct fs_header header = read_fs_header();
        int result = write_file(&header, filename_buffer, output, 128);
        if (result == -1)
        {
            cmd_fail();
        }
        else
        {
            cmd_ok();
        }
        free(modulus_buffer);
        free(data_buffer);
        free(filename_buffer);
    }
}
