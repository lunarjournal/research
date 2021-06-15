// Author: Dylan Muller
// Student Number: MLLDYL002

#include "./lib/nativemsg.h"
#include <string.h>
#include "./lib/cJSON.h"
#include "./lib/serialib.h"
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

#define SERIAL_PORT "/dev/ttyACM0"
#define NOINLINE __attribute__((noinline))

// Probe serial port for data
int probe_serial(serialib *serial, char *buffer)
{
    // Implement 5 second timeout
    time_t start;
    time_t end;
    time_t seconds = 5;

    start = time(NULL);
    end = start + seconds;

    while (!serial->available() && (start < end))
    {
        start = time(NULL);
        usleep(1000);
    }
    serial->readString(buffer, 0, 400, 100);

    if (strstr(buffer, "OK"))
    {
        return 1;
    }
    if (strstr(buffer, "FAIL"))
    {
        return -1;
    }

    return 0;
}
// Format $list$ message
extern "C" NOINLINE int exec_list(serialib *serial, char *buffer)
{

    int count = 0;
    char local;
    serial->flushReceiver();
    serial->writeString("$list$");
    int ret = probe_serial(serial, buffer);
    return ret;
}

// Format $format$ message
extern "C" NOINLINE int exec_format(serialib *serial, char *buffer)
{

    int count = 0;
    char local;
    serial->flushReceiver();
    serial->writeString("$format$");
    int ret = probe_serial(serial, buffer);

    return ret;
}

// Format $delete$ message
extern "C" NOINLINE int exec_delete(serialib *serial, char *buffer, char *filename)
{
    int count = 0;
    char local;
    serial->flushReceiver();
    sprintf(buffer, "$delete$%s$", filename);
    serial->writeString(buffer);
    int ret = probe_serial(serial, buffer);

    return ret;
}

// Format $encrypt$ message
extern "C" NOINLINE int exec_encrypt(serialib *serial,
                 char *buffer,
                 char *modulus,
                 char *data,
                 char *filename)
{

    int count = 0;
    char local;
    serial->flushReceiver();
    sprintf(buffer, "$encrypt$%s$%s$%s$", modulus, data, filename);
    serial->writeString(buffer);
    int ret = probe_serial(serial, buffer);
    return ret;
}

// Format $read$ message
extern "C" NOINLINE int exec_read(serialib *serial, char *buffer, char *filename)
{
    int count = 0;
    char local;
    serial->flushReceiver();
    sprintf(buffer, "$read$%s$", filename);
    serial->writeString(buffer);
    int ret = probe_serial(serial, buffer);

    return ret;
}

// Extract data from response [DATA]
int extract_param(char *input, char *output)
{

    int i = 0;

    char *start = strchr(input, '[');
    char *end = strchr(input, ']');

    if (!start || !end)
    {
        return 0;
    }

    for (i = 0, start++; start != end; i++, start++)
    {
        output[i] = *start;
    }

    output[i] = 0;

    return 1;
}

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

// Convert hexadecimal string to byte array
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

int main()
{
    serialib serial;
    char buffer[400];
    char temp[400];
    uint32_t len;
    uint8_t *msg = NULL;
    cJSON *json = NULL;
    RSA *rsa = NULL;
    cJSON *cmd = NULL;
    BIGNUM *n = NULL;
    char *n_str = NULL;
    FILE * command = 0;
    int code = 0;
    

    sprintf(buffer, "%s/%s", getenv("HOME"), "key.pem");
    // Check for master key PEM
    // If it doesn't exist create key
    if (access(buffer, F_OK) != 0)
    {
        // Open file for write operation
        FILE *fp = fopen(buffer, "w+");
        // Generate 1024 bit key, e=3
        rsa = RSA_generate_key(1024, 3, 0, 0);
        // Write PEM
        PEM_write_RSAPrivateKey(fp, rsa, 0, 0, 0, 0, 0);
        fclose(fp);
    }
    // Wait 2 seconds
    sleep(2);
    // Can't write key, terminate
    if (access(buffer, F_OK) != 0)
    {
        exit(0);
    }

    FILE *fp = fopen(buffer, "rb");
    PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL);
    fclose(fp);

    // Can't write private key
    if (!rsa)
    {
        exit(0);
    }

    // Extract modulus from key
    n = RSA_get0_n(rsa);

    // failed to get modulus
    if (!n)
    {
        exit(0);
    }

    n_str = BN_bn2hex(n);

    while (1)
    {
        
        
        msg = nativemsg_read(&len);
        command = popen("ls /dev/ | grep ttyACM", "r");
        if(fgets(buffer,sizeof(buffer), command) ==NULL){
            exit(EXIT_FAILURE);
        }
        strtok(buffer,"\n");
        sprintf(temp, "/dev/%s", buffer);
        code = serial.openDevice(temp, 115200);
        if (code != 1)
        {
            exit(EXIT_FAILURE);
        }
        serial.flushReceiver();
        
        if (msg == NULL)
        {

            exit(EXIT_FAILURE);
        }

        // Assign a null terminator in the end to make it usable as a string
        msg[len] = '\0';

        json = cJSON_Parse(msg);
        cmd = cJSON_GetObjectItem(json, "cmd");

        if (cmd)
        {
            if (strcmp(cmd->valuestring, "list") == 0)
            {
                char *output = NULL;
                cJSON *object = NULL;
                cJSON *response = NULL;
                int length = NULL;
                int code = NULL;

                code = exec_list(&serial, buffer);

                object = cJSON_CreateObject();

                if (code == 1)
                {
                    response = cJSON_CreateString(buffer);
                }
                else
                {
                    response = cJSON_CreateString("FAIL");
                }
                cJSON_AddItemToObject(object, "msg", response);
                output = cJSON_Print(object);
                length = strlen(output);
                nativemsg_write(output, length);
                cJSON_Delete(object);
            }

            if (strcmp(cmd->valuestring, "format") == 0)
            {
                char *output = NULL;
                cJSON *object = NULL;
                cJSON *response = NULL;
                int length = NULL;
                int code = NULL;

                code = exec_format(&serial, buffer);

                object = cJSON_CreateObject();

                if (code == 1)
                {
                    response = cJSON_CreateString(buffer);
                }
                else
                {
                    response = cJSON_CreateString("FAIL");
                }
                cJSON_AddItemToObject(object, "msg", response);
                output = cJSON_Print(object);
                length = strlen(output);
                nativemsg_write(output, length);
                cJSON_Delete(object);
            }

            if (strcmp(cmd->valuestring, "encrypt") == 0)
            {
                char *output = NULL;
                cJSON *object = NULL;
                cJSON *response = NULL;
                cJSON *filename = NULL;
                cJSON *data = NULL;
                int length = NULL;
                int code = NULL;

                filename = cJSON_GetObjectItem(json, "filename");
                if (!filename)
                {
                    cJSON_Delete(cmd);
                    continue;
                }

                data = cJSON_GetObjectItem(json, "data");
                if (!data)
                {
                    cJSON_Delete(cmd);
                    continue;
                }

                code = exec_encrypt(&serial, buffer, n_str,
                                    data->valuestring,
                                    filename->valuestring);

                object = cJSON_CreateObject();

                if (code == 1)
                {
                    response = cJSON_CreateString(buffer);
                }
                else
                {
                    response = cJSON_CreateString("FAIL");
                }
                cJSON_AddItemToObject(object, "msg", response);
                output = cJSON_Print(object);
                length = strlen(output);
                nativemsg_write(output, length);
                cJSON_Delete(object);
            }

            if (strcmp(cmd->valuestring, "decrypt") == 0)
            {
                char *output = NULL;
                cJSON *object = NULL;
                cJSON *response = NULL;
                cJSON *filename = NULL;
                int length = NULL;
                int code = NULL;
                int i = 0;
                filename = cJSON_GetObjectItem(json, "filename");
                if (!filename)
                {
                    cJSON_Delete(cmd);
                    continue;
                }

                code = exec_read(&serial, buffer, filename->valuestring);
                response = extract_param(buffer, temp);
                
                if(!response)
                {
                    cJSON_Delete(cmd);
                    continue;
                }

                hex2bin(temp, buffer, 128);
                
                // Zero temp memory
                for(i = 0; i < 400; i++){
                    temp[i] = 0;
                }
                // Decrypt data using PKCS#1 padding, store into temp
                RSA_private_decrypt(128, buffer, temp, rsa, RSA_PKCS1_PADDING);
                object = cJSON_CreateObject();

                if (code == 1)
                {
                    response = cJSON_CreateString(temp);
                }
                else
                {
                    response = cJSON_CreateString("FAIL");
                }
                cJSON_AddItemToObject(object, "msg", response);
                output = cJSON_Print(object);
                length = strlen(output);
                nativemsg_write(output, length);
                cJSON_Delete(object);
            }

            if (strcmp(cmd->valuestring, "read") == 0)
            {
                char *output = NULL;
                cJSON *object = NULL;
                cJSON *response = NULL;
                cJSON *filename = NULL;
                int length = NULL;
                int code = NULL;

                filename = cJSON_GetObjectItem(json, "filename");
                if (!filename)
                {
                    cJSON_Delete(cmd);
                    continue;
                }

                code = exec_read(&serial, buffer, filename->valuestring);

                object = cJSON_CreateObject();

                if (code == 1)
                {
                    response = cJSON_CreateString(buffer);
                }
                else
                {
                    response = cJSON_CreateString("FAIL");
                }
                cJSON_AddItemToObject(object, "msg", response);
                output = cJSON_Print(object);
                length = strlen(output);
                nativemsg_write(output, length);
                cJSON_Delete(object);
            }

            if (strcmp(cmd->valuestring, "delete") == 0)
            {
                char *output = NULL;
                cJSON *object = NULL;
                cJSON *response = NULL;
                cJSON *filename = NULL;
                int length = NULL;
                int code = NULL;

                filename = cJSON_GetObjectItem(json, "filename");
                if (!filename)
                {
                    cJSON_Delete(cmd);
                    continue;
                }

                code = exec_delete(&serial, buffer, filename->valuestring);

                object = cJSON_CreateObject();

                if (code == 1)
                {
                    response = cJSON_CreateString(buffer);
                }
                else
                {
                    response = cJSON_CreateString("FAIL");
                }
                cJSON_AddItemToObject(object, "msg", response);
                output = cJSON_Print(object);
                length = strlen(output);
                nativemsg_write(output, length);
                cJSON_Delete(object);
            }
        }
        else
        {
            nativemsg_write("{\"msg\":\"FAIL\"}", 14);
        }

        cJSON_Delete(cmd);
        serial.closeDevice();
    }
    // Use `msg` for stuff here

    // free `msg` when done
    free(msg);
    return 0;
}