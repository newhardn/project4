//
// stegoDestroy --- insert info in a BMP image file
//
// Created by nnewh on 11/9/2023.
//
// 4. Write a program, stegoDestroy.c that will destroy any information hidden in a file, assuming that the information
// hiding method in stego.c might have been used. Your program should take a bmp file as input, and produce a bmp file
// as output. Visually, the output file must be identical to the input file.
//
// 5. Test stegoDestroy.c on aliceStego.bmp. Verify that the output file image is undamaged. What information does
// stego.c extract from your output file?
//
#include "stego.h"

// destroyStego
//
// Read input file containing stego and exclude stego data while writing a new "clean" output file.
//
// FILE* fin    file pointer to open readable input data file, possibly containing stego
// FILE* fout   file pointer to open writable output data file
//
int destroyStego(FILE* fin, FILE* fout) {

    int i,
        j,
        x;

    char temp;



}

// main program
int main(int argc, const char *argv[]) {

    char fnameIn[80],
         fnameOut[80];

    FILE *fin, *fout;

    int bytesWritten = 0;

    // validate number of arguments
    if(argc != 2)
    {
oops:   fprintf(stderr, "\nUsage: %s stegoImage\n\n", argv[0]);
        fprintf(stderr, "Where stegoImage == filename for image containing stego data to be removed\n\n");
        exit(0);
    }

    // grab input and output filenames
    sprintf(fnameIn, argv[1]);
    sprintf(fnameOut, argv[2]);

    // open the input data file
    fin = fopen(fnameIn, "r");
    if(fin == NULL)
    {
        fprintf(stderr, "\nError opening file %s\n\n", fnameIn);
        exit(0);
    }

    // open the output file
    fout = fopen(fnameOut, "w");
    if(fout == NULL)
    {
        fprintf(stderr, "\nError opening file %s\n\n", fnameOut);
        exit(0);
    }

    // destroy stego data by reading the input file and writing a clean output file; returns data written size in bytes
    bytesWritten = destroyStego(fin, fout);

    // print status
    printf("\ndata bytes written = %d\n\n", bytesWritten);
    printf("\n");

    // close files and exit
    fclose(fin);
    fclose(fout);
}