// Author: Dylan Muller
// Student Number: MLLDYL002

#include "fs.h"
#include<Arduino.h>
// File system header struct


// Read byte from EEPROM
unsigned char read_eeprom(int address)
{
    return EEPROM.read(address);
}

// Write byte to EEPROM
char write_eeprom(int address, unsigned char value)
{
    EEPROM.write(address, value);
}

// Write 16 bit value to EEPROM
void write_uint16_eeprom(int address, uint16_t value)
{
    uint16_t mask = 0;
    uint16_t temp = 0;
    int i = 0;
    mask = 0xFF;
    for (i = 0; i < sizeof(uint16_t); i++)
    {
        temp = ((value & (mask << (8 * i))) >> 8 * i) & 0xFF;
        write_eeprom(address + i, temp);
    }
}

// Read 16 bit value from EEPROM
uint16_t read_uint16_eeprom(int address)
{
    uint16_t local = 0;
    uint16_t value = 0;
    int i = 0;

    for (i = 0; i < sizeof(uint16_t); i++)
    {
        value = (read_eeprom(address + i) << 8 * i) & (0xFF << 8 * i);
        local = local | value;
    }
    return local;
}

// Write filesystem header to EEPROM
int write_fs_header(struct fs_header *header)
{
    int total_size = 0;
    ;
    int total_node_size = 0;

    total_size = header->gft_offset;
    total_size += sizeof(struct fs_gft_entry) * header->gft_count;
    total_node_size = sizeof(struct fs_node) + header->node_size;
    total_size += (total_node_size)*header->node_count;

    if (total_size > header->total_size)
    {
        return -1;
    }

    // if it can write header to disk
    write_uint16_eeprom(0, header->magic);
    write_eeprom(2, header->gft_offset);        // gft_offset
    write_eeprom(3, header->gft_count);         // gft_count
    write_uint16_eeprom(4, header->total_size); // total_size
    write_eeprom(6, header->node_size);         // node_size
    write_eeprom(7, header->node_count);        // node_count

    return 0;
}

// Read filesystem header from EEPROM
struct fs_header read_fs_header()
{
    struct fs_header header;
    header.magic = read_uint16_eeprom(0);
    header.gft_offset = read_eeprom(2);
    header.gft_count = read_eeprom(3);
    header.total_size = read_uint16_eeprom(4);
    header.node_size = read_eeprom(6);
    header.node_count = read_eeprom(7);
    return header;
}

// Format file system
int format_fs()
{

    // Define header constants
    struct fs_header header;
    header.magic = 0xABCD;
    header.gft_offset = sizeof(struct fs_header);
    header.gft_count = 7;
    header.total_size = 1024;
    header.node_size = 128;
    header.node_count = 7;

    // Zero EEPROM
    for (int i = 0; i < 1024; i++)
    {
        EEPROM.write(i, 0);
    }
    return write_fs_header(&header);
}

// Find empty GFT entry
int find_empty_gft(struct fs_header *header)
{
    int gft_offset = 0;
    int i = 0;
    int gft_size = 0;
    int active = 0;
    int index = 0;

    gft_size = sizeof(struct fs_gft_entry);
    gft_offset = header->gft_offset;

    for (i = 0; i < header->gft_count; i++)
    {
        index = gft_offset + (i * gft_size);
        active = read_eeprom(index);

        if (active == 0x00)
        {
            return index;
        }
    }
    return -1;
}

// Find empty node entry
int find_empty_node(struct fs_header *header)
{
    int gft_offset = 0;
    int gft_size = 0;
    int node_size = 0;
    int i = 0;
    int active = 0;
    int index = 0;

    gft_offset = header->gft_offset;
    gft_size = sizeof(struct fs_gft_entry);
    gft_offset += gft_size * header->gft_count;
    node_size = sizeof(struct fs_node) + header->node_size;

    for (i = 0; i < header->node_count; i++)
    {
        index = gft_offset + (i * node_size);
        active = read_eeprom(index);

        if (active == 0x00)
        {
            return index;
        }
    }
    return -1;
}

// Release filesystem node
void release_fs_node(struct fs_header *header, int address)
{
    uint16_t addr = 0;
    int nodes = 0;

    addr = address;
    do
    {
        write_eeprom(addr, 0);
        addr = read_uint16_eeprom(addr + 2);
        nodes++;
    } while (addr != 0xffff && nodes < header->node_count);
}

// Write data to filesystem node
int write_fs_node(struct fs_header *header, char *data, int size)
{
    double ratio = 0;
    int nodes_required = 0;
    int i = 0, j = 0;
    int buffer_count = 0;
    int node_addr = 0;
    int next_addr = 0;
    int first_addr = 0;
    int length = 0;

    ratio = (double)size / (double)header->node_size;
    nodes_required = (int)ceil(ratio);

    if (size > (header->node_size * header->node_count))
    {
        return -1;
    }
    if (size > header->total_size)
    {
        return -1;
    }
    for (i = 0; i < nodes_required; i++)
    {

        if (i == 0)
        {
            node_addr = find_empty_node(header);
            if (node_addr == -1)
            {
                //Serial.println("could not allocate node!\n");
                return -1;
            }
            first_addr = node_addr;
            //Serial.print("found first node at ");
            //Serial.println(node_addr, DEC);
        }
        write_eeprom(node_addr, 1);
        length = size - (i * header->node_size);

        if (length > header->node_size)
        {
            write_eeprom(node_addr + 1, header->node_size);
            //Serial.print("node size : ");
            //Serial.println(header->node_size);
        }
        else
        {
            write_eeprom(node_addr + 1, length);
            //Serial.print("node size : ");
            //Serial.println(length);
        }
        if (length > header->node_size)
        {
            next_addr = find_empty_node(header);
            if (next_addr == -1)
            {
                //Serial.println("could not allocate node!\n");
                return -1;
            }
            //Serial.print("found next node at : ");
            //Serial.println( next_addr);
            write_uint16_eeprom(node_addr + 2, next_addr);

            node_addr = next_addr;
        }
        else
        {

            write_uint16_eeprom(node_addr + 2, 0xFFFF);
        }

        if (i == 0)
        {
            node_addr = first_addr;
            for (j = 0; j < header->node_size && buffer_count < size; j++)
            {
                write_eeprom(node_addr + 4 + j, *(data + buffer_count));
                buffer_count += 1;
            }
        }
        node_addr = next_addr;
        for (j = 0; j < header->node_size && buffer_count < size; j++)
        {
            write_eeprom(node_addr + 4 + j, *(data + buffer_count));
            buffer_count += 1;
        }
    }

    return first_addr;
}

// Create GFT entry for file with filename and data
int create_gft_entry(struct fs_header *header, char *filename, char *data, int length)
{
    int gft_addr = 0;
    int node_addr = 0;
    int str_len = 0;
    int file_size = 0;
    int i = 0;

    // Get empty GFT entry
    gft_addr = find_empty_gft(header);

    if (gft_addr == -1)
    {
        //Serial.println("could not find empty gft entry!");
        return -1;
    }

    write_eeprom(gft_addr, 0);
    str_len = strlen(filename) + 1;
    if (str_len > 5)
    {
        //Serial.println("filename too large!");
        return -1;
    }
    // Copy filename
    for (i = 0; i < str_len && i < 5; i++)
    {
        write_eeprom(gft_addr + 1 + i, *(filename + i));
    }
    write_eeprom(gft_addr + 5, 0);
    file_size = length;
    write_uint16_eeprom(gft_addr + 6, file_size);
    node_addr = write_fs_node(header, data, file_size);
    if (node_addr == -1)
    {

        //Serial.println("could not create node entry!");
        return -1;
    }
    write_uint16_eeprom(gft_addr + 8, node_addr);
    write_eeprom(gft_addr, 1);
    return node_addr;
}

// Get GFT entry by index
struct fs_gft_entry get_gft_entry(struct fs_header *header, int index)
{
    struct fs_gft_entry gft_entry;
    int addr = 0;

    addr = header->gft_offset;
    addr += sizeof(struct fs_gft_entry) * index;
    gft_entry.active = read_eeprom(addr);

    for (int i = 0; i < 5; i++)
    {
        gft_entry.filename[i] = read_eeprom(addr + 1 + i);
    }
    gft_entry.file_size = read_uint16_eeprom(addr + 6);
    gft_entry.node_offset = read_uint16_eeprom(addr + 8);

    return gft_entry;
}

// Get GFT entry by filename
struct fs_gft_entry find_gft_entry(struct fs_header *header, char *filename)
{
    int i = 0;
    struct fs_gft_entry entry;
    entry.active = 0;
    for (i = 0; i < header->gft_count; i++)
    {
        entry = get_gft_entry(header, i);
        if (strcmp(entry.filename, filename) == 0)
        {
            return entry;
        }
    }

    return entry;
}

// List all files in filesystem
struct fs_gft_entry *list_files(struct fs_header *header, int *n)
{
    struct fs_gft_entry entry;
    struct fs_gft_entry *entry_ptr;
    int i = 0;
    int j = 0;
    int file_count = 0;
    int size = 0;

    for (i = 0; i < header->gft_count; i++)
    {
        entry = get_gft_entry(header, i);

        if (entry.active)
        {
            file_count++;
        }
    }

    size = sizeof(struct fs_gft_entry) * file_count;
    file_count = 0;
    entry_ptr = (struct fs_gft_entry *)malloc(size);

    for (i = 0; i < header->gft_count; i++)
    {
        entry = get_gft_entry(header, i);

        if (entry.active)
        {

            entry_ptr[file_count].active = entry.active;
            for (j = 0; j < 5; j++)
            {
                entry_ptr[file_count].filename[j] = entry.filename[j];
            }
            entry_ptr[file_count].file_size = entry.file_size;
            entry_ptr[file_count].node_offset = entry.node_offset;
            file_count++;
        }
    }
    *n = file_count;
    return entry_ptr;
}

// Remove a file from filesystem
int delete_file(struct fs_header *header, char *filename)
{
    struct fs_gft_entry entry = find_gft_entry(header, filename);
    struct fs_gft_entry entry2;
    int i = 0;
    int match = 0;
    int offset = 0;

    if (entry.active)
    {
        for (i = 0; i < header->gft_count; i++)
        {
            entry2 = get_gft_entry(header, i);
            if (strcmp(entry2.filename, filename) == 0 &&
                entry2.active == 1)
            {
                match = 1;
                break;
            }
        }

        if (match)
        {
            int size = sizeof(struct fs_gft_entry) * i;
            offset = header->gft_offset + size;
            write_eeprom(offset, 0);
            //Serial.print("releasing node ");
            //Serial.println(entry.node_offset);
            release_fs_node(header, entry.node_offset);
            return 0;
        }

        else
        {
            //Serial.println("failed to delete file!");
            return -1;
        }
    }
    else{
      return -1;
    }
}

// Write data to file
int write_file(struct fs_header *header, char *filename, char *data, int length)
{
    int node = 0;

    delete_file(header, filename);
    node = create_gft_entry(header, filename, data, length);

    if (node == -1)
    {
        //Serial.println("error writing to file");
        return -1;
    }
    return node;
}

// Read data from file
char *read_file(struct fs_header *header, char *filename, int *n)
{
    struct fs_gft_entry entry;
    int i = 0;
    char *buffer = 0;
    uint16_t addr = 0;
    int size = 0;
    int count = 0;
    int nodes = 0;

    entry = find_gft_entry(header, filename);
    if (entry.active == 0)
    {
        //Serial.println("no such file exists!");
        //Serial.println("no such file exists");
        return -1;
    }

    buffer = (char *)malloc(entry.file_size);
    addr = entry.node_offset;
    *n = entry.file_size;

    do
    {
        size = read_eeprom(addr + 1);

        for (i = 0; i < size; i++)
        {
            buffer[count] = read_eeprom(addr + 4 + i);
            count++;
        }

        addr = read_uint16_eeprom(addr + 2);
        nodes++;

    } while (addr != 0xffff && (nodes < header->node_count));

    return buffer;
}
