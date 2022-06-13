//
// Created by debin on 27.05.2022.
//

#ifndef INC_04_BINARY_READER_COM_H
#define INC_04_BINARY_READER_COM_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "bitops.h"
#include "generator.h"

struct data
{
    uint32_t* id;
    double* output;
    uint8_t n;
};

union ui32 {
    uint32_t v;
    uint8_t octets[4];
};

union ui16 {
    uint16_t v;
    uint8_t octets[2];
};

union i16 {
    int16_t v;
    int8_t octets[2];
};

struct args
{
    union ui32 header;
    uint8_t n;
    struct pair* data;
    union ui16 crc;
};

struct pair{
    union ui32 id;
    union i16 data;
};

uint16_t extract_crc(uint8_t *data, uint32_t size);
void crc16_ccitt_false(char* pData, unsigned int length, uint16_t* wCrc);
uint16_t evaluate_crc16_crtd(struct args* frame);
uint16_t evaluate_crc16_rcvd(uint8_t* data, uint32_t size);
void prepare_frame(struct args* frame, int n);
int write_bin_data(int wfd, struct args* frame);

int read_header(int rfd, uint32_t* buf);
int get_payload(int rfd, uint8_t* payloadData, uint32_t size);
uint8_t extract_n(uint8_t *data);
void data_init(struct data *pData, uint8_t n);
int8_t data_extract(char* file);

void display(struct data* data, int n);

double positive_temp_decode(int16_t value);
double negative_temp_decode(int16_t value);
double temp_decode(int16_t value);

int16_t positive_temp_code(double value);
int16_t negative_temp_code(double value);
int16_t temp_code(double value);

void generate_temperature_data(struct pair* tab, int size);
unsigned long mix(unsigned long a, unsigned long b, unsigned long c);
void generate_bin(char* file, int* n, int nframes);

#endif //INC_04_BINARY_READER_COM_H
