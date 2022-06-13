//
// Created by debin on 27.05.2022.
//

#include "com.h"

#define DEGREE_S 0xF8
#define RESOLUTION 0.015625
//#define format 0x3fff
#define MAXTEMP 100
#define MINTEMP -50
#define TEMPDELTA 1
#define MAXID 3200


int8_t data_extract(char* file) {
    struct data *pData;
    int rfd;
    if((rfd = open(file, O_RDONLY | O_BINARY)) < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    pData = (struct data*)malloc(sizeof(struct data));
    struct stat st;
    stat(file, &st);
    _off_t size = st.st_size;
    uint32_t* buf = (uint32_t*)malloc(sizeof(uint32_t));
    while(size > 0)
    {
        if(read_header(rfd, buf)==-1)
        {
            return -1;
        }
        uint32_t payloadSize = right_justifyu32(*buf);
        size -= (payloadSize + sizeof(uint32_t));
        fprintf(stdout, "payload size: %d\n", payloadSize);
        uint8_t* payloadData = (uint8_t*)malloc(payloadSize*sizeof(uint8_t));
        get_payload(rfd, payloadData, payloadSize);

        uint16_t crc = evaluate_crc16_rcvd(payloadData, payloadSize);
        if(crc != extract_crc(payloadData, payloadSize))
        {
            perror("get_payload");
            exit(EXIT_FAILURE);
        }
        else fprintf(stdout, "packet is ok\n");

        data_init(pData, extract_n(payloadData));
        uint16_t temp;
        int position = sizeof(uint8_t);
        for (int i = 0; i < pData->n; ++i) {
            pData->id[i] = 0x00000000;
            for (int j = 0; j < sizeof(uint32_t); ++j) {
                pData->id[i] <<= 8;
                pData->id[i] |= *(payloadData + position);
                position++;
            }
            temp = 0x0000;
            for (int j = 0; j < sizeof(int16_t); ++j) {
                temp <<= 8;
                temp |= *(payloadData + position);
                position++;
            }
            pData->output[i] = temp_decode((int16_t)temp);
        }
        display(pData, pData->n);
    }
    close(rfd);
    free(pData);
    free(buf);
    return 0;
}

void data_init(struct data *pData, uint8_t n) {
    pData->id = (uint32_t*)malloc(n*sizeof(uint32_t));
    pData->output = (double*)malloc(n* sizeof(double));
    pData->n = n;
}

uint8_t extract_n(uint8_t *data) {
    return *data;
}

uint16_t extract_crc(uint8_t *data, uint32_t size) {
    uint16_t crc = 0x0000;
    for (int i = 0; i < sizeof(uint16_t); ++i) {
        crc <<= 8;
        crc |= *(data+(size- sizeof(uint16_t)) + i);
    }
    return crc;
}

uint16_t evaluate_crc16_rcvd(uint8_t* data, uint32_t size)
{
    uint16_t wCrc = 0xffff;
    unsigned int len;
    crc16_ccitt_false(data, size-2, &wCrc);
    return wCrc;
}

uint16_t evaluate_crc16_crtd(struct args* frame)
{
    uint16_t wCrc = 0xffff;
    crc16_ccitt_false(&(frame->n), 1, &wCrc);

    for (int i = 0; i < frame->n; ++i) {
        for (int j = 3; j >= 0; --j) {
            crc16_ccitt_false(&((frame->data)[i].id.octets[j]), 1, &wCrc);
        }

        for (int j = 1; j >= 0; --j) {
            crc16_ccitt_false(&(frame->data[i].data.octets[j]), 1, &wCrc);
        }
    }
    return wCrc;
}

int read_header(int rfd, uint32_t* buf)
{
    if(read(rfd, buf, sizeof(uint32_t)) == -1)
    {
        return -1;
    }
    return 0;
}

int get_payload(int rfd, uint8_t* payloadData, uint32_t size) {
    int ret;
    memset(payloadData, 0, size);
    for (int i = 0; i < size; ++i) {
        if((ret = read(rfd, payloadData, sizeof(uint8_t))) == -1 )
        {
            return -1;
        }
        payloadData++;
    }
}

void crc16_ccitt_false(char* pData, unsigned int length, uint16_t* wCrc)
{
    int i;
    while (length--)
    {
        *wCrc ^= (*(unsigned char *)pData++ << 8);
        for (i=0; i < 8; i++)
            *wCrc = *wCrc & 0x8000 ? (*wCrc << 1) ^ 0x1021 : *wCrc << 1;
    }
}

void prepare_frame(struct args* frame, int n)
{
    struct pair* pairs;
    pairs = (struct pair*)malloc(sizeof(struct pair)*n);
    generate_temperature_data(pairs, n);
    frame->data = pairs;
    frame->n = n;
    frame->header.v = left_justifyu32(sizeof(uint8_t) + frame->n*(sizeof(uint32_t)+sizeof(uint16_t)) + sizeof(uint16_t));
    frame->crc.v = evaluate_crc16_crtd(frame);
}

int write_bin_data(int wfd, struct args* frame)
{
    uint16_t crc;
    for (int i = sizeof(uint32_t)-1; i >= 0; --i) {
        if(write(wfd, &(frame->header.octets[i]), sizeof(uint8_t))<=0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
    if(write(wfd, &(frame->n), sizeof(uint8_t))<=0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i <frame->n; ++i) {
        for (int j = sizeof(uint32_t) -1; j >=0 ; --j) {
            if(write(wfd, &(frame->data[i].id.octets[j]), sizeof(uint8_t)) <= 0)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
        for (int j = sizeof(int16_t) -1; j >=0 ; --j) {
            if(write(wfd, &(frame->data[i].data.octets[j]), sizeof(int8_t)) <= 0)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
    }
    for (int i = sizeof(uint16_t)-1; i >= 0; --i) {
        if(write(wfd, &(frame->crc.octets[i]), sizeof(uint8_t))<=0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
}

void display(struct data* data, int n)
{
    fprintf(stdout, "\n--------------RECIVED DATA--------------\n");
    for (int i = 0; i < n; ++i) {
        if(data->output[i] >= -40.0 && data->output[i] < 80.0) fprintf(stdout, "[Sensor %d]:\t%.4f%c%c\n", data->id[i], data->output[i], DEGREE_S, 'C');
        else fprintf(stdout, "WARNING:\tSensor %d measured %f%c%c\n", data->id[i], data->output[i], DEGREE_S, 'C');
    }
}

double positive_temp_decode(int16_t value)
{
    uint8_t MSB = value >> 8;
    uint8_t LSB = value;
    value = (int16_t)(((MSB << 8) | LSB) >> 2);
    return (double)value*RESOLUTION;
}

double negative_temp_decode(int16_t value)
{
    value = (int16_t)(~((int16_t)value)+1);
//    uint8_t MSB = value >> 8;
//    value &= format;
    double result = (double)value*RESOLUTION;

    return -result;
}

double temp_decode(int16_t value)
{

    if((value & 0x2000) != 0){
        return negative_temp_decode(value);
    }
    else{
        return positive_temp_decode(value);
    }
}

int16_t positive_temp_code(double value)
{
    value /= RESOLUTION;
    return (int16_t)((int16_t)value/* & format*/);
}

int16_t negative_temp_code(double value)
{
    value = -value/RESOLUTION;
    int16_t result = (int16_t)((int16_t)(value) /*& format*/);
    result = (int16_t)(~(result)+1);
    return (int16_t)(result);
}

int16_t temp_code(double value)
{
    if(value < 0)
        return negative_temp_code(value);
    else
        return lfj14(positive_temp_code(value));
}

unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}

void generate_temperature_data(struct pair* tab, int size) {
    double temp;
    unsigned long seed = mix(clock(), time(NULL), getpid());
    srand(seed);
    int16_t base = (int16_t) ((rand() % (MAXTEMP - MINTEMP + 1) + MINTEMP));
    for (int i = 0; i < size; ++i) {
        tab[i].id.v = (uint32_t) (rand() % MAXID);
        tab[i].data.v = temp_code((double)(rand()%MAXTEMP) + MINTEMP - (double) (rand() % 101) / 100.0);
        fprintf(stdout, "(id, measurement) = (%d, %f)\n", tab[i].id.v, temp_decode(tab[i].data.v));
    }
}

void generate_bin(char* file, int* n, int nframes)
{
    struct args frame;
    int wfd;
    if((wfd = open(file, O_WRONLY | O_CREAT | O_BINARY, 0666)) < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < nframes; ++i) {
        prepare_frame(&frame, n[i]);
        write_bin_data(wfd, &frame);
    }
    close(wfd);
}
