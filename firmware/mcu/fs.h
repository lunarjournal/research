// Author: Dylan Muller
// Student Number: MLLDYL002

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <EEPROM.h>

struct fs_header
{
    uint16_t magic;
    unsigned char gft_offset;
    unsigned char gft_count;
    uint16_t total_size;
    unsigned char node_size;
    unsigned char node_count;
} __attribute__((packed));

// GFT entry
struct fs_gft_entry
{
    unsigned char active;
    unsigned char filename[5];
    uint16_t file_size;
    uint16_t node_offset;
} __attribute__((packed));

// Node entry
struct fs_node
{
    unsigned char active;
    unsigned char node_size;
    uint16_t node_offset;
} __attribute__((packed));

struct fs_gft_entry *list_files(struct fs_header *header, int *n);
int write_file(struct fs_header *header, char *filename, char *data, int length);
char *read_file(struct fs_header *header, char *filename, int *n);
struct fs_header read_fs_header();
int format_fs();
int delete_file(struct fs_header *header, char *filename);
