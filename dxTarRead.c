/*
 dxTarRead - 0.91 - public domain
 no warrenty implied; use at your own risk.
 authored from 2017 by Dmitry Hrabrov a.k.a. DeXPeriX
 http://dexperix.net

LICENSE:
    This software is dual-licensed to the public domain and under the following
    license: you are granted a perpetual, irrevocable license to copy, modify,
    publish and distribute this file as you see fit.
*/
const char* dxTarRead(const void* tarData, const long tarSize, 
                      const char* fileName, long* fileSize)
{
    const int NAME_OFFSET = 0, SIZE_OFFSET = 124, MAGIC_OFFSET = 257;
    const int BLOCK_SIZE = 512, NAME_SIZE = 100, SZ_SIZE = 12, MAGIC_SIZE = 5;
    const char MAGIC[] = "ustar"; /* Modern GNU tar's magic const */
    const char* tar = (const char*) tarData; /* From "void*" to "char*" */
    long size, mul, i, p = 0, found = 0, newOffset = 0;
    
    *fileSize = 0; /* will be zero if TAR wrong or there is no such file */
    do { /* "Load" data from tar - just point to passed memory*/
        const char* name = tar + NAME_OFFSET + p + newOffset;
        const char* sz = tar + SIZE_OFFSET + p + newOffset; /* size string */
        p += newOffset; /* pointer to current file's data in TAR */

        for(i=0; i<MAGIC_SIZE; i++) /* Check for supported TAR version */
            if( tar[i + MAGIC_OFFSET + p] != MAGIC[i] ) return 0; /* = NULL */

        size = 0; /* Convert file size from string into integer */
        for(i=SZ_SIZE-2, mul=1; i>=0; mul*=8, i--) /* Octal str to int */
            if( (sz[i]>='0') && (sz[i] <= '9') ) size += (sz[i] - '0') * mul;

        /* Offset size in bytes. Depends on file size and TAR's block size */
        newOffset = (1 + size/BLOCK_SIZE) * BLOCK_SIZE; /* trim by block */
        if( (size % BLOCK_SIZE) > 0 ) newOffset += BLOCK_SIZE;

        i = 0; /* strncmp - compare file's name with that a user wants */
        while((i<NAME_SIZE) && (fileName[i]!=0) && (name[i]==fileName[i])) i++;
        if( (i > 0) && (name[i] == 0) && (fileName[i] == 0) ) found = 1;
    } while( !found && (p + newOffset + BLOCK_SIZE <= tarSize) );
    if( found ){
        *fileSize = size;
        return tar + p + BLOCK_SIZE; /* skip header, point to data */
    } else return 0; /* No file found in TAR - return NULL */
}


#if defined(DXTARREAD_EXAMPLE)
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    long fsize, mysize, readed;
    FILE* f;
    char* data;
    const char* myfile;

    if(argc < 2){
        printf(
            "dxTarRead.c usage example. Public domain.\n"
            "Usage: dxTarReadExample tarFile inTarFile\n"
        );
        return 0;
    }
    
    f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    data = (char*) malloc(fsize + 1);
    readed = fread(data, 1, fsize, f);
    fclose(f);
    if( readed != fsize ){
        puts("File not readed correctly!");
        free(data);
        return -1;
    }
    data[fsize] = 0;

    myfile = dxTarRead(data, fsize, argv[2], &mysize); 
    if( myfile ){
        puts("File content:");
        fwrite(myfile, 1, mysize, stdout);
        printf("\nFile size: %ld\n", mysize);
    }else printf("No file '%s' in TAR archive or archive incorrect\n",argv[2]);
    free(data); /* free data after you finished using tar reader */
    return 0;
}
#endif /* DXTARREAD_EXAMPLE */
