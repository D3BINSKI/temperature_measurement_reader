#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include "bitops.h"
#include "com.h"


void usage(char* source)
{
    fprintf(stderr, "USAGE: %s file", source);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    struct args frame;
    int n[2];
    if(argc != 2) usage(argv[0]);

    //-----------------DATA GENERATOR----------------
    //If the file already exists comment this section

    n[0] = 3; n[1] = 1;
    generate_bin(argv[1], n , 2);

    //-----------------------------------------------

    data_extract(argv[1]);
    return 0;
}










