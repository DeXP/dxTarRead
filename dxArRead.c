/*
 dxArRead - 0.4 - public domain
 no warrenty implied; use at your own risk.
 authored from 2017 by Dmitry Hrabrov a.k.a. DeXPeriX
 http://dexperix.net

LICENSE:
    This software is dual-licensed to the public domain and under the following
    license: you are granted a perpetual, irrevocable license to copy, modify,
    publish and distribute this file as you see fit.
*/
const char* dxArRead(const void* arData, const long arSize, 
                      const char* fileName, long* fileSize)
{
    const int NAME_OFFSET = 0, SIZE_OFFSET = 48, NL_OFFSET = 59;
    const int BLOCK_SIZE = 60, NAME_SIZE = 16, SZ_SIZE = 10, MAGIC_SIZE = 8;
    const char MAGIC[] = "!<arch>\n"; /* ar's magic const */
    const char* ar = (const char*) arData; /* From "void*" to "char*" */
    long fsize, mul, i, p = 0, found = 0, newOffset = 0, sizeEnd;
    
    *fileSize = 0; /* will be zero if AR wrong or there is no such file */
	for(p=0; p<MAGIC_SIZE; p++) 
		if( ar[p] != MAGIC[p] ) return 0;

    do { /* "Load" data from ar - just point to passed memory*/
        const char* name = ar + NAME_OFFSET + p + newOffset;
        const char* sz = ar + SIZE_OFFSET + p + newOffset; /* size string */
        p += newOffset; /* pointer to current file's data in AR */
		if( ar[p + NL_OFFSET] != '\n' ) return 0; /* incorrect archive */
		
        fsize = 0; sizeEnd = SZ_SIZE-1; /* Convert file size from string */
		while( sz[sizeEnd] == ' ' ) sizeEnd--; /* find where size really end */
        for(i=sizeEnd, mul=1; i>=0; mul*=10, i--)
            if( (sz[i]>='1') && (sz[i] <= '9') ) fsize += (sz[i] - '0') * mul;

        /* Offset size in bytes. Depends on file size and AR's block size */
		newOffset = (fsize % 2)? (fsize + BLOCK_SIZE +1): (fsize + BLOCK_SIZE);

        i = 0; /* strncmp - compare file's name with that a user wants */
        while((i<NAME_SIZE) && (fileName[i]!=0) && (name[i]==fileName[i])) i++;
        if( (i > 0) && (fileName[i] == 0) ) found = 1;
    } while( !found && (p + newOffset + BLOCK_SIZE <= arSize) );
    if( found ){
        *fileSize = fsize;
        return ar + p + BLOCK_SIZE; /* skip header, point to data */
    } else return 0; /* No file found in AR - return NULL */
}


#if defined(DXARREAD_EXAMPLE)
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    long fsize, mysize, bytes_read;
    FILE* f;
    char* data;
    const char* myfile;

    if(argc < 2){
        printf(
            "dxArRead.c usage example. Public domain.\n"
            "Usage: dxArReadExample arFile inArFile\n"
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
        puts("File was not read correctly!");
        free(data);
        return -1;
    }
    data[fsize] = 0;

    myfile = dxArRead(data, fsize, argv[2], &mysize); 
    if( myfile ){
        printf("File content: '");
        fwrite(myfile, 1, mysize, stdout);
        printf("'\nFile size: %ld\n", mysize);
    }else printf("No file '%s' in AR archive or archive incorrect\n",argv[2]);
    free(data); /* free data after you finished using ar reader */
    return 0;
}
#endif /* DXARREAD_EXAMPLE */
