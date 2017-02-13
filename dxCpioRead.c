/*
 dxCpioRead - 0.5 - public domain
 no warrenty implied; use at your own risk.
 authored from 2017 by Dmitry Hrabrov a.k.a. DeXPeriX
 http://dexperix.net

LICENSE:
    This software is dual-licensed to the public domain and under the following
    license: you are granted a perpetual, irrevocable license to copy, modify,
    publish and distribute this file as you see fit.
*/
const char* dxCpioRead(const void* cpioData, const long cpioSize,
                      const char* fileName, long* fileSize)
{
    const int BLOCK_SIZE = 26, NAME_LEN_OFFSET = 20, SIZE_OFFSET = 22;
    const unsigned char MAGIC[] = {0xC7, 0x71}; /* =070707oct, little endian */
    const unsigned char* u = (const unsigned char*) cpioData;
    long p = 0, found = 0, newOffset = 0, nameLen = 0, fsize = 0;

    *fileSize = 0; /* will be zero if CPIO wrong or there is no such file */
    do { /* "Load" data from CPIO - just point to passed memory*/
        const unsigned char* name = u + BLOCK_SIZE + p + newOffset;
        long b = 1, s = 0, i; /* big and small byte offset */
        p += newOffset; /* pointer to current file's data in CPIO */

        if( !( (u[p+0] == MAGIC[0]) && (u[p+1] == MAGIC[1]) ) ){
            if((u[p+1] == MAGIC[0]) && (u[p+0] == MAGIC[0])){
                b = 0; s = 1; /* big endian */
            } else return 0; /* = return NULL; magic constant not found */
        }

        nameLen = (u[p+NAME_LEN_OFFSET+b] << 8) | (u[p+NAME_LEN_OFFSET+s]);
        i = p + SIZE_OFFSET;
        fsize = (u[i+b] << 24) | (u[i+s] << 16) | (u[i+2+b] << 8) | (u[i+2+s]);

        /* Offset size in bytes. Depends on file size and CPIO's block size */
        if( (nameLen % 2) != 0 ) nameLen++; /* odd name = +1 NULL byte */
        newOffset = nameLen + fsize + BLOCK_SIZE;
        if( (fsize % 2) != 0 ) newOffset++; /* odd size = +1 NULL byte */

        i = 0; /* strncmp - compare file's name with that a user wants */
        while((i<nameLen) && (fileName[i]!=0) && (name[i]==fileName[i])) i++;
        if( (i > 0) && (name[i] == 0) && (fileName[i] == 0) ) found = 1;
    } while( !found && (p + newOffset + BLOCK_SIZE <= cpioSize) );
    if( found ){
        *fileSize = fsize;
        return (const char*)u + p + BLOCK_SIZE + nameLen; /* skip header */
    } else return 0; /* No file found in CPIO - return NULL */
}


#if defined(DXCPIOREAD_EXAMPLE)
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    long fsize, mysize, bytes_read;
    FILE* f;
    char* data;
    const char* myfile;

    if(argc < 2){
        printf(
            "dxCpioRead.c usage example. Public domain.\n"
            "Usage: dxCpioReadExample cpioFile inCpioFile\n"
        );
        return 0;
    }

    f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    data = (char*) malloc(fsize + 1);
    bytes_read = fread(data, 1, fsize, f);
    fclose(f);
    if( bytes_read != fsize ){
        printf("File was not read correctly!\n");
        free(data);
        return -1;
    }
    data[fsize] = 0;

    myfile = dxCpioRead(data, fsize, argv[2], &mysize); 
    if( myfile ){
        printf("File content:\n");
        fwrite(myfile, mysize, 1, stdout);
        printf("\nFile size: %ld\n", mysize);
    }else printf("No file '%s' in archive. Or incorrect archive\n",argv[2]);
    free(data); /* free data after you finished using CPIO reader */
    return 0;
}
#endif /* DXCPIOREAD_EXAMPLE */
