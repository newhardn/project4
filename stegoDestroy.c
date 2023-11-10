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
        dataBytesWritten,
        imageBytesWritten;

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

    // creating marker and source arrays
    // todo: consider writing out the destination file even if it does not contain stego
    char marker[8], source[8];

    // read 8 bytes (64 bits) of the file -- this is only a potential marker
    x = fread(marker, sizeof(char), 8, faIn->fp);
    if (x != 8) {
        fprintf(stderr, "\nError in file %s\n\n", faIn->fname);
        FileAtomClose(faIn);
        FileAtomClose(faOut);
        exit(1);
    }

    // copy marker to source, intending to replace source IF data contains a stego marker
    memcpy(source, marker, 8);

    //todo: this is where it currently fails to find the marker
    // process the data and look for stego marker
    for (i = 0; i < 8; ++i) {
        ttt = 0x0;

        temp = marker[i]; // grab byte from marker[i]
        for (j = 0; j < 8; ++j) {
            ttt ^= ((temp & 0x1) << j);
        }

        // remove stego marker indicators
        source[i] = (temp & 0xfe) ^ ((~(ttt ^ 0xa5) & 0x1));
    }

    // if not of the form 0xa5, then file does not contain stego data
    if(ttt != 0xa5) {
        fprintf(stderr, "\nError --- file does not contain stego data that I can read\n\n");
        exit(0);
    }

    // Write the modified source data to the output file
    fwrite(source, sizeof(char), 8, faOut->fp);

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
    }
    printf("dataBytes detected = %d\n", dataBytes);

    // read dataBytes characters, restoring their original value by removing stego data, then writing to output file
    data = 0;
    shft = 0;
    dataBytesWritten = 0;
    for(i = 0; i < (dataBytes << 3); ++i)
    {
        x = fscanf(faIn->fp, "%c", &temp);
        if (x != 1)
        {
            fprintf(stderr, "\nError in file %s\n\n", faIn->fname);
            exit(0);
        }

//        printf("bit %d = %d\n", shft, temp & 0x1);

        data = data ^ ((temp & 0x1) << shft);
        ++shft;
        if (shft == 8)
        {
//            printf("data = %c\n", data);

            fprintf(faOut->fp, "%c", data);
            ++dataBytesWritten;
            data = 0;
            shft = 0;

        }// end if

    }// next i

    return dataBytesWritten;
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
    bytesWritten = destroyStego(&fileAtomIn, &fileAtomOut);

    // print status
    printf("\ndata bytes written = %d\n\n", bytesWritten);
    printf("\n");

    // close files and exit
    FileAtomClose(&fileAtomIn);
    FileAtomClose(&fileAtomOut);
}