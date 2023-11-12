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

// struct for managing a file and its name
typedef struct {
    FILE *fp;
    char fname[80];
} fileAtom;

// FileAtomOpen
//
FILE * FileAtomOpen(const char *restrict filename, const char *restrict mode, fileAtom *fa) {
    if (fa == NULL) {
        fprintf(stderr, "\nFatal error: invalid parameters passed to FileAtomOpen\n\n");
        exit(0);
    }

    memset(fa,0,sizeof(fileAtom));
    strcpy(fa->fname,filename);

    // open the file specified by filename, retrieving the file pointer
    FILE *fp = fopen(fa->fname, mode);

    // looks good, fill the atom
    if (fp != NULL) {
        fa->fp = fp;
        sprintf(fa->fname, filename);
    }

    return fp;
}

// FileAtomClose
//
void FileAtomClose(fileAtom *fa) {
    if (fa == NULL) {
        fprintf(stderr, "\nFatal error: invalid parameters passed to FileAtomClose\n\n");
        exit(0);
    }

    // close the file
    fclose(fa->fp);

    // clear the atom
    memset(fa, 0, sizeof(fileAtom));
}


// destroyStego
//
// Read input file containing stego and exclude stego data while writing a new "clean" output file.
//
// fileAtom* faIn    fileAtom struct pointer to open readable input data file, possibly containing stego
// fileAtom* faOut   fileAtom struct pointer to open writable output data file
//
int destroyStego(fileAtom * faIn, fileAtom * faOut) {

    if (faIn == NULL || faOut == NULL) {
        fprintf(stderr, "\nFatal error: invalid fileAtom pointer(s) passed to destroyStego\n\n");
        exit(0);
    }

    int i,
        j,
        x,
        ttt,
        shft,
        byteCount,
        moreData,
        moreImage,
        imageBytes,
        dataBytes,
        imageBytesWritten,
        dataBytesWritten;

    char temp,
         data;

    //
    // Skip first START_FROM bytes of image file
    // Simultaneously writing skipped data to output file
    //
    imageBytesWritten = 0;
    for(i = 0; i < START_FROM; ++i)
    {
        x = fscanf(faIn->fp, "%c", &temp);
        if (x != 1)
        {
            fprintf(stderr, "\nError in file %s\n\n", faIn->fname);
            exit(0);
        }
        fprintf(faOut->fp, "%c", temp);
        ++imageBytesWritten;
    }

    char stego[64];
    // loop to check for stego marker
    for (i = 0; i < 8; ++i) {
        ttt = 0x0;
        for (j = 0; j < 8; ++j) {
            x = fscanf(faIn->fp, "%c", &temp);
            if (x != 1) {
                fprintf(stderr, "\nError in file %s\n\n", faIn->fname);
                exit(0);
            }
            ttt ^= ((temp & 0x1) << j);
            stego[i * 8 + j] = temp; // store the original stego-altered data in an array
        }
    }

    // if not 0xa5, then file does not contain stego data
    if(ttt != 0xa5) {
        fprintf(stderr, "\nError --- file does not contain stego data that I can read\n\n");
        exit(0);
    }

    // second pass to restore the original LSB values
    for (i = 0; i < 8; ++i) {
        ttt = 0x0;
        for (j = 0; j < 8; ++j) {
            temp = stego[i * 8 + j]; // Retrieve byte from the array
            ttt ^= ((temp & 0x1) << j);

            // Restore the original LSB of temp by XOR-ing it with the stego bit
            temp ^= ((ttt >> i) & 0x1);

            // Write the restored byte to the output file
            fprintf(faOut->fp, "%c", temp);
            ++imageBytesWritten;
        }
    }

    // Strip out the databytes value (27 bits across 27 bytes)
    dataBytes = 0;
    for(i = 0; i < 27; ++i)
    {
        x = fscanf(faIn->fp, "%c", &temp);
        if (x != 1)
        {
            fprintf(stderr, "\nError in file %s\n\n", faIn->fname);
            exit(0);
        }

        // update dataBytes with stego bit
        dataBytes ^= ((temp & 0x1) << i);

        // remove stego bit by XOR-ing it again
        temp ^= ((dataBytes >> i) & 0x1);

        // write the restored byte to output file
        fprintf(faOut->fp, "%c", temp);
        ++imageBytesWritten;
    }
    printf("dataBytes detected = %d\n", dataBytes);

    for (i = 0; i < dataBytes; ++i) {
        char bitmapData[8];

        // Read 8 bits of bytewise stego data into bitmapData
        for (j = 0; j < 8; ++j) {
            x = fscanf(faIn->fp, "%c", &bitmapData[j]);
            if (x != 1) {
                fprintf(stderr, "\nError in file %s\n\n", faIn->fname);
                exit(0);
            }
        }

        // assemble the 8 bits into a stego data byte
        char stegoByte = 0;
        for (j = 0; j < 8; ++j) {
            stegoByte |= ((bitmapData[j] & 0x1) << j);
        }

        // loop to remove the stego by applying the shifted bits from the stego data byte
        for (j = 0; j < 8; ++j) {
            // remove the stego bit by XOR-ing it again
            temp = bitmapData[j] ^ ((stegoByte >> j) & 0x1);

            // write the corrected byte of the bitmap to the output file
            fprintf(faOut->fp, "%c", temp);
            ++imageBytesWritten;
        }
    }

    // read and write remaining data from the input file to the output file
    while (fscanf(faIn->fp, "%c", &temp) == 1) {
        fprintf(faOut->fp, "%c", temp);
        ++imageBytesWritten;
    }

    return imageBytesWritten;
}


// main program
int main(int argc, const char *argv[]) {

    char fnameIn[80],
         fnameOut[80];

    FILE *fin, *fout;

    int bytesWritten = 0;

    // validate number of arguments
    if(argc != 3)
    {
oops:   fprintf(stderr, "\nUsage: %s stegoImage outImage\n\n", argv[0]);
        fprintf(stderr, "Where stegoImage == filename for image containing stego data to be removed\n\n");
        fprintf(stderr, "      outImage == data restored from stegoImage file\n\n");
        exit(0);
    }

    // grab input and output filenames
    sprintf(fnameIn, argv[1]);
    sprintf(fnameOut, argv[2]);

    // use fileAtoms for managing file data
    fileAtom fileAtomIn, fileAtomOut;

    // open the input data file
    fin = FileAtomOpen(fnameIn, "r",&fileAtomIn);
    if(fin == NULL)
    {
        fprintf(stderr, "\nError opening file %s\n\n", fnameIn);
        exit(0);
    }

    // open the output file
    fout = FileAtomOpen(fnameOut, "w", &fileAtomOut);
    if(fout == NULL)
    {
        fprintf(stderr, "\nError opening file %s\n\n", fnameOut);
        exit(0);
    }

    // destroy stego data by reading the input file and writing a clean output file; returns data written size in bytes
    int imageBytesWritten = destroyStego(&fileAtomIn, &fileAtomOut);

    // print status
    printf("\nimage bytes written = %d\n\n", imageBytesWritten);
    printf("\n");

    // close files and exit
    FileAtomClose(&fileAtomIn);
    FileAtomClose(&fileAtomOut);
}